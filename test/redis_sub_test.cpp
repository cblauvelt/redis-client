#include <iostream>
#include <string>
#include <vector>

#include "redis/client.hpp"
#include "redis/commands.hpp"
#include "redis/error.hpp"
#include "redis/errors.hpp"
#include "redis/subscriber.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_functions.hpp"

namespace {

using string = std::string;
using namespace redis;

const std::string DEFAULT_REDIS_HOST = "host.docker.internal";
const std::string DEFAULT_REDIS_PORT = "6379";
const log_level logLevel = log_level::debug;

std::optional<std::string> get_env_var(std::string const& key) {
    char* val = getenv(key.c_str());
    return (val == NULL) ? std::nullopt : std::optional(std::string(val));
}

void logMessage(log_level target, log_level level, string_view message) {
    if (target > level) {
        return;
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
    std::cout << fmt::format("[Redis] [{0}] {1}", levelString, message)
              << std::endl;
}

awaitable<void> publish_messages(std::unique_ptr<redis::client> client,
                                 std::string channel, int max_messages) {
    auto reply = co_await client->ping();
    testForValue("PING", reply, "PONG");
    EXPECT_TRUE(client->running());

    for (int i = 0; i < max_messages; i++) {
        reply = co_await client->send(publish(channel, std::to_string(i)));
        testForError("PUBLISH", reply);
    }
}

awaitable<void> run_tests(asio::io_context& ctx, std::unique_ptr<client> client,
                          std::unique_ptr<redis_subscriber> subscriber) {
    try {
        subscriber->start();
        auto error = co_await subscriber->ping();
        EXPECT_FALSE(error) << error.message();
        auto reply = co_await subscriber->read();
        testForValue("PING", reply, "PONG");
        if (reply.error()) {
            logMessage(logLevel, redis::log_level::critical,
                       fmt::format("Could not ping server: {}",
                                   reply.value().as<std::string>().value()));
            co_return;
        }

        EXPECT_TRUE(subscriber->running());

        string channel = "stuff";
        string pattern = "*st*";

        error = co_await subscriber->subscribe(channel);
        EXPECT_FALSE(error) << error.message();
        reply = co_await subscriber->read();
        testForArray("SUBSCRIBE", reply,
                     redis_array{value(string_to_vector("subscribe")),
                                 value(string_to_vector(channel)), value(1)});
        error = co_await subscriber->psubscribe(pattern);
        EXPECT_FALSE(error) << error.message();
        reply = co_await subscriber->read();
        testForArray("PSUBSCRIBE", reply,
                     redis_array{value(string_to_vector("psubscribe")),
                                 value(string_to_vector(pattern)), value(2)});

        // publish messages
        co_spawn(ctx.get_executor(),
                 publish_messages(std::move(client), channel, 50), detached);
        int messages = 0;
        while (messages < 50) {
            logMessage(logLevel, log_level::debug,
                       fmt::format("Reading messages: {}", messages));

            // we will get two messages for each publish because "stuff" matches
            // the subscribe and the psubscribe
            for (int i = 0; i < 2; i++) {
                reply = co_await subscriber->read();
                testForError("message_read", reply);

                auto message = reply.value().as<redis_message>();
                EXPECT_TRUE(message.has_value());
                EXPECT_EQ(message.value().contents, std::to_string(messages));
            }

            messages++;
        }
        logMessage(logLevel, log_level::debug, "All messages received");

        // shut down
        error = co_await subscriber->unsubscribe(channel);
        EXPECT_FALSE(error) << error.message();
        reply = co_await subscriber->read();
        testForArray("UNSUBSCRIBE", reply,
                     redis_array{value(string_to_vector("unsubscribe")),
                                 value(string_to_vector(channel)), value(1)});
        error = co_await subscriber->punsubscribe(pattern);
        EXPECT_FALSE(error) << error.message();
        reply = co_await subscriber->read();
        testForArray("PUNSUBSCRIBE", reply,
                     redis_array{value(string_to_vector("punsubscribe")),
                                 value(string_to_vector(pattern)), value(0)});
    } catch (const std::exception& e) {
        logMessage(logLevel, log_level::error,
                   fmt::format("an error occurred: {}", e.what()));
        EXPECT_TRUE(false);
    }

    logMessage(logLevel, log_level::debug, "stopping reader");
    co_await subscriber->stop();

    ctx.stop();
    co_return;
}

TEST(Subscribe, SubscribeTest) {
    asio::io_context ctx(1);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);
    auto client =
        std::make_unique<redis::client>(ctx.get_executor(), host, 6379);
    client->set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));
    auto subscriber =
        make_unique<redis_subscriber>(ctx.get_executor(), host, 6379);
    subscriber->set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    cpool::co_spawn(
        ctx, run_tests(std::ref(ctx), std::move(client), std::move(subscriber)),
        cpool::detached);

    ctx.run();
}

TEST(Subscribe, SubscribePasswordTest) {
    asio::io_context ctx(1);
    auto host = get_env_var("REDIS_PASSWORD_HOST").value_or(DEFAULT_REDIS_HOST);
    auto portString =
        get_env_var("REDIS_PASSWORD_PORT").value_or(DEFAULT_REDIS_PORT);
    int port = std::stoi(portString);
    auto password = get_env_var("REDIS_PASSWORD").value_or("");
    auto config = client_config{}
                      .set_host(host)
                      .set_port(port)
                      .set_password(password)
                      .set_max_connections(1);

    logMessage(logLevel, redis::log_level::info,
               fmt::format("Logging into {}:{} with password {}", config.host,
                           config.port, config.password));

    auto client = std::make_unique<redis::client>(ctx.get_executor(), config);
    client->set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));
    auto subscriber = make_unique<redis_subscriber>(ctx.get_executor(), config);
    subscriber->set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    cpool::co_spawn(
        ctx, run_tests(std::ref(ctx), std::move(client), std::move(subscriber)),
        cpool::detached);

    ctx.run();
}

} // namespace