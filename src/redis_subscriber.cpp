#include "redis_subscriber.hpp"

namespace redis {

redis_subscriber::redis_subscriber(
    std::unique_ptr<cpool::tcp_connection> connection)
    : connection_(std::move(connection))
    , message_queue_(connection_.get_executor(), 8)
    , on_log_()
    , tasks_cv_(connection_.get_executor())
    , running_tasks_(0) {}

redis_subscriber::redis_subscriber(net::any_io_executor exec, string host,
                                   uint16_t port)
    : connection_(std::make_unique<cpool::tcp_connection>(exec, host, port))
    , message_queue_(exec, 8)
    , on_log_()
    , tasks_cv_(exec)
    , running_tasks_(0) {}

awaitable<cpool::error> redis_subscriber::ping() {
    if (!running()) {
        co_return std::error_code(redis_client_error_code::disconnected);
    }
    auto error = co_await send(redis_command("PING"));
    co_return error;
}

awaitable<cpool::error> redis_subscriber::subscribe(string channel) {
    log_message(log_level::debug, fmt::format("Subscribing to {0}", channel));
    return send(redis_command(std::vector<string>{"SUBSCRIBE", channel}));
}

awaitable<cpool::error> redis_subscriber::unsubscribe(string channel) {
    log_message(log_level::debug, fmt::format("Unsubscribing to {0}", channel));
    return send(redis_command(std::vector<string>{"UNSUBSCRIBE", channel}));
}

awaitable<cpool::error> redis_subscriber::psubscribe(string pattern) {
    log_message(log_level::debug, fmt::format("Psubscribing to {0}", pattern));
    return send(redis_command(std::vector<string>{"PSUBSCRIBE", pattern}));
}

awaitable<cpool::error> redis_subscriber::punsubscribe(string pattern) {
    log_message(log_level::debug,
                fmt::format("Punsubscribing to {0}", pattern));
    return send(redis_command(std::vector<string>{"PUNSUBSCRIBE", pattern}));
}

void redis_subscriber::start() {
    if (running()) {
        return;
    }

    running_tasks_++;
    co_spawn(connection_.get_executor(),
             std::bind(&redis_subscriber::read_messages, this), detached);
    log_message(log_level::debug, "monitoring for messages");
}

awaitable<void> redis_subscriber::stop() {
    message_queue_.close();
    connection_.cancel();
    co_await tasks_cv_.async_wait([&]() { return running_tasks_ == 0; });
    co_await connection_.async_disconnect();
}

awaitable<cpool::error> redis_subscriber::reset() {
    log_message(log_level::debug, fmt::format("Reseting subscriptions"));
    return send(redis_command("RESET"));
}

// Send Commands
awaitable<cpool::error> redis_subscriber::send(redis_command command) {
    cout << "getting connection for send" << endl;
    auto conn = co_await connection_.get();
    cout << "got connection for send" << endl;
    auto buffer = command.serialized_command();
    auto [write_error, bytes_written] =
        co_await conn->async_write(asio::buffer(buffer));
    if (write_error) {
        co_return write_error;
    }
    if (bytes_written != buffer.size()) {
        co_return redis_client_error_code::write_error;
    }

    co_return cpool::no_error;
}

awaitable<void> redis_subscriber::read_messages() {
    log_message(log_level::trace, "starting to read messages");
    buffer_t read_buffer(4096);

    while (true) {
        cpool::error_code err;
        cout << "getting connection" << endl;
        auto conn = co_await connection_.get();

        cout << "wait for read" << endl;
        co_await conn->socket().async_wait(
            net::ip::tcp::socket::wait_read,
            net::redirect_error(use_awaitable, err));
        if (err == net::error::operation_aborted) {
            break;
        }

        cout << "reading" << endl;
        auto [read_error, bytes_read] =
            co_await conn->async_read_some(asio::buffer(read_buffer));
        if (read_error.error_code() == (int)net::error::operation_aborted) {
            cout << "cancelled, wrapping up" << endl;
            break;
        }
        if (read_error || bytes_read == 0) {
            cout << "unexpected error" << endl;
            log_message(
                log_level::error,
                std::error_code(redis_client_error_code::read_error).message());
            continue;
        }

        err = co_await parse_buffer(read_buffer.cbegin(),
                                    read_buffer.cbegin() + bytes_read);
        if (err) {
            break;
        }
    }

    running_tasks_--;
    tasks_cv_.notify_all();
    co_return;
}

awaitable<cpool::error_code>
redis_subscriber::parse_buffer(buffer_t::const_iterator begin,
                               buffer_t::const_iterator end) {
    auto it = begin;
    while (it != end) {
        redis_reply reply;
        it = reply.load_data(it, end);

        auto ec = channels::error_code();
        auto tok = asio::redirect_error(asio::use_awaitable, ec);
        co_await message_queue_.async_send(reply, tok);
        if (ec == channels::errors::channel_closed) {
            cout << "channel closed, wrapping up" << endl;
            co_return ec;
        }
    }

    co_return cpool::error_code();
}

awaitable<redis_reply> redis_subscriber::read() {
    auto ec = channels::error_code();
    auto tok = asio::redirect_error(asio::use_awaitable, ec);

    auto reply = co_await message_queue_.async_consume(tok);
    if (ec) {
        co_return redis_reply(ec);
    }

    co_return reply;
}

void redis_subscriber::set_logging_handler(logging_handler handler) {
    on_log_ = std::move(handler);
    connection_.set_logging_handler(on_log_);
}

bool redis_subscriber::running() const { return (running_tasks_ != 0); }

// Private functions
void redis_subscriber::log_message(log_level level, string_view message) {
    if (on_log_) {
        on_log_(level, message);
    }
}

} // namespace redis
