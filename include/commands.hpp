#pragma once

#include "redis_command.hpp"
#include "redis_defs.hpp"

#include <string>

namespace redis {

redis_command flush_all();

redis_command get(string key);

redis_command set(string key, string value, parameters params = parameters());

redis_command del(string key);

redis_command exists(string key);

redis_command publish(string channel, string message);

} // namespace redis