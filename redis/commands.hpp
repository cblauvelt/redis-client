#pragma once

#include "redis/command.hpp"
#include "redis/types.hpp"

#include <string>

namespace redis {

command flush_all();

command get(string key);

command set(string key, string value, parameters params = parameters());

command del(string key);

command exists(string key);

command incr(string key);

command incrby(string key, int64_t num);

command incrbyfloat(string key, double num);

command decr(string key);

command decrby(string key, int64_t num);

command publish(string channel, string message);

} // namespace redis