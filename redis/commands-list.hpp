#pragma once

#include <map>
#include <vector>

#include "redis/command.hpp"
#include "redis/types.hpp"
#include "redis/value.hpp"

namespace redis {

inline command rpush(string key, redis::value value) {
    return command(std::vector<std::string>{"RPUSH", key, value});
}

inline command rpush(string key, redis::redis_array values) {
    auto commandStrings = std::vector<std::string>{"RPUSH", key};
    for (auto& value : values) {
        commandStrings.push_back(std::move(value));
    }

    return command(commandStrings);
}

inline command rpushx(string key, redis::value value) {
    return command(std::vector<std::string>{"RPUSHX", key, value});
}

inline command rpushx(string key, redis::redis_array values) {
    auto commandStrings = std::vector<std::string>{"RPUSHX", key};
    for (auto& value : values) {
        commandStrings.push_back(std::move(value));
    }

    return command(commandStrings);
}

inline command rpop(string key, int64_t num = 0) {
    if (num == 0) {
        return command(std::vector<std::string>{"RPOP", key});
    }
    return command(std::vector<std::string>{"RPOP", key, std::to_string(num)});
}

inline command brpop(string key, int64_t timeout = 0) {
    return command(
        std::vector<std::string>{"BRPOP", key, std::to_string(timeout)});
}

inline command brpop(std::vector<string> keys, int64_t timeout = 0) {
    auto commandStrings = std::vector<std::string>{"BRPOP"};
    for (auto& key : keys) {
        commandStrings.push_back(std::move(key));
    }
    commandStrings.push_back(std::to_string(timeout));

    return command(commandStrings);
}

inline command lpush(string key, redis::value value) {
    return command(std::vector<std::string>{"LPUSH", key, value});
}

inline command lpush(string key, redis::redis_array values) {
    auto commandStrings = std::vector<std::string>{"LPUSH", key};
    for (auto& value : values) {
        commandStrings.push_back(std::move(value));
    }

    return command(commandStrings);
}

inline command lpushx(string key, redis::value value) {
    return command(std::vector<std::string>{"LPUSHX", key, value});
}

inline command lpushx(string key, redis::redis_array values) {
    auto commandStrings = std::vector<std::string>{"LPUSHX", key};
    for (auto& value : values) {
        commandStrings.push_back(std::move(value));
    }

    return command(commandStrings);
}

inline command lpop(string key, int64_t num = 0) {
    if (num == 0) {
        return command(std::vector<std::string>{"LPOP", key});
    }
    return command(std::vector<std::string>{"LPOP", key, std::to_string(num)});
}

inline command blpop(string key, int64_t timeout = 0) {
    return command(
        std::vector<std::string>{"BLPOP", key, std::to_string(timeout)});
}

inline command blpop(std::vector<string> keys, int64_t timeout = 0) {
    auto commandStrings = std::vector<std::string>{"BLPOP"};
    for (auto& key : keys) {
        commandStrings.push_back(std::move(key));
    }
    commandStrings.push_back(std::to_string(timeout));

    return command(commandStrings);
}

inline command lset(string key, int64_t index, redis::value value) {
    return command(
        std::vector<std::string>{"LSET", key, std::to_string(index), value});
}

inline command llen(string key) {
    return command(std::vector<std::string>{"LLEN", key});
}

inline command lindex(string key, int64_t index) {
    return command(
        std::vector<std::string>{"LINDEX", key, std::to_string(index)});
}

inline command lrange(string key, int64_t start, int64_t stop) {
    return command(std::vector<std::string>{
        "LRANGE", key, std::to_string(start), std::to_string(stop)});
}

inline command lrem(string key, int64_t count, string elem) {
    return command(
        std::vector<std::string>{"LREM", key, std::to_string(count), elem});
}

inline command linsertbefore(string key, int64_t index) {
    return command(std::vector<std::string>{"LINSERT", key, "BEFORE",
                                            std::to_string(index)});
}

inline command linsertafter(string key, int64_t index) {
    return command(std::vector<std::string>{"LINSERT", key, "AFTER",
                                            std::to_string(index)});
}

} // namespace redis