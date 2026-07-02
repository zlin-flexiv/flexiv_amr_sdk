#pragma once

/**
 * @file seer_amr.h
 * @brief SEER AMR client API.
 */

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <flexiv/amr/vendor/seer/seer_data.h>
#include <flexiv/amr/vendor/seer/seer_options.h>

namespace flexiv::amr::seer {

/**
 * @brief Client for communicating with a SEER AMR controller.
 * @details Owns the SEER TCP request channels and exposes blocking status-query and command member
 * functions. Status functions return SDK-populated response structures; command functions accept
 * user-provided request structures.
 * @note Network operations are serialized per client and may throw AmrException subclasses on
 * connection, timeout, protocol, or robot-side failures.
 * @warning Do not destroy the client while another thread is using it. Always stop indefinite
 * open-loop motion in cleanup paths.
 */
class SeerAmrClient {
  public:
    /**
     * @brief Create a SEER AMR client.
     * @param options Connection, timeout, port, and reconnect options.
     */
    explicit SeerAmrClient(const SeerAmrOptions& options);

    /**
     * @brief Destroy the client and close owned request channels.
     */
    ~SeerAmrClient();

    /**
     * @brief Copy construction is disabled because the client owns network resources.
     */
    SeerAmrClient(const SeerAmrClient&) = delete;

    /**
     * @brief Copy assignment is disabled because the client owns network resources.
     */
    SeerAmrClient& operator=(const SeerAmrClient&) = delete;

    /**
     * @brief Move construction is disabled to keep channel ownership stable.
     */
    SeerAmrClient(SeerAmrClient&&) = delete;

    /**
     * @brief Move assignment is disabled to keep channel ownership stable.
     */
    SeerAmrClient& operator=(SeerAmrClient&&) = delete;

    /**
     * @brief Connect to all required SEER request channels.
     */
    void Connect();

    /**
     * @brief Close all SEER request channels.
     */
    void Disconnect();

    /**
     * @brief Check whether all required SEER request channels are connected.
     * @return True if every request channel is connected.
     */
    bool IsConnected() const;

    /**
     * @brief Get basic AMR information and software versions.
     * @return AMR information reported by the SEER controller.
     */
    SeerAmrInfo GetAmrInfo();

    /**
     * @brief Get AMR runtime statistics.
     * @return Runtime statistics reported by the SEER controller.
     */
    SeerAmrRunInfo GetAmrRunInfo();

    /**
     * @brief Get the current AMR pose.
     * @return Current pose and localization information.
     */
    SeerAmrPose GetAmrPose();

    /**
     * @brief Get actual and received AMR velocities.
     * @return Linear, angular, steering, tray, and stationary-state information.
     */
    SeerAmrSpeed GetAmrSpeed();

    /**
     * @brief Get current obstacle blocking and slowdown status.
     * @return Blocking flags, reason codes, obstacle coordinates, and sensor IDs.
     */
    SeerBlockStatus GetBlockStatus();

    /**
     * @brief Get current battery and charging status.
     * @return Battery status reported by the SEER controller.
     */
    SeerBatteryStatus GetBatteryStatus();

    /**
     * @brief Get current physical and software emergency-stop status.
     * @return Emergency-stop, motor-driver, and relay states.
     */
    SeerEmergencyStatus GetEmergencyStatus();

    /**
     * @brief Get current digital input and output states.
     * @return Typed digital input and output collections.
     */
    SeerIoData GetIoData();

    /**
     * @brief Get current navigation status.
     * @return Navigation status reported by the SEER controller.
     */
    SeerNavigationStatus GetNavigationStatus();

    /**
     * @brief Get current audio playback status.
     * @return Current playback status, audio name, loop flag, and remaining count.
     * @note SEER API 1850 has no request JSON body and uses the status port.
     */
    SeerAudioStatus GetAudioStatus();

    /**
     * @brief Get current task status.
     * @return Task progress and task state list.
     */
    SeerTaskStatus GetTaskStatus();

    /**
     * @brief Get current task list status.
     * @return Current task list status with raw `tasklist_status` JSON preserved.
     * @note SEER API 1026 has no request JSON body and uses the status port.
     */
    SeerTaskListStatus GetTaskListStatus();

    /**
     * @brief Get current localization status code.
     * @return Localization status code reported by SEER.
     */
    int GetLocationStatus();

    /**
     * @brief Get laser scan data.
     * @param return_beams3d Request 3D laser beam data when supported by the AMR.
     * @return Laser scans and associated metadata.
     */
    SeerLaserData GetLaserData(bool return_beams3d = false);

    /**
     * @brief Get ultrasonic sensor data.
     * @return Ultrasonic node distances and activation states reported by the SEER controller.
     */
    SeerUltrasonicData GetUltrasonicData();

    /**
     * @brief Get sensor information for selected depth cameras.
     * @param depth_cameras Depth camera names to query. The list must not be empty.
     * @return Raw sensor information JSON reported by the SEER controller.
     */
    SeerSensorInfoData GetSensorInfo(const std::vector<std::string>& depth_cameras);

    /**
     * @brief Get motor encoder pulse data.
     * @return Motor encoder objects reported by the SEER controller.
     */
    SeerEncoderData GetEncoderData();

    /**
     * @brief Get the latest IMU sample.
     * @return IMU orientation, raw sensor values, offsets, and sample metadata.
     */
    SeerImuData GetImuData();

    /**
     * @brief Check whether AMR control is currently seized.
     * @return True if control is seized.
     */
    bool IsControlSeized();

    /**
     * @brief Perform AMR relocation.
     * @param relocation Relocation request parameters.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool PerformRelocation(const SeerRelocationRequest& relocation);

    /**
     * @brief Cancel the current relocation operation.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool CancelRelocation();

    /**
     * @brief Execute open-loop velocity motion in the AMR coordinate frame.
     * @param command Velocity, steering, and duration parameters.
     * @return True if the AMR accepts and reports success for the command.
     * @warning This command cancels any active navigation task. A duration of zero keeps the AMR
     * moving until StopOpenLoopMotion() or another motion command is issued.
     */
    bool ExecuteOpenLoopMotion(const SeerOpenLoopMotionCommand& command);

    /**
     * @brief Stop the current open-loop motion.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool StopOpenLoopMotion();

    /**
     * @brief Execute fixed-path navigation.
     * @param command Path navigation command. `source_id` and `id` are required.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool ExecuteFixedPathNavigation(const SeerPathNaviCommand& command);

    /**
     * @brief Execute specified-path navigation.
     * @param commands Ordered path navigation commands. Each command requires `source_id` and `id`.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool ExecuteSpecifiedPathNavigation(const std::vector<SeerPathNaviCommand>& commands);

    /**
     * @brief Translate a fixed distance at a fixed velocity in the AMR coordinate frame.
     * @param command Translation distance, velocity components, and motion mode.
     * @return True if the AMR accepts and reports success for the command.
     * @warning This command cancels any unfinished navigation task and cannot run concurrently
     * with SEER motion tasks 3055 through 3058.
     */
    bool ExecuteTranslation(const SeerTranslationCommand& command);

    /**
     * @brief Rotate a fixed angle at a fixed angular velocity in the AMR coordinate frame.
     * @param command Rotation angle, angular velocity, and motion mode.
     * @return True if the AMR accepts and reports success for the command.
     * @warning This command cancels any unfinished navigation task and cannot run concurrently
     * with SEER motion tasks 3055 through 3058.
     */
    bool ExecuteRotation(const SeerRotationCommand& command);

    /**
     * @brief Pause current navigation.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool PauseNavigation();

    /**
     * @brief Resume paused navigation.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool ResumeNavigation();

    /**
     * @brief Cancel current navigation.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool CancelNavigation();

    /**
     * @brief Seize AMR control ownership.
     * @param name User-defined control owner name.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool SeizeControl(const std::string& name);

    /**
     * @brief Release AMR control ownership.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool ReleaseControl();

    /**
     * @brief Clear AMR errors.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool ClearErrors();

    /**
     * @brief Play an uploaded audio file.
     * @param name Audio file name without extension.
     * @param loop True to loop playback; false to play once.
     * @return True if the AMR accepts and reports success for the command.
     * @note SEER API 6000 uses the operation port. The name must contain only documented ASCII
     * characters and must be 1 to 20 characters long.
     */
    bool PlayAudio(const std::string& name, bool loop = false);

    /**
     * @brief Pause current audio playback.
     * @return True if the AMR accepts and reports success for the command.
     * @note SEER API 6010 has no request JSON body and uses the operation port.
     */
    bool PauseAudio();

    /**
     * @brief Upload a WAV audio file.
     * @param audio_file Local `.wav` audio file path, for example `collision.wav`.
     * @return True if the AMR accepts and reports success for the command.
     * @note SEER API 6030 uses a raw binary request body: audio bytes followed by a fixed
     * 20-byte file name field. The SDK reads the file bytes and uses the file stem as the AMR
     * audio name.
     * @warning Only WAV audio files are supported.
     */
    bool UploadAudio(const std::string& audio_file);

    /**
     * @brief Download an audio file from the AMR.
     * @param name Audio file name without extension.
     * @return Audio file bytes.
     * @note SEER API 6031 returns raw file bytes on success and a JSON error object on failure.
     */
    std::vector<uint8_t> DownloadAudio(const std::string& name);

    /**
     * @brief Set one digital output.
     * @param id Digital output ID.
     * @param status Desired output status.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool SetDigitalOutput(int id, bool status);

    /**
     * @brief Set multiple digital outputs.
     * @param outputs Digital output ID and desired status pairs.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool SetDigitalOutputs(const std::vector<std::pair<int, bool>>& outputs);

    /**
     * @brief Set multiple digital outputs from an initializer list.
     * @param outputs Digital output ID and desired status pairs.
     * @return True if the AMR accepts and reports success for the command.
     */
    bool SetDigitalOutputs(std::initializer_list<std::pair<int, bool>> outputs);

    /**
     * @brief Set the software emergency-stop signal.
     * @param active True to output the emergency-stop signal; false to release it.
     * @return True if the AMR accepts and reports success for the command.
     * @warning Releasing the software emergency stop does not guarantee that the AMR can move;
     * physical emergency stops and other safety conditions may still be active.
     */
    bool SetSoftEmergencyStop(bool active);

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace flexiv::amr::seer
