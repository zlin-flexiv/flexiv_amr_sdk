#pragma once

/**
 * @file seer_data.h
 * @brief Data structures exchanged with SEER AMR services.
 */

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace flexiv::amr::seer {

/**
 * @brief Basic information about the AMR and its software versions.
 * @details Returned by SeerAmrClient::GetAmrInfo(). Applications read these fields after the SDK
 * parses the SEER response; they are not request parameters.
 * @note Fields omitted by the controller retain their default values.
 */
struct SeerAmrInfo {
    /** AMR ID reported by the SEER controller. */
    std::string amr_id;

    /** AMR vehicle name or ID. */
    std::string amr_name;

    /** User note configured on the AMR. */
    std::string amr_note;

    /** RoboKit software version. */
    std::string robokit_version;

    /** AMR model name. */
    std::string amr_model_name;

    /** DSP firmware version. */
    std::string dsp_version;

    /** Gyroscope firmware version. */
    std::string gyro_version;

    /** Current map version. */
    std::string map_version;

    /** AMR model configuration version. */
    std::string model_version;

    /** Network protocol version. */
    std::string netprotocol_version;

    /** Modbus interface version. */
    std::string modbus_version;

    /** Name of the currently loaded map. */
    std::string current_map;
};

/**
 * @brief Runtime statistics of the AMR.
 * @details Returned by SeerAmrClient::GetAmrRunInfo(). Applications read these values; they are
 * populated from the SEER status response.
 * @note Fields omitted by the controller retain their default values.
 */
struct SeerAmrRunInfo {
    /** Total odometer mileage reported by the AMR. */
    double mileage = 0.0;

    /** Mileage accumulated today. */
    double today_mileage = 0.0;

    /** Runtime for the current session. */
    double time = 0.0;

    /** Total accumulated runtime. */
    double total_time = 0.0;

    /** Controller temperature. */
    double controller_temp = 0.0;

    /** Controller humidity. */
    double controller_humi = 0.0;

    /** Controller voltage. */
    double controller_voltage = 0.0;
};

/**
 * @brief Current AMR pose and localization information.
 * @details Returned by SeerAmrClient::GetAmrPose() after parsing the robot status response.
 * @note Position is expressed in the current map frame and `angle` is in radians.
 */
struct SeerAmrPose {
    /** X position in the current map frame. */
    double x = 0.0;

    /** Y position in the current map frame. */
    double y = 0.0;

    /** Heading angle in the current map frame. */
    double angle = 0.0;

    /** Localization confidence reported by the AMR. */
    double confidence = 0.0;

    /** Localization method code reported by the AMR. */
    int localization_method = 0;

    /** Current station ID or name. */
    std::string current_station;

    /** Last station ID or name. */
    std::string last_station;
};

/**
 * @brief Current actual and received AMR velocities.
 * @details Returned by SeerAmrClient::GetAmrSpeed(). Linear and angular values use the AMR
 * coordinate frame. The `r_` fields are the latest velocities received by the controller, while
 * the fields without that prefix are actual measured values.
 * @note Positive angular velocity is counterclockwise. Fields omitted by the controller retain
 * their default values.
 */
struct SeerAmrSpeed {
    /** Actual X-axis velocity in m/s. */
    double vx = 0.0;

    /** Actual Y-axis velocity in m/s. */
    double vy = 0.0;

    /** Actual angular velocity in rad/s. */
    double w = 0.0;

    /** Actual single-steering-wheel angle in radians. */
    double steer = 0.0;

    /** Actual tray angle in radians. */
    double spin = 0.0;

    /** Received X-axis velocity in m/s. */
    double r_vx = 0.0;

    /** Received Y-axis velocity in m/s. */
    double r_vy = 0.0;

    /** Received angular velocity in rad/s. */
    double r_w = 0.0;

    /** Received single-steering-wheel angle in radians. */
    double r_steer = 0.0;

    /** Received tray angular velocity in rad/s. */
    double r_spin = 0.0;

    /** Actual steering angles for multi-steering-wheel AMRs, in model order and radians. */
    std::vector<double> steer_angles;

    /** Received steering angles for multi-steering-wheel AMRs, in model order and radians. */
    std::vector<double> r_steer_angles;

    /** Whether all walking motors are stationary. */
    bool is_stop = false;
};

/**
 * @brief Current obstacle blocking and slowdown status of the AMR.
 * @details Returned by SeerAmrClient::GetBlockStatus(). Reason values use the SEER codes: 0
 * ultrasonic, 1 laser, 2 falling-down sensor, 3 collision sensor, 4 infrared sensor, 5 lock, 6
 * dynamic obstacle, 7 virtual laser point, 8 3D camera, 9 distance sensor, and 10 DI ultrasonic.
 * @note A reason or sensor ID of -1 means the corresponding optional field was not reported.
 */
struct SeerBlockStatus {
    /** Whether the AMR is currently blocked. */
    bool blocked = false;

    /** Blocking reason code, or -1 when not reported. */
    int block_reason = -1;

    /** X coordinate of the nearest blocking obstacle in meters. */
    double block_x = 0.0;

    /** Y coordinate of the nearest blocking obstacle in meters. */
    double block_y = 0.0;

    /** ID of the sensor that caused blocking, or -1 when not reported. */
    int block_id = -1;

    /** Whether the AMR is currently slowed by an obstacle. */
    bool slowed = false;

    /** Slowdown reason code, or -1 when not reported. */
    int slow_reason = -1;

    /** X coordinate of the nearest slowdown obstacle in meters. */
    double slow_x = 0.0;

    /** Y coordinate of the nearest slowdown obstacle in meters. */
    double slow_y = 0.0;

    /** ID of the sensor that caused slowdown, or -1 when not reported. */
    int slow_id = -1;
};

/**
 * @brief Battery and charging status of the AMR.
 * @details Returned by SeerAmrClient::GetBatteryStatus(). Applications read these values; they are
 * not command parameters.
 * @note Fields omitted by the controller retain their default values.
 */
struct SeerBatteryStatus {
    /** Battery level percentage. */
    double battery_level = 0.0;

    /** Battery temperature. */
    double battery_temp = 0.0;

    /** Battery voltage. */
    double voltage = 0.0;

    /** Battery current. */
    double current = 0.0;

    /** Maximum allowed charging voltage. */
    double max_charge_voltage = 0.0;

    /** Maximum allowed charging current. */
    double max_charge_current = 0.0;

    /** Battery charge cycle count. */
    int battery_cycle = 1;

    /** Whether the AMR is currently charging. */
    bool charging = false;

    /** Whether manual charging is active. */
    bool manual_charge = false;

    /** Whether automatic charging is active. */
    bool auto_charge = false;

    /** User-defined battery data string. */
    std::string battery_user_data;

    /** Extra battery information reported by the controller. */
    std::string extra;
};

/**
 * @brief Physical, motor-driver, relay, and software emergency-stop status.
 * @details Returned by SeerAmrClient::GetEmergencyStatus(). Applications read these values after
 * the SDK parses the SEER status response.
 * @note Fields omitted by the controller retain their default values.
 */
struct SeerEmergencyStatus {
    /** Whether the physical emergency-stop button is active. */
    bool emergency = false;

    /** Whether a motor driver is in emergency-stop state. */
    bool driver_emc = false;

    /** Whether the relay is enabled. */
    bool electric = false;

    /** Whether software emergency stop is active. */
    bool soft_emc = false;
};

/** @brief One digital input reported by the AMR. */
struct SeerDigitalInput {
    /** Digital input ID. */
    int id = 0;

    /** Input source, such as `normal`, `virtual`, or `modbus`. */
    std::string source;

    /** True for high level and false for low level. */
    bool status = false;

    /** Whether this digital input is enabled. */
    bool valid = false;
};

/** @brief One digital output reported by the AMR. */
struct SeerDigitalOutput {
    /** Digital output ID. */
    int id = 0;

    /** Output source, such as `normal` or `modbus`. */
    std::string source;

    /** True for high level and false for low level. */
    bool status = false;
};

/**
 * @brief Digital input and output states returned by the AMR.
 * @details Returned by SeerAmrClient::GetIoData(). The response uses the protocol keys `DI` and
 * `DO`; the SDK exposes them as typed collections.
 * @note Missing `DI` or `DO` arrays produce empty collections.
 */
struct SeerIoData {
    /** Digital inputs reported by the AMR. */
    std::vector<SeerDigitalInput> digital_inputs;

    /** Digital outputs reported by the AMR. */
    std::vector<SeerDigitalOutput> digital_outputs;
};

/**
 * @brief State of one task in the SEER task status list.
 * @details Nested response item contained in SeerTaskStatus::task_states.
 */
struct SeerTaskState {
    /** Task status code reported by SEER. */
    int status = 0;

    /** Task type code reported by SEER. */
    int type = 0;

    /** Task ID. */
    std::string task_id;
};

/**
 * @brief Progress and status information of the current SEER task.
 * @details Returned by SeerAmrClient::GetTaskStatus(). The SDK converts the nested SEER task
 * package into this application-facing structure.
 * @note When no task package is present, fields remain default-initialized.
 */
struct SeerTaskStatus {
    /** Completion percentage of the current task. */
    double percentage = 0.0;

    /** Remaining or traveled task distance reported by SEER. */
    double distance = 0.0;

    /** Closest target ID or name. */
    std::string closest_target;

    /** Source station ID or name. */
    std::string source_name;

    /** Target station ID or name. */
    std::string target_name;

    /** Additional task information. */
    std::string info;

    /** Detailed state list for sub-tasks. */
    std::vector<SeerTaskState> task_states;
};

/**
 * @brief Status of the current SEER task list.
 * @details Returned by SeerAmrClient::GetTaskListStatus(). The local FMR document defines
 * `tasklist_status` as a JSON object and provides these commonly observed fields. The original
 * object is preserved in SeerTaskListStatus::data for vendor-specific extensions.
 * @note Fields omitted by the controller retain their default values.
 */
struct SeerTaskListStatus {
    /** Raw `tasklist_status` JSON object reported by SEER. */
    nlohmann::json data = nlohmann::json::object();

    /** Current action group ID. */
    int action_group_id = 0;

    /** Current action IDs in the task list. */
    std::vector<int> action_ids;

    /** Whether the task list is configured to loop. */
    bool loop = false;

    /** Current task ID. */
    int task_id = 0;

    /** Task list name. */
    std::string task_list_name;

    /** Task list status code reported by SEER. */
    int task_list_status = 0;
};

/**
 * @brief Current navigation status of the AMR.
 * @details Returned by SeerAmrClient::GetNavigationStatus() to report navigation progress and path
 * state.
 * @note `task_status` uses SEER values: 0 none, 2 running, 3 suspended, 4 completed, 5 failed,
 * and 6 canceled.
 */
struct SeerNavigationStatus {
    /** Navigation task status code reported by SEER. */
    int task_status = 0;

    /** Navigation task type code reported by SEER. */
    int task_type = 0;

    /** Current target ID. */
    std::string target_id;

    /** Path IDs that have already been completed. */
    std::vector<std::string> finished_path;

    /** Path IDs that have not yet been completed. */
    std::vector<std::string> unfinished_path;
};

/**
 * @brief Current audio playback status.
 * @details Returned by SeerAmrClient::GetAudioStatus(). Applications read these values after the
 * SDK parses SEER status API 1850.
 * @note The local FMR example represents `loop` as the string `"false"`, while the field table
 * defines it as a bool. The SDK accepts both forms.
 */
struct SeerAudioStatus {
    /** Current audio playback status code reported by SEER. */
    int status = 0;

    /** Current audio name without extension, or empty when no audio is active. */
    std::string sound_name;

    /** Whether current audio playback is looping. */
    bool loop = false;

    /** Remaining playback count reported by SEER. */
    int count = 0;
};

/**
 * @brief One laser scan beam.
 * @details Nested 2D beam sample contained in SeerLaser::beams.
 */
struct SeerLaserBeam {
    /** Beam angle. */
    double angle = 0.0;

    /** Beam distance. */
    double dist = 0.0;

    /** Beam signal strength. */
    double rssi = 0.0;

    /** Whether this beam is valid. */
    bool valid = false;

    /** Whether this beam detects an obstacle. */
    bool is_obstacle = false;

    /** Whether this beam is virtual. */
    bool is_virtual = false;
};

/**
 * @brief Static information about one laser device.
 * @details Nested device metadata contained in SeerLaser::device_info.
 */
struct SeerLaserDeviceInfo {
    /** Laser device name. */
    std::string device_name;

    /** Maximum scan angle. */
    double max_angle = 0.0;

    /** Maximum scan range. */
    double max_range = 0.0;

    /** Minimum scan angle. */
    double min_angle = 0.0;

    /** Minimum scan range. */
    double min_range = 0.0;

    /** Published angular step. */
    double pub_step = 0.0;

    /** Real angular step. */
    double real_step = 0.0;

    /** Scan frequency. */
    double scan_freq = 0.0;

    /** Time increment between beams. */
    double time_increment = 0.0;
};

/**
 * @brief Header information attached to one laser scan.
 * @details Nested timestamp and frame metadata contained in SeerLaser::header.
 */
struct SeerLaserHeader {
    /** Data timestamp nanosecond field. */
    std::string data_nsec;

    /** Coordinate frame ID. */
    std::string frame_id;

    /** Publish timestamp nanosecond field. */
    std::string pub_nsec;

    /** Sequence number. */
    std::string seq;
};

/**
 * @brief Installation pose of one laser device on the AMR.
 * @details Nested installation metadata contained in SeerLaser::install_info.
 */
struct SeerLaserInstallInfo {
    /** Whether the laser is installed upside down. */
    bool upside = false;

    /** X offset of the laser installation pose. */
    double x = 0.0;

    /** Y offset of the laser installation pose. */
    double y = 0.0;

    /** Yaw offset of the laser installation pose. */
    double yaw = 0.0;

    /** Z offset of the laser installation pose. */
    double z = 0.0;
};

/**
 * @brief Laser scan data and metadata from one laser device.
 * @details One device entry within SeerLaserData::lasers. It combines 2D beams with device, frame,
 * and installation metadata.
 */
struct SeerLaser {
    /** Laser beams in this scan. */
    std::vector<SeerLaserBeam> beams;

    /** Device information for this laser. */
    SeerLaserDeviceInfo device_info;

    /** Header information for this scan. */
    SeerLaserHeader header;

    /** Installation information for this laser. */
    SeerLaserInstallInfo install_info;

    /** Whether this laser is used for localization. */
    bool use_for_loc = false;
};

/**
 * @brief Laser data returned by the AMR.
 * @details Returned by SeerAmrClient::GetLaserData(). The `lasers` collection contains one entry
 * for each reported laser device.
 * @note This structure currently models the documented 2D `beams` response.
 */
struct SeerLaserData {
    /** Laser scans reported by all available laser devices. */
    std::vector<SeerLaser> lasers;
};

/**
 * @brief One ultrasonic sensor node sample.
 * @details Nested response item contained in SeerUltrasonicData::ultrasonic_nodes.
 */
struct SeerUltrasonicNode {
    /** Ultrasonic node ID. */
    int id = 0;

    /** Sensed distance in meters. */
    double dist = 0.0;

    /** Whether this ultrasonic node is active. */
    bool valid = false;
};

/**
 * @brief Ultrasonic sensor data returned by the AMR.
 * @details Returned by SeerAmrClient::GetUltrasonicData(). Each node reports its configured ID,
 * sensed distance in meters, and activation state.
 * @note Convert `id` and `dist` to a world-frame point with the corresponding ultrasonic
 * configuration in the AMR model file.
 */
struct SeerUltrasonicData {
    /** Ultrasonic nodes reported by the AMR. */
    std::vector<SeerUltrasonicNode> ultrasonic_nodes;
};

/**
 * @brief Sensor information returned by the AMR.
 * @details Returned by SeerAmrClient::GetSensorInfo(). The local FMR documentation does not
 * define a stable response schema for this API, so the SDK preserves the response JSON.
 */
struct SeerSensorInfoData {
    /** Raw JSON response from `robot_status_sensors_data_res`. */
    nlohmann::json data;
};

/** @brief Encoder data returned by the AMR. */
struct SeerEncoderData {
    /** Raw objects from the `motor_encoder` response array. */
    std::vector<nlohmann::json> motor_encoders;

    /** Values from the deprecated `encoder` response array. */
    std::vector<std::int64_t> legacy_encoder_pulses;
};

/**
 * @brief Header information attached to one IMU sample.
 * @details Nested timestamp and frame metadata contained in SeerImuData::header.
 */
struct SeerImuHeader {
    /** Data acquisition timestamp since AMR startup, in nanoseconds. */
    std::string data_nsec;

    /** Coordinate frame ID. */
    std::string frame_id;

    /** Publish timestamp since AMR startup, in nanoseconds. */
    std::string pub_nsec;

    /** Sequence number. */
    std::string seq;
};

/**
 * @brief IMU data returned by the AMR.
 * @details Returned by SeerAmrClient::GetImuData(). It contains orientation, raw accelerometer and
 * gyroscope ADC values, offsets, quaternion data, and sample metadata.
 * @note Missing optional response fields retain their default values.
 */
struct SeerImuData {
    /** Header information for this IMU sample. */
    SeerImuHeader header;

    /** Yaw angle in radians. */
    double yaw = 0.0;

    /** Roll angle in radians. */
    double roll = 0.0;

    /** Pitch angle in radians. */
    double pitch = 0.0;

    /** Raw X-axis accelerometer ADC value. */
    double acc_x = 0.0;

    /** Raw Y-axis accelerometer ADC value. */
    double acc_y = 0.0;

    /** Raw Z-axis accelerometer ADC value. */
    double acc_z = 0.0;

    /** Raw X-axis gyroscope ADC value. */
    double rot_x = 0.0;

    /** Raw Y-axis gyroscope ADC value. */
    double rot_y = 0.0;

    /** Raw Z-axis gyroscope ADC value. */
    double rot_z = 0.0;

    /** X-axis gyroscope offset ADC value. */
    std::int32_t rot_off_x = 0;

    /** Y-axis gyroscope offset ADC value. */
    std::int32_t rot_off_y = 0;

    /** Z-axis gyroscope offset ADC value. */
    std::int32_t rot_off_z = 0;

    /** X component of the orientation quaternion. */
    double qx = 0.0;

    /** Y component of the orientation quaternion. */
    double qy = 0.0;

    /** Z component of the orientation quaternion. */
    double qz = 0.0;

    /** W component of the orientation quaternion. */
    double qw = 0.0;
};

/**
 * @brief Request parameters for AMR relocation.
 * @details User-provided input passed to SeerAmrClient::PerformRelocation(). Only engaged optional
 * fields are serialized into the request.
 * @note When `is_auto` is true, the controller ignores all position fields.
 * @warning Manual relocation requires both `x` and `y`.
 */
struct SeerRelocationRequest {
    /** Whether to perform automatic relocation. If true, position fields are ignored. */
    std::optional<bool> is_auto;

    /** Target X position for manual relocation. Required when automatic relocation is not used. */
    std::optional<double> x;

    /** Target Y position for manual relocation. Required when automatic relocation is not used. */
    std::optional<double> y;

    /** Target heading angle for manual relocation. */
    std::optional<double> angle;

    /** Relocation search length. */
    std::optional<double> length;

    /** Whether to relocate to the configured home pose. */
    std::optional<bool> home;
};

/**
 * @brief Open-loop velocity command in the AMR coordinate frame.
 * @details User-provided input passed to SeerAmrClient::ExecuteOpenLoopMotion(). For multi-steering
 * robots, use `vx`, `vy`, and `w`. For a single steering wheel, use `steer` or `real_steer`.
 * @note `real_steer` takes priority over `steer`; either steering field suppresses `vy` and `w`.
 * @warning Executing this command cancels active navigation. A zero `duration` keeps moving until
 * SeerAmrClient::StopOpenLoopMotion() is called.
 */
struct SeerOpenLoopMotionCommand {
    /** X-axis velocity in m/s. Defaults to zero when omitted. */
    std::optional<double> vx;

    /** Y-axis velocity in m/s. Defaults to zero when omitted. */
    std::optional<double> vy;

    /** Angular velocity in rad/s. Positive is counterclockwise. */
    std::optional<double> w;

    /**
     * Single-steering-wheel increment command: -2 for reset, 1 for +15 degrees, or -1 for
     * -15 degrees.
     */
    std::optional<int> steer;

    /** Target single-steering-wheel angle in radians. Takes priority over `steer`. */
    std::optional<double> real_steer;

    /** Command duration in milliseconds. Zero keeps moving until explicitly stopped. */
    std::optional<std::int64_t> duration;
};

/**
 * @brief Path navigation command parameters.
 * @details User-provided input for SeerAmrClient::ExecuteFixedPathNavigation() and each element of
 * SeerAmrClient::ExecuteSpecifiedPathNavigation().
 * @warning `source_id` and `id` are required. Starting a new task cancels an unfinished task.
 */
struct SeerPathNaviCommand {
    /** Source path or station ID. Required by path navigation requests. */
    std::optional<std::string> source_id;

    /** Target path or station ID. Required by path navigation requests. */
    std::optional<std::string> id;

    /** User task ID. */
    std::optional<std::string> task_id;

    /** Target heading angle. */
    std::optional<double> angle;

    /** Navigation method. */
    std::optional<std::string> method;

    /** Maximum linear speed. */
    std::optional<double> max_speed;

    /** Maximum angular speed. */
    std::optional<double> max_wspeed;

    /** Maximum linear acceleration. */
    std::optional<double> max_acc;

    /** Maximum angular acceleration. */
    std::optional<double> max_wacc;

    /** Command duration. */
    std::optional<double> duration;

    /** Target orientation. */
    std::optional<double> orientation;

    /** Whether the AMR should spin at the target. */
    std::optional<bool> spin;
};

/**
 * @brief Fixed-distance translation command in the AMR coordinate frame.
 * @details User-provided input passed to SeerAmrClient::ExecuteTranslation(). Velocity signs select
 * direction while `dist` remains an absolute distance.
 * @note Mode 0 uses odometry and mode 1 uses localization; omission defaults to odometry.
 * @warning `dist` is required and non-negative. This command cancels an unfinished navigation task.
 */
struct SeerTranslationCommand {
    /** Absolute translation distance in meters. Required and must be non-negative. */
    std::optional<double> dist;

    /** X-axis velocity in m/s. Positive is forward and negative is backward. */
    std::optional<double> vx;

    /** Y-axis velocity in m/s. Positive is left and negative is right. */
    std::optional<double> vy;

    /** Motion mode: 0 for odometry mode or 1 for localization mode. */
    std::optional<int> mode;
};

/**
 * @brief Fixed-angle rotation command in the AMR coordinate frame.
 * @details User-provided input passed to SeerAmrClient::ExecuteRotation(). The sign of `vw`
 * selects the turn direction while `angle` remains absolute.
 * @note Positive `vw` turns counterclockwise; negative `vw` turns clockwise.
 * @warning `angle` and `vw` are required. This command cancels an unfinished navigation task.
 */
struct SeerRotationCommand {
    /** Absolute rotation angle in radians. Required, non-negative, and may exceed 2 pi. */
    std::optional<double> angle;

    /** Angular velocity in rad/s. Required; positive is counterclockwise. */
    std::optional<double> vw;

    /** Motion mode: 0 for odometry mode or 1 for localization mode. */
    std::optional<int> mode;
};

}  // namespace flexiv::amr::seer
