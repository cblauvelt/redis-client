#include "redis/command.hpp"

namespace redis {

using string = std::string;

command::command(string command) {
    auto it = command.begin();
    auto end = command.end();
    string member;
    char delim = ' ';

    while (it != end) {
        if (*it == delim) {        // add a new member
            if (!member.empty()) { // but not if it's empty
                commands_.push_back(member);
            }
            member.clear(); // Clear member to start
            it++;
        } else if (*it ==
                   '"') { // Do not split on delim when it is enclosed in quotes
            it++;
            while (*it != '"') {
                member += *it++;

                if (it == end) { // Check before the next iteration of the loop
                    break;
                }
            }
            if (it != end) {
                it++; // advance past the quote sign
            }
        } else {
            member += *it++;
        }
    }

    // Insert the last member if it's not empty
    if (!member.empty()) {
        commands_.push_back(member);
    }
}

command::command(std::vector<string> commands)
    : commands_(std::move(commands)) {}

bool command::empty() const { return commands_.empty(); }

std::vector<string> command::commands() const { return commands_; }

string command::serialized_command() const {
    string retVal;
    if (empty()) {
        return retVal;
    }

    if (commands_.size() == 1) { // This is used to optimize "PING"
        retVal = commands_[0];
        retVal += "\r\n";
        return retVal;
    }

    retVal = "*";
    retVal += std::to_string(commands_.size());
    retVal += "\r\n";
    for (auto command : commands_) {
        retVal += "$";
        retVal += std::to_string(command.length()) + "\r\n";
        retVal += command + "\r\n";
    }

    return retVal;
}

bool command::operator==(const command& rhs) const {
    return (commands_ == rhs.commands_);
}

bool command::operator!=(const command& rhs) const { return !(*this == rhs); }

} // namespace redis
