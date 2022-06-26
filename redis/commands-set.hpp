#pragma once

#include <map>
#include <vector>

#include "redis/command.hpp"
#include "redis/types.hpp"
#include "redis/value.hpp"

namespace redis {

template <typename... Args> command sadd(std::string key, Args... members) {
    auto commandStrings = std::vector<std::string>{"SADD", key};
    (commandStrings.push_back(std::forward<Args>(members)), ...);
    return command(commandStrings);
}

inline command sadd(std::string key, strings members) {
    auto commandStrings = std::vector<std::string>{"SADD", key};
    for (auto& member : members) {
        commandStrings.push_back(std::move(member));
    }

    return command(commandStrings);
}

template <typename... Args> command sdiff(Args... keys) {
    auto commandStrings = std::vector<std::string>{"SDIFF"};
    (commandStrings.push_back(std::forward<Args>(keys)), ...);
    return command(commandStrings);
}

inline command sdiff(strings keys) {
    auto commandStrings = std::vector<std::string>{"SDIFF"};
    for (auto& key : keys) {
        commandStrings.push_back(std::move(key));
    }

    return command(commandStrings);
}

template <typename... Args> command sinter(Args... keys) {
    auto commandStrings = std::vector<std::string>{"SINTER"};
    (commandStrings.push_back(std::forward<Args>(keys)), ...);
    return command(commandStrings);
}

inline command sinter(strings keys) {
    auto commandStrings = std::vector<std::string>{"SINTER"};
    for (auto& key : keys) {
        commandStrings.push_back(std::move(key));
    }

    return command(commandStrings);
}

inline command sismember(std::string key, std::string member) {
    return command(std::vector<std::string>{"SISMEMBER", key, member});
}

inline command sismember(std::string key, strings members) {
    auto commandStrings = std::vector<std::string>{"SISMEMBER", key};
    for (auto& member : members) {
        commandStrings.push_back(std::move(member));
    }

    return command(commandStrings);
}

inline command smembers(std::string key) {
    return command(std::vector<std::string>{"SMEMBERS", key});
}

inline command spop(std::string key, int num_pop = 1) {
    if (num_pop == 1) {
        return command(std::vector<std::string>{"SPOP", key});
    }

    return command(
        std::vector<std::string>{"SPOP", key, std::to_string(num_pop)});
}

template <typename... Args> command srem(std::string key, Args... members) {
    auto commandStrings = std::vector<std::string>{"SREM", key};
    (commandStrings.push_back(std::forward<Args>(members)), ...);
    return command(commandStrings);
}

inline command srem(std::string key, strings members) {
    auto commandStrings = std::vector<std::string>{"SREM", key};
    for (auto& member : members) {
        commandStrings.push_back(std::move(member));
    }

    return command(commandStrings);
}

template <typename... Args> command sunion(Args... keys) {
    auto commandStrings = std::vector<std::string>{"SUNION"};
    (commandStrings.push_back(std::forward<Args>(keys)), ...);
    return command(commandStrings);
}

inline command sunion(strings keys) {
    auto commandStrings = std::vector<std::string>{"SUNION"};
    for (auto& key : keys) {
        commandStrings.push_back(std::move(key));
    }

    return command(commandStrings);
}

} // namespace redis
