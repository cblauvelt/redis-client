#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include <boost/asio.hpp>
#include <cpool/connection_pool.hpp>
#include <cpool/tcp_connection.hpp>

#include "redis/client_config.hpp"
#include "redis/command.hpp"
#include "redis/helper_functions.hpp"
#include "redis/reply.hpp"
#include "redis/subscriber.hpp"
#include "redis/types.hpp"
#include "redis/value.hpp"

namespace redis {

namespace asio = boost::asio;
using boost::asio::awaitable;

/**
 * @brief This class is used to coordinate communication with a Redis Server.
 */
class client : public enable_shared_from_this<client> {

  public:
    /**
     * @brief Creates a client using default properties.
     * @param exec The Asio executor to use for event handling.
     * @param config The configuration object
     */
    client(cpool::net::any_io_executor exec, client_config config);

    /**
     * @brief Creates a client using default properties.
     * @param exec The Asio executor to use for event handling.
     * @param host The IP Address or hostname of the server.
     * @param port The remote port on which the server is listening.
     */
    client(cpool::net::any_io_executor exec, string host, uint16_t port);

    client(const client&) = delete;
    client& operator=(const client&) = delete;

    /**
     * @brief Sets the configuration object if the client is not running.
     * @param config The configuration object that holds the configuration
     * details.
     *
     * @section Warning: This function will do nothing if the client is already
     * running, that is, after a call to start().
     */
    void set_config(client_config config);

    /**
     * @brief The current configuration of the client.
     */
    client_config config() const;

    /**
     * @brief Sends a PING command to the server to test connectivity.
     * received.
     */
    [[nodiscard]] awaitable<reply> ping();

    /**
     * @brief Fetches a new connection and sends the command to the server.
     * @param command The command to send to the server.
     * @returns The reply from the server. This reply can include the requested
     * value or an error. Check for errors with `reply.error()`
     */
    [[nodiscard]] awaitable<reply> send(command command);

    /**
     * @brief Fetches a new connection and sends the commands to the server.
     * @param commands The commands to send to the server.
     * @returns The replies from the server. This reply can include the
     * requested value or an error.
     */
    [[nodiscard]] awaitable<replies> send(commands commands);

    /**
     * @brief Sets the callback to be executed when an error message is
     * generated.
     */
    void set_logging_handler(logging_handler handler);

    /**
     * @brief Returns whether or not the client is running.
     */
    bool running() const;

    // Event handlers
  private:
    /**
     * @brief Used to send the command to the server.
     * @param connection The connection to use to connect to the server.
     * @param command The command to send to the server.
     */
    [[nodiscard]] awaitable<reply> send(cpool::tcp_connection* connection,
                                        command command);

    /**
     * @brief Used to send the command to the server.
     * @param connection The connection to use to connect to the server.
     * @param command The command to send to the server.
     */
    [[nodiscard]] awaitable<replies> send(cpool::tcp_connection* connection,
                                          commands commands);

    /**
     * @brief Creates the connection object
     *
     * @return std::unique_ptr<cpool::tcp_connection>
     */
    std::unique_ptr<cpool::tcp_connection> connection_ctor();

    [[nodiscard]] awaitable<cpool::error>
    on_connection_state_change(cpool::tcp_connection* conn,
                               const cpool::client_connection_state state);

    [[nodiscard]] awaitable<cpool::error>
    auth_client(cpool::tcp_connection* conn,
                const cpool::client_connection_state state);

    /**
     * @brief Logs the message using the on_log_ event hander.
     * @param level The level of logging @see log_level
     * @param message The message to be logged.
     */
    void log_message(log_level level, string_view message);

  private:
    /// The io_service that is used to schedule asynchronous events.
    cpool::net::any_io_executor exec_;

    /// The configuration options of the client.
    client_config config_;

    /// The connection to the server. @see cpool::tcp_connection.
    std::unique_ptr<cpool::connection_pool<cpool::tcp_connection>> con_pool_;

    // event handlers
    /// Called when there is a call to log_message. Does nothing if set to
    /// nullptr.
    logging_handler on_log_;
};

} // namespace redis
