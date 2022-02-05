# redis-client
A C++ client for redis based on ASIO.

## How to Use
redis-client depends on the [connection-pool library](https://github.com/cblauvelt/connection-pool). Install the conan dependencies

### Get Dependencies
If you have docker installed you can:
```bash
docker volume create conan
git clone https://github.com/cblauvelt/connection-pool.git
docker container run --rm -v conan:/home/conan/.conan -v $(pwd)/connection-pool:/src cblauvelt/cpp-builder-gcc10:latest conan create /src
```

If you have conan installed in your system and a compiler that supports C++20:
```bash
git clone https://github.com/cblauvelt/connection-pool.git
cd connection-pool
conan create .
```

### Create Dependency
If you have docker installed you can:
```bash
docker volume create conan
git clone https://github.com/cblauvelt/redis-client.git
docker container run --rm -v conan:/home/conan/.conan -v $(pwd)/redis-client:/src cblauvelt/cpp-builder-gcc10:latest conan create /src
```

If you have conan installed in your system and a compiler that supports C++20:
```bash
git clone https://github.com/cblauvelt/redis-client.git
cd redis-client
conan create .
```

If you're not using conan, you can simply copy the include files into your project.


### Using VSCode
If you're using VSCode you can now use the library in your project by following the instructions here:
https://github.com/cblauvelt/docker-vscode-cpp.git

## Examples
Ensure that you start the redis server before running these tests.

Using the redis-client:
```C++
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "commands.hpp"
#include "redis_client.hpp"
#include "redis_command.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace redis;

std::string get_env_var(std::string const& key) {
    char* val = getenv(key.c_str());
    return val == NULL ? std::string("127.0.0.1") : std::string(val);
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
    auto host = get_env_var("REDIS_HOST");

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
        co_await timer.async_wait(std::chrono::milliseconds(100));
    }

    ctx.stop();
    co_return;
}

void main() {
    asio::io_context ctx(1);

    cpool::co_spawn(ctx, run_tests(std::ref(ctx)), cpool::detached);

    ctx.run();
}
```