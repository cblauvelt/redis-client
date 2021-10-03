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

#include "helper_functions.hpp"
#include "redis_client_config.hpp"
#include "redis_command.hpp"
#include "redis_defs.hpp"
#include "redis_reply.hpp"
#include "redis_subscriber.hpp"
#include "redis_value.hpp"

namespace redis {

namespace asio = boost::asio;
using boost::asio::awaitable;
using error_code = boost::system::error_code;

/**
 * @brief This class is used to coordinate communication with a Redis Server.
 */
class redis_client : public enable_shared_from_this<redis_client> {

    enum class state : uint8_t { not_running = 0, running };

  public:
    /**
     * @brief Creates a redis_client using default properties.
     * @param exec The Asio executor to use for event handling.
     * @param config The configuration object
     */
    redis_client(cpool::net::any_io_executor exec, redis_client_config config);

    /**
     * @brief Creates a redis_client using default properties.
     * @param exec The Asio executor to use for event handling.
     * @param host The IP Address or hostname of the server.
     * @param port The remote port on which the server is listening.
     */
    redis_client(cpool::net::any_io_executor exec, string host, uint16_t port);

    redis_client(const redis_client&) = delete;
    redis_client& operator=(const redis_client&) = delete;

    /**
     * @brief Sets the configuration object if the client is not running.
     * @param config The configuration object that holds the configuration
     * details.
     *
     * @section Warning: This function will do nothing if the client is already
     * running, that is, after a call to start().
     */
    void set_config(redis_client_config config);

    /**
     * @brief The current configuration of the client.
     */
    redis_client_config config() const;

    /**
     * @brief Sends a PING command to the server to test connectivity.
     * received.
     */
    [[nodiscard]] awaitable<redis_reply> ping();

    /**
     * @brief Fetches a new connection and sends the command to the server.
     * @param command The redis_command to send to the server.
     */
    [[nodiscard]] awaitable<redis_reply> send(redis_command command);

    /**
     * @brief Sets the callback to be executed when an error message is
     * generated.
     */
    void set_logging_handler(logging_handler handler);

    /**
     * @brief Returns whether or not the client is running.
     */
    bool running() const;

    /**
     * @brief Logs the message using the mOnLog event hander.
     * @param level The level of logging @see log_level
     * @param message The message to be logged.
     */
    void log_message(log_level level, string_view message);

    // Event handlers
  private:
    /**
     * @brief Used to send the command to the server.
     * @param connection The connection to use to connect to the server.
     * @param command The redis_command to send to the server.
     */
    awaitable<redis_reply> send(cpool::tcp_connection* connection,
                                redis_command command);
    /**
     * @brief This is passed to the TcpConnection::setStateChangeHandler and
     * calls additional error handlers based on the state of the connection.
     */
    void
    on_connection_state_change(const cpool::client_connection_state& state);

  private:
    /// The io_service that is used to schedule asynchronous events.
    cpool::net::any_io_executor exec_;

    /// The configuration options of the client.
    redis_client_config config_;

    /// The connection to the server. @see cpool::tcp_connection.
    std::unique_ptr<cpool::connection_pool<cpool::tcp_connection>> con_pool_;

    // event handlers
    /// Called when there is a call to log_message. Does nothing if set to
    /// nullptr.
    logging_handler on_log_;

    // tracking state
    /// Tracks the state of the client.
    std::atomic<state> state_;
};

} // namespace redis
