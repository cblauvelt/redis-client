#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "commands-json.hpp"
#include "commands.hpp"
#include "redis_client.hpp"
#include "redis_command.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace redis;

const std::string DEFAULT_REDIS_HOST = "host.docker.internal";
const std::string DEFAULT_REDIS_PORT = "6379";

std::optional<std::string> get_env_var(std::string const& key) {
    char* val = getenv(key.c_str());
    return (val == NULL) ? std::nullopt : std::optional(std::string(val));
}

void testForError(std::string cmd, const redis::redis_reply& reply) {
    EXPECT_FALSE(reply.error()) << cmd;
    if (reply.error() == redis_client_error_code::redis_error) {
        EXPECT_EQ(reply.value().as<redis_error>().value().what(), std::string())
            << cmd;
    }
}

void testForString(std::string cmd, redis::redis_reply reply, string expected) {
    testForError(cmd, reply);

    auto optString = reply.value().as<string>();
    EXPECT_TRUE(optString.has_value()) << "Expected string: " << expected;
    EXPECT_EQ(optString.value(), expected) << "Expected string: " << expected;
}

void testForInt(std::string cmd, redis::redis_reply reply, int expected) {
    testForError(cmd, reply);

    redis_value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value());
    EXPECT_EQ(optInt.value(), expected);
}

void testForFloat(std::string cmd, redis::redis_reply reply, float expected) {
    testForError(cmd, reply);

    redis_value value = reply.value();
    auto optFloat = value.as<float>();

    EXPECT_TRUE(optFloat.has_value());
    EXPECT_FLOAT_EQ(optFloat.value(), expected);
}

void testForDouble(std::string cmd, redis::redis_reply reply, double expected) {
    testForError(cmd, reply);

    redis_value value = reply.value();
    auto optDouble = value.as<double>();

    EXPECT_TRUE(optDouble.has_value());
    EXPECT_DOUBLE_EQ(optDouble.value(), expected);
}

void testForArray(std::string cmd, redis::redis_reply reply,
                  redis_array expected) {
    testForError(cmd, reply);

    redis_value value = reply.value();
    auto optArray = value.as<redis_array>();

    EXPECT_TRUE(optArray.has_value());
    EXPECT_EQ(optArray.value(), expected);
}

void testForSuccess(std::string cmd, redis::redis_reply reply) {
    testForError(cmd, reply);

    redis_value value = reply.value();

    EXPECT_TRUE(value.as<bool>().value_or(false)) << cmd;
}

void testForType(std::string cmd, redis::redis_reply reply,
                 redis::redis_type type) {
    testForError(cmd, reply);

    EXPECT_EQ(reply.value().type(), type);
}

void logMessage(log_level level, string_view message) {
    cout << message << endl;
}

awaitable<void> test_basic(redis_client& client, int c,
                           std::atomic<int>& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key = std::string("foo") + std::to_string(c);
    redis::redis_reply reply;

    for (int i = 0; i < 2; i++) {
        reply = co_await client.send(redis::set(key, "42"));
        testForSuccess("SET", reply);

        reply = co_await client.send(get(key));
        testForInt("GET", reply, 42);

        reply = co_await client.send(exists(key));
        testForSuccess("EXISTS", reply);

        reply = co_await client.send(del(key));
        testForSuccess("DEL", reply);

        reply = co_await client.send(get(key));
        testForType("GET", reply, redis_type::nil);

        reply = co_await client.send(publish(key, "stuff" + to_string(i)));
        testForType("PUBLISH", reply, redis_type::integer);
    }

    barrier--;
    co_return;
}

awaitable<void> run_tests(asio::io_context& ctx) {
    std::atomic<int> barrier;
    auto exec = co_await cpool::net::this_coro::executor;
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(redis::log_level::info, host);
    redis_client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    barrier = 50;
    int num_runners = barrier;
    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_basic(client, i, barrier), cpool::detached);
    }

    while (barrier != 0) {
        cpool::timer timer(exec);
        co_await timer.async_wait(100ms);
    }

    ctx.stop();
    co_return;
}

awaitable<void> run_password_tests(asio::io_context& ctx) {
    std::atomic<int> barrier;
    auto exec = co_await cpool::net::this_coro::executor;
    auto host = get_env_var("REDIS_PASSWORD_HOST").value_or(DEFAULT_REDIS_HOST);
    auto portString =
        get_env_var("REDIS_PASSWORD_PORT").value_or(DEFAULT_REDIS_PORT);
    int port = std::stoi(portString);
    auto password = get_env_var("REDIS_PASSWORD").value_or("");
    auto config =
        redis_client_config{}.set_host(host).set_port(port).set_password(
            password);

    logMessage(redis::log_level::info,
               fmt::format("Logging into {}:{} with password {}", config.host,
                           config.port, config.password));

    redis_client client(exec, config);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    barrier = 2;
    int num_runners = barrier;
    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_basic(client, i, barrier), cpool::detached);
    }

    while (barrier != 0) {
        cpool::timer timer(exec);
        co_await timer.async_wait(100ms);
    }

    ctx.stop();
    co_return;
}

awaitable<void> test_json(redis_client& client, int c,
                          std::atomic<int>& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key1 = std::string("object") + std::to_string(c);
    string key2 = std::string("amoreinterestingexample") + std::to_string(c);
    string key3 = std::string("arr") + std::to_string(c);
    redis::redis_reply reply;

    for (int i = 0; i < 2; i++) {
        reply = co_await client.send(
            json_set(key1, ".", "{\"foo\": \"bar\", \"ans\": 0}"));
        testForSuccess("JSON_SET", reply);

        reply = co_await client.send(json_get(key1, ".ans"));
        testForInt("JSON_GET", reply, 0);

        reply = co_await client.send(json_type(key1, ".foo"));
        testForString("JSON_TYPE", reply, "string");

        reply = co_await client.send(json_strlen(key1, ".foo"));
        testForInt("JSON_STRLEN", reply, 3);

        reply = co_await client.send(json_strappend(key1, ".foo", "\"baz\""));
        testForInt("JSON_STRAPPEND", reply, 6);

        reply = co_await client.send(json_get(key1, ".foo"));
        testForString("JSON_GET", reply, "\"barbaz\"");

        reply = co_await client.send(json_numincrby(key1, ".ans", "1"));
        testForInt("JSON_NUMINCRBY", reply, 1);

        reply = co_await client.send(json_numincrby(key1, ".ans", "1.5"));
        testForFloat("JSON_NUMINCRBY", reply, 2.5F);
        testForDouble("JSON_NUMINCRBY", reply, 2.5);

        reply = co_await client.send(json_numincrby(key1, ".ans", "-0.75"));
        testForFloat("JSON_NUMINCRBY", reply, 1.75F);
        testForDouble("JSON_NUMINCRBY", reply, 1.75);

        reply = co_await client.send(json_nummultby(key1, ".ans", "24"));
        testForInt("JSON_NUMMULRBY", reply, 42);

        reply = co_await client.send(json_del(key1, "."));
        testForSuccess("JSON_DEL", reply);

        reply = co_await client.send(
            json_set(key2, ".", "[ true, { \"answer\": 42 }, null ]"));
        testForSuccess("JSON_SET", reply);

        reply = co_await client.send(json_get(key2, "."));
        testForString("JSON_GET", reply, "[true,{\"answer\":42},null]");

        reply = co_await client.send(json_get(key2, "[1].answer"));
        testForInt("JSON_GET", reply, 42);

        reply = co_await client.send(json_del(key2, "[-1]"));
        testForSuccess("JSON_DEL", reply);

        reply = co_await client.send(json_get(key2, "."));
        testForString("JSON_GET", reply, "[true,{\"answer\":42}]");

        reply = co_await client.send(json_del(key2, "."));
        testForSuccess("JSON_DEL", reply);

        // Test array functions
        reply = co_await client.send(json_set(key3, ".", "[]"));
        testForSuccess("JSON_SET", reply);

        reply = co_await client.send(json_arrappend(key3, ".", "0"));
        testForInt("JSON.ARRAPPEND", reply, 1);

        reply = co_await client.send(json_get(key3, "."));
        testForString("JSON.GET", reply, "[0]");

        std::vector<string> values{"0", "-2", "-1"};
        reply = co_await client.send(json_arrinsert(key3, ".", values));
        testForInt("JSON.ARRINSERT", reply, 3);

        reply = co_await client.send(json_get(key3, "."));
        testForString("JSON.GET", reply, "[-2,-1,0]");

        reply = co_await client.send(json_arrtrim(key3, ".", "1", "1"));
        testForInt("JSON.ARRTRIM", reply, 1);

        reply = co_await client.send(json_get(key3, "."));
        testForString("JSON.GET", reply, "[-1]");

        reply = co_await client.send(json_arrpop(key3, "."));
        testForString("JSON.ARRPOP", reply, "-1");

        reply = co_await client.send(json_arrpop(key3, "."));
        testForType("JSON.ARRPOP", reply, redis_type::nil);

        reply = co_await client.send(json_del(key3, "."));
        testForSuccess("JSON_DEL", reply);

        // JSON Object Commands
        // clang-format off
        reply = co_await client.send(json_set(
            key1, ".",
                "{\"name\":\"Leonard Cohen\",\"lastSeen\":1478476800,\"loggedOut\": true}"));
        // clang-format on
        testForSuccess("JSON.SET", reply);

        reply = co_await client.send(json_objlen(key1, "."));
        testForInt("JSON.OBJLEN", reply, 3);

        reply = co_await client.send(json_objkeys(key1, "."));
        redis_array array =
            redis_array{redis_value(string_to_vector("name")),
                        redis_value(string_to_vector("lastSeen")),
                        redis_value(string_to_vector("loggedOut"))};
        testForArray("JSON.OBJKEYS", reply, array);
    }

    barrier--;
    co_return;
}

awaitable<void> run_json_tests(asio::io_context& ctx) {
    std::atomic<int> barrier;
    auto exec = co_await cpool::net::this_coro::executor;
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);
    redis_client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    barrier = 50;
    int num_runners = barrier;
    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_json(client, i, barrier), cpool::detached);
    }

    while (barrier != 0) {
        cpool::timer timer(exec);
        co_await timer.async_wait(100ms);
    }

    auto reply = co_await client.send(flush_all());
    testForSuccess("FLUSH_ALL", reply);

    ctx.stop();
    co_return;
}

TEST(Redis, ClientTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

TEST(Redis, MultiClientTest) {
    asio::io_context ctx(8);

    cpool::co_spawn(ctx, run_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

TEST(Redis, PasswordClientTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_password_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

TEST(Redis, JsonTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_json_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

} // namespace