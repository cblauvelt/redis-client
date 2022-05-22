#pragma once

#include <chrono>
#include <string>
#include <string_view>

using namespace std::chrono_literals;

namespace redis {

/**
 * @brief Contains all the parameters to configure a RedisClient or a
 * RedisSubscriber
 */
struct client_config {
    /// host A string that represents the host name, can be an IP Address
    std::string host;

    /// port The TCP port on which the server is listening
    uint16_t port;

    /// max_connections The maximum number of connections in the connection pool
    unsigned int max_connections;

    /// username Used for authentication with the redis server. If password is
    /// defined and username is blank, it is set to "default".
    std::string username;

    /// password Used for authentication with the redis server
    std::string password;

    /// Creates a configuration with default parameters
    client_config()
        : host("127.0.0.1")
        , port(6379)
        , max_connections(8)
        , username()
        , password() {}

    /**
     * @brief Sets the host name of the server.
     * @param host A string representing the host name or the IP address.
     * @returns The configuration object so subsequent commands to set methods
     * can be chained.
     */
    client_config set_host(std::string host) {
        this->host = host;
        return *this;
    }

    /**
     * @brief Sets the port name of the server.
     * @param port An unsigned integer representing the port number for the
     * server.
     * @returns The configuration object so subsequent commands to set methods
     * can be chained.
     */
    client_config set_port(uint16_t port) {
        this->port = port;
        return *this;
    }

    /**
     * @brief Sets the maximum connections in the connection pool.
     * @param num_connections The max number of connections.
     * @returns The configuration object so subsequent commands to set methods
     * can be chained.
     */
    client_config set_max_connections(unsigned int num_connections) {
        this->max_connections = num_connections;
        return *this;
    }

    /**
     * @brief Sets the username of the server.
     * @param username The username to authenticate with the redis server.
     * @returns The configuration object so subsequent commands to set methods
     * can be chained.
     */
    client_config set_username(std::string username) {
        this->username = username;
        return *this;
    }

    /**
     * @brief Sets the password of the server.
     * @param password The password to authenticate with the redis server.
     * @returns The configuration object so subsequent commands to set methods
     * can be chained.
     */
    client_config set_password(std::string password) {
        this->password = password;
        return *this;
    }
};

} // namespace redis