#pragma once

#include "redis/message.hpp"
#include "redis/reply.hpp"

namespace redis {

class command;

enum class log_level : uint8_t {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    error = 4,
    critical = 5
};

/// The function object to handle an error message.
using logging_handler =
    std::function<void(log_level level, std::string_view message)>;
/// Additional key-value parameters that can be added onto some commands.
using parameters = std::vector<std::string>;
/// Used for pipelining
using commands = std::vector<redis::command>;

using buffer_t = std::vector<uint8_t>;

} // namespace redis