#pragma once

#include <queue>
#include <string>

#include <boost/asio/experimental/channel.hpp>
#include <cpool/awaitable_latch.hpp>
#include <cpool/connection_pool.hpp>
#include <cpool/tcp_connection.hpp>

#include "redis/client_config.hpp"
#include "redis/command.hpp"
#include "redis/errors.hpp"
#include "redis/helper_functions.hpp"
#include "redis/message.hpp"
#include "redis/reply.hpp"
#include "redis/subscriber_connection.hpp"
#include "redis/types.hpp"
#include "redis/value.hpp"

namespace redis {

using namespace cpool;
using namespace boost::asio::experimental;

/**
 * @brief This class is used to coordinate communication with a Redis
 * Server.
 */
class redis_subscriber {

  public:
    /**
     * @brief Creates a redis_subscriber using default properties.
     * @param connection The connection on which the subscribe request was made.
     */
    redis_subscriber(std::unique_ptr<cpool::tcp_connection> connection);

    /**
     * @brief Creates a subscriber using default properties.
     * @param exec The Asio executor to use for event handling.
     * @param config The configuration object
     */
    redis_subscriber(cpool::net::any_io_executor exec, client_config config);

    /**
     * @brief Creates a redis_subscriber using default properties.
     * @param exec The Asio executor to use for event handling.
     * @param host The IP Address or hostname of the server.
     * @param port The remote port on which the server is listening.
     */
    redis_subscriber(net::any_io_executor exec, std::string host,
                     uint16_t port);

    redis_subscriber(const redis_subscriber&) = delete;
    redis_subscriber& operator=(const redis_subscriber&) = delete;

    /**
     * @brief Sends a PING command to the server to test connectivity.
     * received.
     */
    [[nodiscard]] awaitable<cpool::error> ping();

    /**
     * @brief Subscribes to a channel and executes the callback onEvent when a
     * message is received.
     * @param channel The channel to subscribe to.
     */
    [[nodiscard]] awaitable<cpool::error> subscribe(string channel);

    /**
     * @brief Unsubscribes to a channel.
     * @param channel The channel to unsubscribe.
     */
    [[nodiscard]] awaitable<cpool::error> unsubscribe(string channel);

    /**
     * @brief Subscribes to a pattern and executes the callback onEvent when a
     * message is received.
     * @param pattern The pattern to subscribe to.
     */
    [[nodiscard]] awaitable<cpool::error> psubscribe(string pattern);

    /**
     * @brief Unsubscribes to a channel.
     * @param pattern The channel to unsubscribe.
     */
    [[nodiscard]] awaitable<cpool::error> punsubscribe(string pattern);

    /**
     * @brief Starts reading subscribed messages
     *
     */
    void start();

    /**
     * @brief Stops all activity within the subscriber
     *
     */
    awaitable<void> stop();

    /**
     * @brief Unsubscribes to all messages
     *
     */
    [[nodiscard]] awaitable<cpool::error> reset();

    /**
     * @brief Reads messages published from the channel
     *
     */
    [[nodiscard]] awaitable<reply> read();

    /**
     * @brief Sets the callback to be executed when an error message is
     * generated.
     */
    void set_logging_handler(logging_handler handler);

    /**
     * @brief Returns whether or not the client is running.
     */
    bool running() const;

  private:
    /**
     * @brief Used to send the command to the server.
     * @param command The command to send to the server.
     */
    [[nodiscard]] awaitable<cpool::error> send(command command);

    /**
     * @brief reads messages from the server.
     */
    [[nodiscard]] awaitable<void> read_messages();

    /**
     * @brief parses the messages in the read buffer
     *
     */
    [[nodiscard]] awaitable<cpool::error_code>
    parse_buffer(buffer_t::const_iterator begin, buffer_t::const_iterator end);

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
    redis_subscriber_connection connection_;

    /// The queue to read messages from
    channel<void(cpool::error_code, reply)> message_queue_;

    // event handlers
    /// Called when there is a call to logMessage. Does nothing if set to
    /// nullptr.
    logging_handler on_log_;

    /// track number of detached tasks
    cpool::awaitable_latch latch_;
    std::atomic_bool read_messages_;
};

} // namespace redis
