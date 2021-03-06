#pragma once

#include <system_error>

namespace redis {

enum class error_code : uint8_t {
    /// no_error No error has occurred
    no_error = 0,
    /// not_supported The requested action is not yet supported
    not_supported = 1,
    /// wrong_type The requested conversion could not be performed.
    wrong_type = 2

};

enum class client_error_code : uint8_t {
    /// no_error No error has occurred
    no_error = 0,
    /// RedisError An error has been returned by the Redis server.
    error,
    /// disconnected The client was disconnected
    disconnected,
    /// write_error There was an error while writing the command to the server
    write_error,
    /// read_error There was an error while reading a response from the server
    read_error,
    /// response_command_mismatch There was a mismatch between the number of
    /// commands sent and the number of responses received.
    response_command_mismatch,
    /// client_stopped The client has been stopped and no further requests will
    /// succeed
    client_stopped
};

enum class subscriber_error_code : uint8_t {
    /// no_error No error has occurred
    no_error = 0,
    /// error An error has been returned by the Redis server.
    error,
    /// disconnected The client was disconnected
    disconnected,
    /// write_error There was an error while writing the command to the server
    write_error,
    /// read_error There was an error while reading a response from the server
    read_error,
    /// response_command_mismatch There was a mismatch between the number of
    /// commands sent and the number of responses received.
    response_command_mismatch,
    /// bad_reply The reply did not match what was expected.
    bad_reply
};

enum class parse_error_code : uint8_t {
    no_error = 0,
    /// eof The end of the buffer was reached unexpectedly
    eof = 1,
    /// out_of_range The parsed number was too large for the container
    out_of_range = 2,
    /// malformed_message The message did not meet the Redis standard
    malformed_message
};

std::error_code make_error_code(redis::error_code);
std::error_code make_error_code(redis::client_error_code);
std::error_code make_error_code(redis::subscriber_error_code);
std::error_code make_error_code(redis::parse_error_code);

} // namespace redis

namespace std {
// Tell the C++ STL metaprogramming that enum error_code
// is registered with the standard error code system
template <> struct is_error_code_enum<redis::error_code> : true_type {};
template <> struct is_error_code_enum<redis::client_error_code> : true_type {};
template <>
struct is_error_code_enum<redis::subscriber_error_code> : true_type {};
template <> struct is_error_code_enum<redis::parse_error_code> : true_type {};
} // namespace std