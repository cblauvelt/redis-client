#pragma once

#include "redis/command.hpp"
#include "types.hpp"

#include <string>

namespace redis {

command flush_all();

command get(string key);

command set(string key, string value, parameters params = parameters());

command del(string key);

command exists(string key);

command publish(string channel, string message);

} // namespace redis