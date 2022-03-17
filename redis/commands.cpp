#include "commands.hpp"

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

command publish(string channel, string message) {
    return command(std::vector<std::string>{"PUBLISH", channel, message});
}

} // namespace redis