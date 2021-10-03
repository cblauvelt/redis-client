#pragma once

#include <string>
#include <vector>

#include "redis_value.hpp"

namespace redis {

using std::string;

/**
 * @brief Models a command to the redis server.
 */
struct redis_message {

    string channel;
    string pattern;
    string contents;

    redis_message() = default;

    /**
     * @brief Creates a message that contains the result of the message.
     * @param message A redis array that contains the conents of the message.
     */
    redis_message(std::vector<string> message)
        : channel()
        , pattern()
        , contents() {
        if (message.size() == 3) {
            if (message[0] != "message") {
                return;
            }

            channel = message[1];
            contents = message[2];
            return;
        }

        if (message.size() == 4) {
            if (message[0] != "pmessage") {
                return;
            }

            pattern = message[1];
            channel = message[2];
            contents = message[3];
            return;
        }
    }

    /**
     * @brief Creates a message that contains the result of the message.
     * @param message A redis array that contains the conents of the message.
     */
    redis_message(redis_array message)
        : channel()
        , pattern()
        , contents() {
        if (message.size() == 3) {
            if (message[0].as<string>().value_or("") != "message") {
                return;
            }

            channel = message[1].as<string>().value_or("");
            contents = message[2].as<string>().value_or("");
            return;
        }

        if (message.size() == 4) {
            if (message[0].as<string>().value_or("") != "pmessage") {
                return;
            }

            pattern = message[1].as<string>().value_or("");
            channel = message[2].as<string>().value_or("");
            contents = message[3].as<string>().value_or("");
            return;
        }
    }

    /**
     * @brief Returns true if the channel or pattern is not empty
     *
     */
    bool valid() const { return (!channel.empty()); }

    /**
     * @brief Returns if the message has been populated with data.
     */
    bool empty() const { return contents.empty(); }
};

} // namespace redis
