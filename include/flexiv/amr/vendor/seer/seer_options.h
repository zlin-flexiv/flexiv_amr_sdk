#pragma once

/**
 * @file seer_options.h
 * @brief Configuration options for SEER AMR communication.
 */

#include <cstdint>
#include <string>

namespace flexiv::amr::seer {

/**
 * @brief Timeout settings for SEER TCP request channels.
 * @details User-provided timeout values nested in SeerAmrOptions::time_out.
 * @note All timeout values are expressed in milliseconds.
 */
struct SeerTimeoutOptions {
    /** Timeout in milliseconds for establishing each TCP connection. */
    int connect_timeout_ms = 3000;

    /** Timeout in milliseconds for sending a request frame. */
    int send_timeout_ms = 3000;

    /** Timeout in milliseconds for receiving a response frame. */
    int recv_timeout_ms = 3000;
};

/**
 * @brief TCP ports used by SEER protocol services.
 * @details User-configurable service ports nested in SeerAmrOptions::ports.
 * @warning Change these defaults only when the robot is configured to use matching ports.
 */
struct SeerPortOptions {
    /** Port used by status query commands. */
    uint16_t status_port = 19204;

    /** Port used by control commands. */
    uint16_t control_port = 19205;

    /** Port used by navigation commands. */
    uint16_t navigation_port = 19206;

    /** Port used by configuration commands. */
    uint16_t config_port = 19207;

    /** Port used by operation commands. */
    uint16_t operation_port = 19210;

    /** Port reserved for SEER push data. */
    uint16_t push_port = 19301;
};

/**
 * @brief Options used to create a SEER AMR client.
 * @details User-provided input passed to SeerAmrClient::SeerAmrClient(). Configure the robot host,
 * timeouts, service ports, and connection behavior before constructing the client.
 * @note With `auto_connect` disabled, call SeerAmrClient::Connect() before the first request.
 */
struct SeerAmrOptions {
    /** AMR host name or IP address. */
    std::string host;

    /** Timeout settings for all request channels. */
    SeerTimeoutOptions time_out;

    /** TCP port settings for SEER services. */
    SeerPortOptions ports;

    /** Connect to all required SEER channels during client construction. */
    bool auto_connect = true;

    /** Reconnect automatically before a request when a channel is disconnected. */
    bool auto_reconnect = true;
};

}  // namespace flexiv::amr::seer
