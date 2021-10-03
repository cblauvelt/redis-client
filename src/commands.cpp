#include "commands.hpp"

namespace redis {

redis_command flush_all() { return redis_command("FLUSHALL"); }

redis_command get(string key) {
    return redis_command(std::vector<std::string>{"GET", key});
}

redis_command set(string key, string value, parameters params) {
    std::vector<std::string> commandString{"SET", key, value};
    std::move(params.begin(), params.end(), std::back_inserter(commandString));
    return redis_command(commandString);
}

redis_command del(string key) {
    return redis_command(std::vector<std::string>{"DEL", key});
}

redis_command exists(string key) {
    return redis_command(std::vector<std::string>{"EXISTS", key});
}

redis_command publish(string channel, string message) {
    return redis_command(std::vector<std::string>{"PUBLISH", channel, message});
}

} // namespace redis