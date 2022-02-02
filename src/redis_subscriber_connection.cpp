#include "redis_subscriber_connection.hpp"

#include <cpool/back_off.hpp>

namespace redis {

redis_subscriber_connection::redis_subscriber_connection(
    std::unique_ptr<cpool::tcp_connection> connection)
    : connection_(std::move(connection))
    , on_log_()
    , connecting_(false) {}

redis_subscriber_connection::redis_subscriber_connection(
    net::any_io_executor exec, string host, uint16_t port)
    : connection_(std::make_unique<tcp_connection>(exec, host, port))
    , on_log_()
    , connecting_(false) {}

net::any_io_executor redis_subscriber_connection::get_executor() const {
    return connection_->get_executor();
}

awaitable<tcp_connection*> redis_subscriber_connection::get() {
    // log_message(redis::log_level::trace, "Getting connection");
    if (connection_->connected()) {
        co_return connection_.get();
    }

    co_await async_connect();
    co_return connection_.get();
}

cpool::error redis_subscriber_connection::cancel() {
    cpool::error_code err;
    connection_->socket().cancel(err);
    return cpool::error(err);
}

bool redis_subscriber_connection::connected() const {
    return connection_->connected();
}

[[nodiscard]] awaitable<cpool::error>
redis_subscriber_connection::async_disconnect() {

    return connection_->async_disconnect();
}

void redis_subscriber_connection::set_logging_handler(logging_handler handler) {
    on_log_ = std::move(handler);
}

[[nodiscard]] awaitable<cpool::error>
redis_subscriber_connection::async_connect() {
    // wait for it to connect if another thread is already connecting
    bool expected_connecting_status = false;
    if (!connecting_.compare_exchange_strong(expected_connecting_status,
                                             true)) {
        co_await connection_->wait_for(client_connection_state::connected);
        co_return cpool::no_error;
    }

    log_message(redis::log_level::trace, "Attempting first connect");
    auto exec = co_await net::this_coro::executor;
    auto err = co_await connection_->async_connect();
    if (err) {
        log_message(
            redis::log_level::error,
            fmt::format("connection attempt failed {0}", err.message()));
    }

    int attempts = 1;
    cpool::timer timer(exec);
    while (!connection_->connected()) {
        auto delay = cpool::timer_delay(++attempts);
        log_message(
            redis::log_level::info,
            fmt::format("connection failed; waiting {0}ms", delay.count()));

        co_await timer.async_wait(delay);

        log_message(redis::log_level::info,
                    fmt::format("attempting connection to: {0}:{1}",
                                connection_->host(), connection_->port()));
        auto error = co_await connection_->async_connect();
        if (error.error_code() == net::error::operation_aborted) {
            connecting_ = false;
            co_return error;
        }
        if (error) {
            log_message(
                redis::log_level::error,
                fmt::format("connection attempt failed {0}", err.message()));
        }
    }

    log_message(redis::log_level::info,
                fmt::format("connected to: {0}:{1}", connection_->host(),
                            connection_->port()));
    connecting_ = false;
    co_return cpool::no_error;
}

void redis_subscriber_connection::log_message(log_level level,
                                              string_view message) {
    if (on_log_) {
        on_log_(level, message);
    }
}

} // namespace redis