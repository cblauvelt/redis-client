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
#include "redis/commands-set.hpp"
#include "redis/commands-ttl.hpp"
#include "redis/commands.hpp"

#include "test_functions.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

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

awaitable<void> test_basic(client& client, int c,
                           cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key1 = std::string("basic") + std::to_string(c) + std::string("_1");
    string key2 = std::string("basic") + std::to_string(c) + std::string("_2");
    string key3 = std::string("basic") + std::to_string(c) + std::string("_3");
    redis::reply reply;

    for (int i = 0; i < 2; i++) {
        reply = co_await client.send(redis::set(key1, "42"));
        testForSuccess("SET", reply);

        reply = co_await client.send(redis::set(key2, "142"));
        testForSuccess("SET", reply);

        reply = co_await client.send(get(key1));
        testForValue("GET", reply, 42);

        reply = co_await client.send(exists(key1, key2, key3));
        testForValue("EXISTS", reply, 2);

        reply = co_await client.send(del(key1, key2));
        testForValue("DEL", reply, 2);

        reply = co_await client.send(get(key1));
        testForType("GET", reply, redis_type::nil);

        reply = co_await client.send(publish(key1, "stuff" + to_string(i)));
        testForType("PUBLISH", reply, redis_type::integer);
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_basic_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(logLevel, redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_basic(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> test_ttl(client& client, int c,
                         cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key = std::string("ttl") + std::to_string(c);
    redis::reply reply;
    const auto time_delay_sec = 10s;

    reply = co_await client.send(redis::ttl(key));
    testForValue("TTL", reply, -2);

    reply = co_await client.send(redis::expire(key, time_delay_sec));
    testForValue("EXPIRE", reply, 0);

    reply = co_await client.send(redis::set(key, "42"));
    testForSuccess("SET", reply);

    reply = co_await client.send(redis::ttl(key));
    testForValue("TTL", reply, -1);

    reply = co_await client.send(redis::expire(key, time_delay_sec));
    testForValue("EXPIRE", reply, 1);

    reply = co_await client.send(redis::ttl(key));
    testForLE("TTL", reply, time_delay_sec.count());

    reply = co_await client.send(redis::persist(key));
    testForValue("PERSIST", reply, 1);

    reply = co_await client.send(redis::persist(key));
    testForValue("PERSIST", reply, 0);

    reply = co_await client.send(del(key));
    testForSuccess("DEL", reply);

    reply = co_await client.send(redis::pexpire(key, time_delay_sec));
    testForValue("PEXPIRE", reply, 0);

    reply = co_await client.send(redis::set(key, "42"));
    testForSuccess("SET", reply);

    reply = co_await client.send(redis::pexpire(key, time_delay_sec));
    testForValue("PEXPIRE", reply, 1);

    reply = co_await client.send(redis::pttl(key));
    testForLE("PTTL", reply, time_delay_sec.count() * 1000);

    reply = co_await client.send(del(key));
    testForSuccess("DEL", reply);

    barrier.count_down();
    co_return;
}

awaitable<void> run_ttl_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(logLevel, redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_ttl(client, i, barrier), cpool::detached);
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
        testForValue("INCRBY", reply2, 257);

        auto reply3 = replies[2];
        testForValue("GET", reply3, 257);
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_pipeline_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(logLevel, redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
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
            testForValue("RPUSH", reply, 1);
            reply = co_await client.send(redis::rpush(key, test_arr));
            testForValue("RPUSH", reply, 7);
            reply =
                co_await client.send(redis::lpush(key, std::string("field0")));
            testForValue("LPUSH", reply, 8);

            auto new_arr =
                redis::redis_array{std::string("field0"), std::string("42")};
            reply = co_await client.send(redis::lpush(key, new_arr));
            testForValue("LPUSH", reply, 10);

            reply = co_await client.send(redis::llen(key));
            testForValue("LLEN", reply, 10);

            reply = co_await client.send(redis::lrange(key, 0, 1));
            testForArray("LRANGE", reply, std::views::reverse(new_arr));

            reply = co_await client.send(redis::lpop(key, 2));
            testForArray("LPOP", reply, std::views::reverse(new_arr));

            reply = co_await client.send(redis::rpop(key));
            testForValue("RPOP", reply,
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
            testForValue("DEL", reply, 1);
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

    logMessage(logLevel, redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
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
            testForValue("HSET", reply, 1);

            reply = co_await client.send(redis::hset(key, test_hash));
            testForValue("HSET", reply, 2);

            reply = co_await client.send(redis::hget(key, "field2"));
            testForValue("HGET", reply, (string)test_hash["field2"]);

            reply = co_await client.send(redis::hgetall(key));
            testForType("HGETALL", reply, redis_type::array);
            testForArray("HGETALL", reply, test_arr);
            testForValue("HGETALL", reply, test_hash);

            reply = co_await client.send(redis::hkeys(key));
            testForArray("HKEYS", reply, test_keys);

            reply = co_await client.send(redis::hvals(key));
            testForArray("HVALS", reply, test_vals);

            reply = co_await client.send(redis::hincrby(key, "field1", 1));
            testForValue("HINCRBY", reply, 43);

            reply =
                co_await client.send(redis::hincrbyfloat(key, "field1", 2.3));
            testForValue("HINCRBY", reply, 45.3);

            reply = co_await client.send(redis::hexists(key, "field1"));
            testForValue("HEXISTS", reply, true);

            reply = co_await client.send(redis::hlen(key));
            testForValue("HLEN", reply, 3);

            reply = co_await client.send(redis::hdel(key, "field1"));
            testForValue("HDEL", reply, 1);

            reply = co_await client.send(redis::hexists(key, "field1"));
            testForValue("HEXISTS", reply, false);

            reply = co_await client.send(redis::hlen(key));
            testForValue("HLEN", reply, 2);

            reply = co_await client.send(redis::del(key));
            testForValue("DEL", reply, 1);
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

    logMessage(logLevel, redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_hash(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

awaitable<void> test_set(client& client, int c,
                         cpool::awaitable_latch& barrier) {
    auto exec = co_await cpool::net::this_coro::executor;
    string key1 = std::string("set") + std::to_string(c) + std::string("1");
    string key2 = std::string("set") + std::to_string(c) + std::string("2");
    redis::reply reply;

    try {
        for (int i = 0; i < 2; i++) {
            auto newMembers = strings{"Big", "Hello", "World"};
            auto testKeyList = strings{key1, key2};

            reply = co_await client.send(redis::sadd(key1, "Hello", "World"));
            testForValue("SADD", reply, 2);

            reply = co_await client.send(redis::sadd(key1, newMembers));
            testForValue("SADD", reply, 1);

            reply = co_await client.send(redis::smembers(key1));
            testForSortedArray("SMEMBERS", reply, newMembers);

            reply = co_await client.send(redis::srem(key1, "Hello", "World"));
            testForValue("SADD", reply, 2);

            reply = co_await client.send(redis::smembers(key1));
            testForSortedArray("SMEMBERS", reply, strings{"Big"});

            reply = co_await client.send(redis::del(key1));
            testForValue("DEL", reply, 1);

            reply = co_await client.send(redis::sadd(key1, "a", "b", "c"));
            testForValue("SADD", reply, 3);

            reply = co_await client.send(redis::sadd(key2, "c", "d", "e"));
            testForValue("SADD", reply, 3);

            reply = co_await client.send(redis::sdiff(key1, key2));
            testForSortedArray("SDIFF", reply, strings{"a", "b"});

            reply = co_await client.send(redis::sdiff(testKeyList));
            testForSortedArray("SDIFF", reply, strings{"a", "b"});

            reply = co_await client.send(redis::sinter(key1, key2));
            testForSortedArray("SINTER", reply, strings{"c"});

            reply = co_await client.send(redis::sinter(testKeyList));
            testForSortedArray("SINTER", reply, strings{"c"});

            reply = co_await client.send(redis::sunion(key1, key2));
            testForSortedArray("SUNION ", reply,
                               strings{"a", "b", "c", "d", "e"});

            reply = co_await client.send(redis::sunion(testKeyList));
            testForSortedArray("SUNION ", reply,
                               strings{"a", "b", "c", "d", "e"});

            reply = co_await client.send(redis::spop(key1));
            testForType("SPOP", reply, redis_type::bulk_string);

            reply = co_await client.send(redis::spop(key2, 2));
            testForArraySize("SPOP", reply, 2);

            reply = co_await client.send(redis::del(key1, key2));
            testForValue("DEL", reply, 2);
        }
    } catch (const std::exception& ex) {
        EXPECT_EQ(std::string(), ex.what());
    }

    barrier.count_down();
    co_return;
}

awaitable<void> run_set_tests(asio::io_context& ctx) {
    const int num_runners = 50;
    auto exec = co_await cpool::net::this_coro::executor;
    cpool::awaitable_latch barrier(exec, num_runners);
    auto host = get_env_var("REDIS_HOST").value_or(DEFAULT_REDIS_HOST);

    logMessage(logLevel, redis::log_level::info, host);
    client client(exec, host, 6379);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
    EXPECT_TRUE(client.running());

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_set(client, i, barrier), cpool::detached);
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

    logMessage(logLevel, redis::log_level::info,
               fmt::format("Logging into {}:{} with password {}", config.host,
                           config.port, config.password));

    client client(exec, config);
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    auto reply = co_await client.ping();
    testForValue("PING", reply, "PONG");
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
        testForValue("JSON_GET", reply, 0);

        reply = co_await client.send(json_type(key1, ".foo"));
        testForValue("JSON_TYPE", reply, "string");

        reply = co_await client.send(json_strlen(key1, ".foo"));
        testForValue("JSON_STRLEN", reply, 3);

        reply = co_await client.send(json_strappend(key1, ".foo", "\"baz\""));
        testForValue("JSON_STRAPPEND", reply, 6);

        reply = co_await client.send(json_get(key1, ".foo"));
        testForValue("JSON_GET", reply, "\"barbaz\"");

        reply = co_await client.send(json_numincrby(key1, ".ans", "1"));
        testForValue("JSON_NUMINCRBY", reply, 1);

        reply = co_await client.send(json_numincrby(key1, ".ans", "1.5"));
        testForValue("JSON_NUMINCRBY", reply, 2.5F);
        testForValue("JSON_NUMINCRBY", reply, 2.5);

        reply = co_await client.send(json_numincrby(key1, ".ans", "-0.75"));
        testForValue("JSON_NUMINCRBY", reply, 1.75F);
        testForValue("JSON_NUMINCRBY", reply, 1.75);

        reply = co_await client.send(json_nummultby(key1, ".ans", "24"));
        testForValue("JSON_NUMMULRBY", reply, 42);

        reply = co_await client.send(json_del(key1, "."));
        testForSuccess("JSON_DEL", reply);

        reply = co_await client.send(
            json_set(key2, ".", "[ true, { \"answer\": 42 }, null ]"));
        testForSuccess("JSON_SET", reply);

        reply = co_await client.send(json_get(key2, "."));
        testForValue("JSON_GET", reply, "[true,{\"answer\":42},null]");

        reply = co_await client.send(json_get(key2, "[1].answer"));
        testForValue("JSON_GET", reply, 42);

        reply = co_await client.send(json_del(key2, "[-1]"));
        testForSuccess("JSON_DEL", reply);

        reply = co_await client.send(json_get(key2, "."));
        testForValue("JSON_GET", reply, "[true,{\"answer\":42}]");

        reply = co_await client.send(json_del(key2, "."));
        testForSuccess("JSON_DEL", reply);

        // Test array functions
        reply = co_await client.send(json_set(key3, ".", "[]"));
        testForSuccess("JSON_SET", reply);

        reply = co_await client.send(json_arrappend(key3, ".", "0"));
        testForValue("JSON.ARRAPPEND", reply, 1);

        reply = co_await client.send(json_get(key3, "."));
        testForValue("JSON.GET", reply, "[0]");

        std::vector<string> values{"0", "-2", "-1"};
        reply = co_await client.send(json_arrinsert(key3, ".", values));
        testForValue("JSON.ARRINSERT", reply, 3);

        reply = co_await client.send(json_get(key3, "."));
        testForValue("JSON.GET", reply, "[-2,-1,0]");

        reply = co_await client.send(json_arrtrim(key3, ".", "1", "1"));
        testForValue("JSON.ARRTRIM", reply, 1);

        reply = co_await client.send(json_get(key3, "."));
        testForValue("JSON.GET", reply, "[-1]");

        reply = co_await client.send(json_arrpop(key3, "."));
        testForValue("JSON.ARRPOP", reply, "-1");

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
        testForValue("JSON.OBJLEN", reply, 3);

        reply = co_await client.send(json_objkeys(key1, "."));
        redis_array array = redis_array{value(string_to_vector("name")),
                                        value(string_to_vector("lastSeen")),
                                        value(string_to_vector("loggedOut"))};
        testForArray("JSON.OBJKEYS", reply, array);

        reply = co_await client.send(json_del(key1, "."));
        testForSuccess("JSON_DEL", reply);
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
    client.set_logging_handler(std::bind(
        logMessage, logLevel, std::placeholders::_1, std::placeholders::_2));

    for (int i = 0; i < num_runners; i++) {
        cpool::co_spawn(ctx, test_json(client, i, barrier), cpool::detached);
    }

    co_await barrier.wait();

    ctx.stop();
    co_return;
}

TEST(Redis, BasicTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_basic_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}

TEST(Redis, TtlTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_ttl_tests(std::ref(ctx)), cpool::detached);

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

TEST(Redis, SetTest) {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_set_tests(std::ref(ctx)), cpool::detached);

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