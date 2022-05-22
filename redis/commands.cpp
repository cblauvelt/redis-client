#include "redis/commands.hpp"

namespace redis {

command flush_all() { return command("FLUSHALL"); }

command get(string key) {
    return command(std::vector<std::string>{"GET", key});
}

command set(string key, string value, parameters params) {
    std::vector<std::string> commandString{"SET", key, value};
    std::move(params.begin(), params.end(), std::back_inserter(commandString));
    return command(commandString);
}

command del(string key) {
    return command(std::vector<std::string>{"DEL", key});
}

command exists(string key) {
    return command(std::vector<std::string>{"EXISTS", key});
}

command incr(string key) {
    return command(std::vector<std::string>{"INCR", key});
}

command incrby(string key, int64_t num) {
    return command(
        std::vector<std::string>{"INCRBY", key, std::to_string(num)});
}

command incrbyfloat(string key, double num) {
    return command(
        std::vector<std::string>{"INCRBYFLOAT", key, std::to_string(num)});
}

command decr(string key) {
    return command(std::vector<std::string>{"DECR", key});
}

command decrby(string key, int64_t num) {
    return command(
        std::vector<std::string>{"DECR BY", key, std::to_string(num)});
}

command publish(string channel, string message) {
    return command(std::vector<std::string>{"PUBLISH", channel, message});
}

} // namespace redis