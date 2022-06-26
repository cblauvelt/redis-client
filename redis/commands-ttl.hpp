#pragma once

#include "redis/command.hpp"
#include "redis/types.hpp"

#include <string>

namespace redis {

enum class ttl_param : uint8_t { None = 0, NX, XX, GT, LT };
inline std::string to_string(ttl_param param) {
    switch (param) {
    case ttl_param::NX:
        return "NX";

    case ttl_param::XX:
        return "XX";

    case ttl_param::GT:
        return "GT";

    case ttl_param::LT:
        return "LT";

    default:
        return "";
    }
}

command expire(string key, std::chrono::seconds time,
               ttl_param param = ttl_param::None) {
    if (param == ttl_param::None) {
        return command(std::vector<std::string>{"EXPIRE", key,
                                                std::to_string(time.count())});
    }

    return command(std::vector<std::string>{
        "EXPIRE", key, std::to_string(time.count()), to_string(param)});
}

command expireat(string key, int64_t unix_time,
                 ttl_param param = ttl_param::None) {
    if (param == ttl_param::None) {
        return command(std::vector<std::string>{"EXPIREAT", key,
                                                std::to_string(unix_time)});
    }

    return command(std::vector<std::string>{"EXPIREAT", key, to_string(param)});
}

command persist(string key) {
    return command(std::vector<std::string>{"PERSIST", key});
}

command pexpire(string key, std::chrono::milliseconds time,
                ttl_param param = ttl_param::None) {
    if (param == ttl_param::None) {
        return command(std::vector<std::string>{"PEXPIRE", key,
                                                std::to_string(time.count())});
    }

    return command(std::vector<std::string>{
        "PEXPIRE", key, std::to_string(time.count()), to_string(param)});
}

command pexpireat(string key, int64_t unix_time,
                  ttl_param param = ttl_param::None) {
    if (param == ttl_param::None) {
        return command(std::vector<std::string>{"PEXPIREAT", key,
                                                std::to_string(unix_time)});
    }

    return command(
        std::vector<std::string>{"PEXPIREAT", key, to_string(param)});
}

command pttl(string key) {
    return command(std::vector<std::string>{"PTTL", key});
}

command ttl(string key) {
    return command(std::vector<std::string>{"TTL", key});
}

} // namespace redis