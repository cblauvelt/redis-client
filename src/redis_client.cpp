#include "redis_client.hpp"

#include <chrono>

namespace redis {

redis_client::redis_client(cpool::net::any_io_executor exec,
                           redis_client_config config)
    : exec_(std::move(exec))
    , config_(config)
    , con_pool_(nullptr)
    , on_log_() {

    auto conn_creator = [&]() -> std::unique_ptr<cpool::tcp_connection> {
        auto conn = std::make_unique<cpool::tcp_connection>(exec_, config_.host,
                                                            config_.port);
        if (!config_.password.empty()) {
            if (config_.username.empty()) {
                config_.username = "default";
            }

            // login when a connection is created
            conn->set_state_change_handler(
                std::bind(&redis_client::auth_client, this,
                          std::placeholders::_1, std::placeholders::_2));
        }

        return conn;
    };

    con_pool_ = std::make_unique<cpool::connection_pool<cpool::tcp_connection>>(
        exec_, conn_creator, config_.max_connections);
}

redis_client::redis_client(cpool::net::any_io_executor exec, string host,
                           uint16_t port)
    : exec_(std::move(exec))
    , config_()
    , con_pool_()
    , on_log_() {

    config_.host = host;
    config_.port = port;

    auto conn_creator = [&]() -> std::unique_ptr<cpool::tcp_connection> {
        return std::make_unique<cpool::tcp_connection>(exec_, config_.host,
                                                       config_.port);
    };

    con_pool_ = std::make_unique<cpool::connection_pool<cpool::tcp_connection>>(
        exec_, conn_creator, config_.max_connections);
}

void redis_client::set_config(redis_client_config config) {
    config_ = config;

    auto conn_creator = [&]() -> std::unique_ptr<cpool::tcp_connection> {
        auto conn = std::make_unique<cpool::tcp_connection>(exec_, config_.host,
                                                            config_.port);
        if (!config_.password.empty()) {
            if (config_.username.empty()) {
                config_.username = "default";
            }

            // login when a connection is created
            conn->set_state_change_handler(
                std::bind(&redis_client::auth_client, this,
                          std::placeholders::_1, std::placeholders::_2));
        }

        return conn;
    };

    con_pool_ = std::make_unique<cpool::connection_pool<cpool::tcp_connection>>(
        exec_, conn_creator, config_.max_connections);
}

redis_client_config redis_client::config() const { return config_; }

awaitable<redis_reply> redis_client::ping() {
    return send(redis_command("PING"));
}

// Send Commands
awaitable<redis_reply> redis_client::send(redis_command command) {
    auto connection = co_await con_pool_->get_connection();

    auto reply = co_await send(connection, command);

    con_pool_->release_connection(connection);

    co_return reply;
}

awaitable<redis_reply> redis_client::send(cpool::tcp_connection* connection,
                                          redis_command command) {
    auto buffer = command.serialized_command();
    auto [write_error, bytes_written] =
        co_await connection->async_write(asio::buffer(buffer));
    if (write_error || bytes_written != buffer.size()) {
        co_return redis_client_error_code::write_error;
    }

    std::vector<uint8_t> read_buffer(4096);
    auto [read_error, bytes_read] =
        co_await connection->async_read_some(asio::buffer(read_buffer));
    if (read_error || bytes_read == 0) {
        co_return redis_client_error_code::read_error;
    }
    redis_reply reply;
    reply.load_data(read_buffer.cbegin(), read_buffer.cbegin() + bytes_read);

    co_return reply;
}

awaitable<cpool::error>
redis_client::auth_client(cpool::tcp_connection* conn,
                          const cpool::client_connection_state state) {

    if (state == cpool::client_connection_state::connected) {
        auto username = this->config().username;
        auto password = this->config().password;
        auto loginCmd =
            redis_command(std::vector<std::string>{"AUTH", username, password});

        this->log_message(redis::log_level::trace, "AUTH password");
        auto reply = co_await this->send(conn, loginCmd);
        if (reply.error()) {
            this->log_message(redis::log_level::error, reply.error().message());
        }
        co_return reply.error();
    }

    co_return cpool::error();
}

void redis_client::set_logging_handler(logging_handler handler) {
    on_log_ = std::move(handler);
}

bool redis_client::running() const { return (con_pool_->size() != 0); }

// Private functions

void redis_client::log_message(log_level level, string_view message) {
    if (on_log_) {
        on_log_(level, message);
    }
}

} // namespace redis
