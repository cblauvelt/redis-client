#pragma once

#include "redis_command.hpp"
#include "redis_defs.hpp"

namespace redis {
inline redis_command json_get(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.GET", key, path});
}

inline redis_command json_set(string key, string path, string value) {
    return redis_command(
        std::vector<std::string>{"JSON.SET", key, path, value});
}

inline redis_command json_del(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.DEL", key, path});
}

inline redis_command json_type(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.TYPE", key, path});
}

inline redis_command json_strlen(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.STRLEN", key, path});
}

inline redis_command json_strappend(string key, string path, string value) {
    return redis_command(
        std::vector<std::string>{"JSON.STRAPPEND", key, path, value});
}

inline redis_command json_numincrby(string key, string path, string value) {
    return redis_command(
        std::vector<std::string>{"JSON.NUMINCRBY", key, path, value});
}

inline redis_command json_nummultby(string key, string path, string value) {
    return redis_command(
        std::vector<std::string>{"JSON.NUMMULTBY", key, path, value});
}

inline redis_command json_arrappend(string key, string path, string value) {
    return redis_command(
        std::vector<std::string>{"JSON.ARRAPPEND", key, path, value});
}

inline redis_command json_arrinsert(string key, string path,
                                    std::vector<string> values) {
    std::vector<std::string> commandStrings{"JSON.ARRINSERT", key, path};
    std::move(values.begin(), values.end(), std::back_inserter(commandStrings));
    return redis_command(commandStrings);
}

inline redis_command json_arrtrim(string key, string path, string startIndex,
                                  string length) {
    return redis_command(std::vector<std::string>{"JSON.ARRTRIM", key, path,
                                                  startIndex, length});
}

inline redis_command json_arrpop(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.ARRPOP", key, path});
}

inline redis_command json_objlen(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.OBJLEN", key, path});
}

inline redis_command json_objkeys(string key, string path) {
    return redis_command(std::vector<std::string>{"JSON.OBJKEYS", key, path});
}

} // namespace redis