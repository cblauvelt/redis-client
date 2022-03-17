#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace redis {

/**
 * @brief Models a command to the redis server.
 */
class command {

  public:
    /**
     * @brief Creates a command from a ' ' delimited list of parameters.
     * @param command A ' ' delimited of a command followed by the parameters of
     * the command.
     */
    command(std::string command);

    /**
     * @brief Creates a command from a list of strings that contain the command
     * followed by the parameters.
     * @param command A list whose first element is the command follwed by a
     * list of parameters.
     */
    command(std::vector<std::string> command);

    /**
     * @returns bool True if the command list is empty.
     */
    bool empty() const;

    /**
     * @returns vector<string> The list of commands.
     */
    std::vector<std::string> commands() const;

    /**
     * @returns string A string that contains the command serialized into
     * RedisProtocol.
     */
    std::string serialized_command() const;

    /**
     * @return bool true if the commands are equal, otherwise false.
     */
    bool operator==(const command& rhs) const;

    /**
     * @return bool false if the commands are equal, otherwise true.
     */
    bool operator!=(const command& rhs) const;

  private:
    std::vector<std::string> commands_;
};

} // namespace redis
