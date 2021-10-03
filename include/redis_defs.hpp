#pragma once

#include "redis_message.hpp"
#include "redis_reply.hpp"

namespace redis {

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

using buffer_t = std::vector<uint8_t>;

} // namespace redis