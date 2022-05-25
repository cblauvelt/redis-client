#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "cpool/awaitable_latch.hpp"
#include "redis/client.hpp"
#include "redis/command.hpp"
#include "redis/commands-hash.hpp"
#include "redis/commands-json.hpp"
#include "redis/commands-list.hpp"
#include "redis/commands.hpp"

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

void testForError(std::string cmd, const redis::reply& reply) {
    EXPECT_FALSE(reply.error()) << cmd;
    if (reply.error() == client_error_code::error) {
        EXPECT_EQ(reply.value().as<redis::error>().value().what(),
                  std::string())
            << cmd;
    }
}

void testForString(std::string cmd, redis::reply reply, string expected) {
    testForError(cmd, reply);

    auto optString = reply.value().as<string>();
    EXPECT_TRUE(optString.has_value()) << "Expected string: " << expected;
    EXPECT_EQ(optString.value(), expected) << "Expected string: " << expected;
}

void testForInt(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value());
    EXPECT_EQ(optInt.value(), expected);
}

void testForFloat(std::string cmd, redis::reply reply, float expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optFloat = value.as<float>();

    EXPECT_TRUE(optFloat.has_value());
    EXPECT_FLOAT_EQ(optFloat.value(), expected);
}

void testForDouble(std::string cmd, redis::reply reply, double expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optDouble = value.as<double>();

    EXPECT_TRUE(optDouble.has_value());
    EXPECT_DOUBLE_EQ(optDouble.value(), expected);
}

template <class T>
void testForArray(std::string cmd, redis::reply reply, T expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optArray = value.as<redis_array>();
    redis::redis_array array;

    EXPECT_TRUE(optArray.has_value());
    EXPECT_NO_THROW(array = optArray.value());
    EXPECT_EQ(array.size(), expected.size());
    for (int i = 0; i < array.size(); i++) {
        // EXPECT_EQ(array[i].type(), expected[i].type());
        EXPECT_EQ(array[i], expected[i]);
    }
}

void testForHash(std::string cmd, redis::reply reply, redis::hash expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optHash = value.as<redis::hash>();
    redis::hash hash;

    EXPECT_TRUE(optHash.has_value());
    EXPECT_NO_THROW(hash = optHash.value());
    EXPECT_EQ(hash.size(), expected.size());

    for (auto [key, value] : hash) {
        EXPECT_EQ(value, expected[key]);
    }
}

void testForBool(std::string cmd, redis::reply reply, bool expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();

    EXPECT_EQ(value.as<bool>().value(), expected) << cmd;
}

void testForSuccess(std::string cmd, redis::reply reply) {
    testForError(cmd, reply);

    redis::value value = reply.value();

    EXPECT_TRUE(value.as<bool>().value_or(false)) << cmd;
}

void testForType(std::string cmd, redis::reply reply, redis::redis_type type) {
    testForError(cmd, reply);

    EXPECT_EQ(reply.value().type(), type);
}

void logMessage(log_level level, string_view message) {
    cout << message << endl;
}

awaitable<void> test_basic(client& client, int c,
                           cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key = std::string("foo") + std::to_string(c);
    redis::reply reply;

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

    barrier.count_down();
    co_return;
}

awaitable<void> run_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_basic(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> test_pipeline_basic(client& client, int c,
                                    cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key = std::string("foo") + std::to_string(c);
    redis::commands commands{redis::set(key, "255"), redis::incrby(key, 2),
                             redis::get(key)};
    redis::replies replies;

    for (int i = 0; i < 2; i++) {
        replies = co_await client.send(commands);
        EXPECT_EQ(commands.size(), replies.size());
        auto reply1 = replies[0];
        testForSuccess("SET", reply1);

        auto reply2 = replies[1];
        testForInt("INCRBY", reply2, 257);

        auto reply3 = replies[2];
        testForInt("GET", reply3, 257);
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_pipeline_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_pipeline_basic(client, i, barrier),
                        cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> test_list(client& client, int c,
                          cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key = std::string("list") + std::to_string(c);
    redis::reply reply;

    auto test_arr = redis::redis_array{
        redis::value("field1"), redis::value("42"),
        redis::value("field2"), redis::value(string_to_vector("Hello")),
        redis::value("field3"), redis::value(string_to_vector("World"))};

    try {

        reply = co_await client.send(redis::del(key));
        for (int i = 0; i < 2; i++) {
            reply =
                co_await client.send(redis::rpush(key, std::string("field1")));
            testForInt("RPUSH", reply, 1);
            reply = co_await client.send(redis::rpush(key, test_arr));
            testForInt("RPUSH", reply, 7);
            reply =
                co_await client.send(redis::lpush(key, std::string("field0")));
            testForInt("LPUSH", reply, 8);

            auto new_arr =
                redis::redis_array{std::string("field0"), std::string("42")};
            reply = co_await client.send(redis::lpush(key, new_arr));
            testForInt("LPUSH", reply, 10);

            reply = co_await client.send(redis::llen(key));
            testForInt("LLEN", reply, 10);

            reply = co_await client.send(redis::lrange(key, 0, 1));
            testForArray("LRANGE", reply, std::views::reverse(new_arr));

            reply = co_await client.send(redis::lpop(key, 2));
            testForArray("LPOP", reply, std::views::reverse(new_arr));

            reply = co_await client.send(redis::rpop(key));
            testForString("RPOP", reply,
                          redis::value(string_to_vector("World")));

            reply = co_await client.send(redis::blpop(key));
            new_arr =
                redis::redis_array{redis::value(key), redis::value("field0")};
            testForArray("BLPOP", reply, new_arr);

            reply = co_await client.send(redis::brpop(key, 1));
            new_arr =
                redis::redis_array{redis::value(key), redis::value("field3")};
            testForArray("RPOP", reply, new_arr);

            reply = co_await client.send(redis::del(key));
            testForInt("DEL", reply, 1);
        }
    } catch (const std::exception& ex) {
        EXPECT_EQ(std::string(), ex.what());
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_list_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_list(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> test_hash(client& client, int c,
                          cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key = std::string("hash") + std::to_string(c);
    redis::reply reply;

    auto test_hash = redis::hash{{"field1", redis::value("42")},
                                 {"field2", redis::value("Hello")},
                                 {"field3", redis::value("World")}};

    auto test_arr = redis::redis_array{
        redis::value("field1"), redis::value("42"),
        redis::value("field2"), redis::value(string_to_vector("Hello")),
        redis::value("field3"), redis::value(string_to_vector("World"))};

    auto test_keys = redis::redis_array{
        redis::value("field1"), redis::value("field2"), redis::value("field3")};
    auto test_vals = redis::redis_array{
        redis::value("42"), redis::value(string_to_vector("Hello")),
        redis::value(string_to_vector("World"))};

    try {
        for (int i = 0; i < 2; i++) {
            reply = co_await client.send(
                redis::hset(key, "field1", redis::value("42")));
            testForInt("HSET", reply, 1);

            reply = co_await client.send(redis::hset(key, test_hash));
            testForInt("HSET", reply, 2);

            reply = co_await client.send(redis::hget(key, "field2"));
            testForString("HGET", reply, (string)test_hash["field2"]);

            reply = co_await client.send(redis::hgetall(key));
            testForType("HGETALL", reply, redis_type::array);
            testForArray("HGETALL", reply, test_arr);
            testForHash("HGETALL", reply, test_hash);

            reply = co_await client.send(redis::hkeys(key));
            testForArray("HKEYS", reply, test_keys);

            reply = co_await client.send(redis::hvals(key));
            testForArray("HVALS", reply, test_vals);

            reply = co_await client.send(redis::hincrby(key, "field1", 1));
            testForInt("HINCRBY", reply, 43);

            reply =
                co_await client.send(redis::hincrbyfloat(key, "field1", 2.3));
            testForFloat("HINCRBY", reply, 45.3);

            reply = co_await client.send(redis::hexists(key, "field1"));
            testForBool("HEXISTS", reply, true);

            reply = co_await client.send(redis::hlen(key));
            testForInt("HLEN", reply, 3);

            reply = co_await client.send(redis::hdel(key, "field1"));
            testForInt("HDEL", reply, 1);

            reply = co_await client.send(redis::hexists(key, "field1"));
            testForBool("HEXISTS", reply, false);

            reply = co_await client.send(redis::hlen(key));
            testForInt("HLEN", reply, 2);

            reply = co_await client.send(redis::del(key));
            testForInt("DEL", reply, 1);
        }
    } catch (const std::exception& ex) {
        EXPECT_EQ(std::string(), ex.what());
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_hash_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_hash(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> run_password_tests(asio::io_context& ctx) {
    const int num_runners = 2;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_PASSWORD_HOST").value_or(DEFAULT_REDIS_HOST);
    auto portString =
        get_env_var("REDIS_PASSWORD_PORT").value_or(DEFAULT_REDIS_PORT);
    int port = std::stoi(portString);
    auto password = get_env_var("REDIS_PASSWORD").value_or("");
    auto config =
        client_config{}.set_host(host).set_port(port).set_password(password);

    logMessage(redis::log_level::info,
               fmt::format("Logging into {}:{} with password {}", config.host,
                           config.port, config.password));

    client client(exec, config);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForString("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_basic(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> test_json(client& client, int c,
                          cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key1 = std::string("object") + std::to_string(c);
    string key2 = std::string("amoreinterestingexample") + std::to_string(c);
    string key3 = std::string("arr") + std::to_string(c);
    redis::reply reply;

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
        redis_array array = redis_array{value(string_to_vector("name")),
                                        value(string_to_vector("lastSeen")),
                                        value(string_to_vector("loggedOut"))};
        testForArray("JSON.OBJKEYS", reply, array);
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_json_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);
    client client(exec, host, 6379);
    client.set_logging_handler(
        std::bind(logMessage, std::placeholders::_1, std::placeholders::_2));

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_json(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

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

TEST(Redis, PipelineTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_pipeline_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

TEST(Redis, ListTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_list_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

TEST(Redis, HashTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_hash_tests(std::ref(ctx)), cpool::detached);

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