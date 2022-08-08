#include <iostream>
#include <string>
#include <vector>

#include <cpool/awaitable_latch.hpp>

#include "redis/error.hpp"
#include "redis/errors.hpp"
#include "redis/subscriber_connection.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using string = std::string;
using namespace redis;

const std::string DEFAULT_REDIS_HOST = "host.docker.internal";

std::optional<std::string> get_env_var(std::string const& key) {
    char* val = getenv(key.c_str());
    return (val == NULL) ? std::nullopt : std::optional(std::string(val));
}

awaitable<void> logMessage(log_level target, log_level level,
                           string_view message) {
    if (target > level) {
        co_return;
    }

    std::string levelString;

    switch (level) {
    case log_level::trace:
        levelString = "Trace";
        break;
    case log_level::debug:
        levelString = "Debug";
        break;
    case log_level::info:
        levelString = "Info";
        break;
    case log_level::warn:
        levelString = "Warn";
        break;
    case log_level::error:
        levelString = "Error";
        break;
    case log_level::critical:
        levelString = "Critical";
        break;
    default:
        break;
    }
    std::cout << fmt::format("{0}: {1}", levelString, message) << std::endl;
}

awaitable<void> connect_and_hold(redis_subscriber_connection& connection,
                                 cpool::awaitable_latch& barrier) {
    auto conn = co_await connection.get();
    cpool::timer timer(connection.get_executor());
    co_await timer.async_wait(100ms);
    barrier.count_down();
}

awaitable<void> run_tests(asio::io_context& ctx) {
    auto exec = co_await cpool::net::this_coro::executor;
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);
    cpool::awaitable_latch latch(exec, 1);

    redis_subscriber_connection connection(exec, host, 6379);
    connection.set_logging_handler(std::bind(logMessage, log_level::info,
                                             std::placeholders::_1,
                                             std::placeholders::_2));

    co_spawn(exec, connect_and_hold(connection, latch), detached);

    auto conn = co_await connection.get();
    EXPECT_TRUE(connection.connected());

    co_await latch.wait();

    auto error = co_await connection.async_disconnect();
    EXPECT_FALSE(error);

    ctx.stop();
}

TEST(SubConnection, Subscribe) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

} // namespace