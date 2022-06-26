#pragma once

#include "redis/command.hpp"
#include "redis/types.hpp"

#include <string>

namespace redis {

command flush_all();

command get(string key);

command set(string key, string value, parameters params = parameters());

template <typename... Args> command del(Args... keys) {
    auto commandStrings = std::vector<std::string>{"DEL"};
    (commandStrings.push_back(std::forward<Args>(keys)), ...);
    return command(std::move(commandStrings));
}

template <typename... Args> command exists(Args... keys) {
    auto commandStrings = std::vector<std::string>{"EXISTS"};
    (commandStrings.push_back(std::forward<Args>(keys)), ...);
    return command(std::move(commandStrings));
}

command incr(string key);

command incrby(string key, int64_t num);

command incrbyfloat(string key, double num);

command decr(string key);

command decrby(string key, int64_t num);

command publish(string channel, string message);

} // namespace redis