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

namespace {

using string = std::string;
using namespace redis;

const std::string DEFAULT_REDIS_HOST = "host.docker.internal";

std::optional<std::string> get_env_var(std::string const& key) {
    char* val = getenv(key.c_str());
    return (val == NULL) ? std::nullopt : std::optional(std::string(val));
}

void testForError(std::string cmd, const redis::reply& reply) {
    EXPECT_FALSE(reply.error()) << cmd << reply.error().message();
    if (reply.error() == client_error_code::error) {
        EXPECT_EQ(reply.value().as<redis::error>().value().what(),
                  std::string())
            << cmd;
    }
}

void testForValue(std::string cmd, redis::reply reply, string expected) {
    testForError(cmd, reply);

    auto optString = reply.value().as<string>();
    EXPECT_TRUE(optString.has_value());
    EXPECT_EQ(optString.value(), expected);
}

void testForValue(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value());
    EXPECT_EQ(optInt.value(), expected);
}

void testForValue(std::string cmd, redis::reply reply, float expected) {
    testForError(cmd, reply);

    value value = reply.value();
    auto optFloat = value.as<float>();

    EXPECT_TRUE(optFloat.has_value());
    EXPECT_FLOAT_EQ(optFloat.value(), expected);
}

void testForValue(std::string cmd, redis::reply reply, double expected) {
    testForError(cmd, reply);

    value value = reply.value();
    auto optDouble = value.as<double>();

    EXPECT_TRUE(optDouble.has_value());
    EXPECT_DOUBLE_EQ(optDouble.value(), expected);
}

void testForArray(std::string cmd, redis::reply reply, redis_array expected) {
    testForError(cmd, reply);

    value value = reply.value();
    auto optArray = value.as<redis_array>();

    EXPECT_TRUE(optArray.has_value());
    auto array_value = optArray.value();
    ASSERT_EQ(array_value.size(), expected.size()) << cmd;
    for (int i = 0; i < array_value.size(); i++) {
        ASSERT_EQ(array_value[i].type(), expected[i].type()) << cmd;
        ASSERT_TRUE(array_value[i] == expected[i]) << cmd;
    }
}

void testForSuccess(std::string cmd, redis::reply reply) {
    testForError(cmd, reply);

    value value = reply.value();

    EXPECT_TRUE(value.as<bool>().value_or(false)) << cmd;
}

void testForType(std::string cmd, redis::reply reply, redis::redis_type type) {
    testForError(cmd, reply);

    EXPECT_EQ(reply.value().type(), type);
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

awaitable<void> publish_messages(client& client, std::string channel,
                                 int max_messages) {
    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < max_messages; i++) {
        reply = co_await client.send(publish(channel, std::to_string(i)));
        testForError("PUBLISH", reply);
    }
}

awaitable<void> run_tests(asio::io_context& ctx) {
    std::atomic<int> barrier;
    auto exec = co_await cpool::net::this_coro::executor;
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(logMessage, log_level::trace,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
    redis_subscriber subscriber(exec, host, 6379);
    subscriber.set_logging_handler(std::bind(logMessage, log_level::trace,
                                             std::placeholders::_1,
                                             std::placeholders::_2));

    subscriber.start();
    auto error = co_await subscriber.ping();
    EXPECT_FALSE(error) << error.message();
    auto reply = co_await subscriber.read();
    testForValue("PING", reply, "PONG");

    EXPECT_TRUE(subscriber.running());

    string channel = "stuff";
    string pattern = "*st*";

    error = co_await subscriber.subscribe(channel);
    EXPECT_FALSE(error) << error.message();
    reply = co_await subscriber.read();
    testForArray("SUBSCRIBE", reply,
                 redis_array{value(string_to_vector("subscribe")),
                             value(string_to_vector(channel)), value(1)});
    error = co_await subscriber.psubscribe(pattern);
    EXPECT_FALSE(error) << error.message();
    reply = co_await subscriber.read();
    testForArray("PSUBSCRIBE", reply,
                 redis_array{value(string_to_vector("psubscribe")),
                             value(string_to_vector(pattern)), value(2)});

    // publish messages
    co_spawn(exec, publish_messages(client, channel, 50), detached);
    int messages = 0;
    while (messages < 50) {
        cout << "Reading messages: " << messages << endl;

        // we will get two messages for each publish because "stuff" matches the
        // subscribe and the psubscribe
        for (int i = 0; i < 2; i++) {
            reply = co_await subscriber.read();
            testForError("message_read", reply);

            auto message = reply.value().as<redis_message>();
            EXPECT_TRUE(message.has_value());
            EXPECT_EQ(message.value().contents, std::to_string(messages));
        }

        messages++;
    }
    cout << "All messages received" << endl;

    // // shut down
    error = co_await subscriber.unsubscribe(channel);
    EXPECT_FALSE(error) << error.message();
    reply = co_await subscriber.read();
    testForArray("UNSUBSCRIBE", reply,
                 redis_array{value(string_to_vector("unsubscribe")),
                             value(string_to_vector(channel)), value(1)});
    error = co_await subscriber.punsubscribe(pattern);
    EXPECT_FALSE(error) << error.message();
    reply = co_await subscriber.read();
    testForArray("PUNSUBSCRIBE", reply,
                 redis_array{value(string_to_vector("punsubscribe")),
                             value(string_to_vector(pattern)), value(0)});

    cout << "stopping reader" << endl;
    co_await subscriber.stop();

    ctx.stop();
}

TEST(SubTest, Subscribe) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

} // namespace