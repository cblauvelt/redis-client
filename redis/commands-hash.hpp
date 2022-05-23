#pragma once

#include <map>
#include <vector>

#include "redis/command.hpp"
#include "redis/types.hpp"
#include "redis/value.hpp"

namespace redis {

inline command hexists(string key, std::string field) {
    return command(std::vector<std::string>{"hexists", key, field});
}

inline command hset(string key, std::string field, redis::value value,
                    parameters params = parameters()) {
    std::vector<std::string> commandString{"HSET", key, field, (string)value};
    std::move(params.begin(), params.end(), std::back_inserter(commandString));
    return command(commandString);
}

inline command hset(string key,
                    const std::map<std::string, redis::value>& values,
                    parameters params = parameters()) {
    std::vector<std::string> commandString{"HSET", key};
    for (auto [key, value] : values) {
        commandString.push_back(key);
        commandString.push_back((string)value);
    }
    std::move(params.begin(), params.end(), std::back_inserter(commandString));

    return commandString;
}

inline command hsetnx(string key, std::string field, redis::value value) {
    return command(
        std::vector<std::string>{"hsetnx", key, field, (string)value});
}

inline command hget(string key, std::string field) {
    return command(std::vector<std::string>{"HGET", key, field});
}

inline command hget(string key, std::vector<string> fields) {
    std::vector<std::string> commandString{"HSET", key};
    std::move(fields.begin(), fields.end(), std::back_inserter(commandString));
    return commandString;
}

inline command hgetall(std::string key) {
    return command(std::vector<std::string>{"HGETALL", key});
}

inline command hkeys(string key) {
    return command(std::vector<std::string>{"HKEYS", key});
}

inline command hvals(string key) {
    return command(std::vector<std::string>{"HVALS", key});
}

inline command hdel(std::string key, std::string field) {
    return command(std::vector<std::string>{"HDEL", key, field});
}

inline command hlen(std::string key) {
    return command(std::vector<std::string>{"HLEN", key});
}

inline command hincrby(std::string key, std::string field, int64_t num) {
    return command(
        std::vector<std::string>{"HINCRBY", key, field, std::to_string(num)});
}

inline command hincrbyfloat(std::string key, std::string field, double num) {
    return command(std::vector<std::string>{"HINCRBYFLOAT", key, field,
                                            std::to_string(num)});
}

} // namespace redis