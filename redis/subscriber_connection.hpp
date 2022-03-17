#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <cpool/condition_variable.hpp>
#include <cpool/tcp_connection.hpp>
#include <cpool/timer.hpp>

#include "redis/types.hpp"

namespace redis {

using namespace cpool;

class redis_subscriber_connection {
  public:
    redis_subscriber_connection(std::unique_ptr<tcp_connection> connection);

    redis_subscriber_connection(net::any_io_executor exec, string host,
                                uint16_t port);

    net::any_io_executor get_executor() const;

    [[nodiscard]] awaitable<tcp_connection*> get();

    /**
     * @brief Cancels all pending requests
     *
     */
    cpool::error cancel();

    bool connected() const;

    /**
     * @brief Disconnects to the server
     *
     */
    [[nodiscard]] awaitable<cpool::error> async_disconnect();

    /**
     * @brief Sets the callback to be executed when an error message is
     * generated.
     */
    void set_logging_handler(logging_handler handler);

  private:
    /**
     * @brief Connects to the server
     *
     */
    [[nodiscard]] awaitable<cpool::error> async_connect();

    /**
     * @brief Logs the message using the on_log_ event hander.
     * @param level The level of logging @see log_level
     * @param message The message to be logged.
     */
    void log_message(log_level level, string_view message);

  private:
    /// The connection to the server. @see cpool::tcp_connection.
    std::unique_ptr<cpool::tcp_connection> connection_;

    /// Called when there is a call to logMessage. Does nothing if set to
    /// nullptr.
    logging_handler on_log_;

    std::atomic_bool connecting_;
};
} // namespace redis