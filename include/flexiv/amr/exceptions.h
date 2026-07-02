#pragma once

/**
 * @file exceptions.h
 * @brief Exception and error code definitions used by Flexiv AMR SDK.
 */

#include <stdexcept>
#include <string>

namespace flexiv::amr {

/**
 * @brief Error code carried by AMR SDK exceptions.
 */
enum class AmrErrorCode {
    /** Unknown or uncategorized error. */
    kUnknown = 0,

    /** Timeout while establishing a TCP connection. */
    kConnectTimeout,
    /** Timeout while sending a request to the AMR. */
    kSendTimeout,
    /** Timeout while receiving a response from the AMR. */
    kRecvTimeout,

    /** Failed to resolve the AMR host address. */
    kResolveFailed,
    /** Failed to connect to the AMR. */
    kConnectFailed,
    /** Failed to send data to the AMR. */
    kSendFailed,
    /** Failed to receive data from the AMR. */
    kRecvFailed,

    /** Connection was closed or is not available. */
    kDisconnected,

    /** Received or generated an invalid SEER packet header. */
    kInvalidHeader,
    /** Received invalid JSON data from the AMR. */
    kInvalidJson,
    /** Received or generated an invalid SEER frame. */
    kInvalidFrame,

    /** The AMR rejected the request and returned an error code. */
    kRobotRejected,
    /** The requested AMR command is not supported. */
    kUnsupportedCommand,
    /** Internal SDK error. */
    kInternalError
};

/**
 * @brief Base exception type for Flexiv AMR SDK errors.
 */
class AmrException : public std::runtime_error {
  public:
    /**
     * @brief Construct an AMR exception.
     * @param code Error code associated with the failure.
     * @param message Human-readable error message.
     */
    AmrException(AmrErrorCode code, const std::string& message)
        : std::runtime_error(message), code_(code) {}

    /**
     * @brief Get the SDK error code.
     * @return Error code associated with this exception.
     */
    AmrErrorCode code() const noexcept { return code_; }

  private:
    AmrErrorCode code_;
};

/**
 * @brief Exception thrown when a connection, send, or receive operation times out.
 */
class AmrTimeoutError : public AmrException {
  public:
    /**
     * @brief Construct a receive-timeout exception.
     * @param message Human-readable error message.
     */
    explicit AmrTimeoutError(const std::string& message)
        : AmrException(AmrErrorCode::kRecvTimeout, message) {}

    /**
     * @brief Construct a timeout exception with a specific timeout error code.
     * @param code Timeout error code.
     * @param message Human-readable error message.
     */
    AmrTimeoutError(AmrErrorCode code, const std::string& message) : AmrException(code, message) {}
};

/**
 * @brief Exception thrown when connecting to or using an AMR connection fails.
 */
class AmrConnectionError : public AmrException {
  public:
    /**
     * @brief Construct a connection exception.
     * @param code Connection error code.
     * @param message Human-readable error message.
     */
    AmrConnectionError(AmrErrorCode code, const std::string& message)
        : AmrException(code, message) {}
};

/**
 * @brief Exception thrown when socket input or output fails.
 */
class AmrIOError : public AmrException {
  public:
    /**
     * @brief Construct an I/O exception.
     * @param code I/O error code.
     * @param message Human-readable error message.
     */
    AmrIOError(AmrErrorCode code, const std::string& message) : AmrException(code, message) {}
};

/**
 * @brief Exception thrown when SEER protocol data is malformed or inconsistent.
 */
class AmrProtocolError : public AmrException {
  public:
    /**
     * @brief Construct a protocol exception.
     * @param code Protocol error code.
     * @param message Human-readable error message.
     */
    AmrProtocolError(AmrErrorCode code, const std::string& message) : AmrException(code, message) {}
};

/**
 * @brief Exception thrown when the AMR rejects a command or status request.
 */
class AmrRobotError : public AmrException {
  public:
    /**
     * @brief Construct an AMR-side exception.
     * @param code AMR-side error code.
     * @param message Human-readable error message.
     */
    AmrRobotError(AmrErrorCode code, const std::string& message) : AmrException(code, message) {}
};

}  // namespace flexiv::amr
