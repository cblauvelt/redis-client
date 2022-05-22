#include "redis/client.hpp"

#include <chrono>

#include <absl/cleanup/cleanup.h>

namespace redis {

client::client(cpool::net::any_io_executor exec, client_config config)
    : exec_(std::move(exec))
    , config_(config)
    , con_pool_(nullptr)
    , on_log_(nullptr) {

    con_pool_ = std::make_unique<cpool::connection_pool<cpool::tcp_connection>>(
        exec_, std::bind(&client::connection_ctor, this),
        config_.max_connections);
}

client::client(cpool::net::any_io_executor exec, string host, uint16_t port)
    : exec_(std::move(exec))
    , config_()
    , con_pool_(nullptr)
    , on_log_(nullptr) {

    config_.host = host;
    config_.port = port;

    con_pool_ = std::make_unique<cpool::connection_pool<cpool::tcp_connection>>(
        exec_, std::bind(&client::connection_ctor, this),
        config_.max_connections);
}

void client::set_config(client_config config) {
    config_ = config;

    con_pool_ = std::make_unique<cpool::connection_pool<cpool::tcp_connection>>(
        exec_, std::bind(&client::connection_ctor, this),
        config_.max_connections);
}

client_config client::config() const { return config_; }

awaitable<reply> client::ping() { return send(command("PING")); }

// Send Commands
awaitable<reply> client::send(command command) {
    log_message(redis::log_level::trace,
                fmt::format("getting connection - connections {} - idle {}",
                            con_pool_->size(), con_pool_->size_idle()));
    auto connection = co_await con_pool_->get_connection();
    if (connection == nullptr) {
        co_return reply(redis::client_error_code::client_stopped);
    }

    auto defer_release = absl::Cleanup([&]() {
        if (connection != nullptr) {
            connection->expires_never();

            con_pool_->release_connection(connection);
        }
    });

    auto reply = co_await send(connection, command);

    co_return reply;
}

awaitable<replies> client::send(commands commands) {
    log_message(redis::log_level::trace,
                fmt::format("getting connection - connections {} - idle {}",
                            con_pool_->size(), con_pool_->size_idle()));
    auto connection = co_await con_pool_->get_connection();
    if (connection == nullptr) {
        co_return redis::replies(
            commands.size(),
            redis::reply{redis::client_error_code::client_stopped});
    }

    auto defer_release = absl::Cleanup([&]() {
        if (connection != nullptr) {
            connection->expires_never();

            con_pool_->release_connection(connection);
        }
    });

    auto replies = co_await send(connection, commands);

    co_return replies;
}

awaitable<reply> client::send(cpool::tcp_connection* connection,
                              command command) {
    auto buffer = command.serialized_command();
    auto [write_error, bytes_written] =
        co_await connection->async_write(asio::buffer(buffer));
    if (write_error || bytes_written != buffer.size()) {
        co_return client_error_code::write_error;
    }

    std::vector<uint8_t> read_buffer(4096);
    auto [read_error, bytes_read] =
        co_await connection->async_read_some(asio::buffer(read_buffer));
    if (read_error || bytes_read == 0) {
        co_return client_error_code::read_error;
    }
    reply reply;
    reply.load_data(read_buffer.cbegin(), read_buffer.cbegin() + bytes_read);

    co_return reply;
}

awaitable<replies> client::send(cpool::tcp_connection* connection,
                                commands commands) {
    std::string buffer;
    for (auto& command : commands) {
        buffer += command.serialized_command();
    }

    auto [write_error, bytes_written] =
        co_await connection->async_write(asio::buffer(buffer));
    if (write_error || bytes_written != buffer.size()) {
        co_return redis::replies(
            commands.size(),
            redis::reply{redis::client_error_code::write_error});
    }

    std::vector<uint8_t> read_buffer(4096);
    auto [read_error, bytes_read] =
        co_await connection->async_read_some(asio::buffer(read_buffer));
    if (read_error || bytes_read == 0) {
        co_return redis::replies(
            commands.size(),
            redis::reply{redis::client_error_code::read_error});
    }

    redis::replies replies;
    redis::buffer_t::const_iterator it = read_buffer.cbegin();
    size_t bytes_remaining = bytes_read;
    for (int i = 0; i < commands.size(); i++) {
        redis::reply reply;
        it = reply.load_data(it, it + bytes_remaining);
        bytes_remaining = it - read_buffer.cbegin();
        replies.push_back(reply);
    }

    co_return replies;
}

std::unique_ptr<cpool::tcp_connection> client::connection_ctor() {

    auto conn = std::make_unique<cpool::tcp_connection>(exec_, config_.host,
                                                        config_.port);
    if (!config_.password.empty()) {
        if (config_.username.empty()) {
            config_.username = "default";
        }

        // login when a connection is created
        conn->set_state_change_handler(std::bind(&client::auth_client, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));
    }

    return conn;
}

[[nodiscard]] awaitable<cpool::error>
client::on_connection_state_change(cpool::tcp_connection* conn,
                                   const cpool::client_connection_state state) {
    switch (state) {
    case cpool::client_connection_state::disconnected:
        log_message(log_level::info, fmt::format("disconnected from {0}:{1}",
                                                 conn->host(), conn->port()));
        break;

    case cpool::client_connection_state::resolving:
        log_message(log_level::info,
                    fmt::format("resolving {0}", conn->host()));
        break;

    case cpool::client_connection_state::connecting:
        log_message(log_level::info, fmt::format("connecting to {0}:{1}",
                                                 conn->host(), conn->port()));
        break;

    case cpool::client_connection_state::connected:
        log_message(log_level::info, fmt::format("connected to {0}:{1}",
                                                 conn->host(), conn->port()));
        break;

    case cpool::client_connection_state::disconnecting:
        log_message(log_level::info, fmt::format("disconnecting from {0}:{1}",
                                                 conn->host(), conn->port()));
        break;

    default:
        log_message(
            log_level::warn,
            fmt::format("unknown client_connection_state: {0}", (int)state));
    }

    co_return cpool::error();
}

awaitable<cpool::error>
client::auth_client(cpool::tcp_connection* conn,
                    const cpool::client_connection_state state) {

    if (state == cpool::client_connection_state::connected) {
        auto username = this->config().username;
        auto password = this->config().password;
        auto loginCmd =
            command(std::vector<std::string>{"AUTH", username, password});

        this->log_message(redis::log_level::trace, "AUTH password");
        auto reply = co_await this->send(conn, loginCmd);
        if (reply.error()) {
            this->log_message(redis::log_level::error, reply.error().message());
        }
        co_return reply.error();
    }

    auto error = co_await on_connection_state_change(conn, state);
    if (error) {
        log_message(
            log_level::error,
            fmt::format("error while executing on_state_change_handler: {}",
                        error.message()));
    }

    co_return cpool::error();
}

void client::set_logging_handler(logging_handler handler) {
    on_log_ = std::move(handler);
}

bool client::running() const { return (con_pool_->size() != 0); }

// Private functions

void client::log_message(log_level level, string_view message) {
    if (on_log_) {
        on_log_(level, message);
    }
}

} // namespace redis
