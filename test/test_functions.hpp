#pragma once

#include "redis/client.hpp"
#include "redis/command.hpp"
#include "redis/error.hpp"
#include "redis/errors.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using string = std::string;

inline void testForError(std::string cmd, const redis::reply& reply) {
    EXPECT_FALSE(reply.error()) << cmd;
    if (reply.error() == redis::client_error_code::error) {
        EXPECT_EQ(reply.value().as<redis::error>().value().what(),
                  std::string())
            << cmd;
    }
}

inline void testForValue(std::string cmd, redis::reply reply,
                         redis::value expected) {
    testForError(cmd, reply);

    EXPECT_EQ(reply.value(), expected) << "Expected string: " << expected;
}

inline void testForValue(std::string cmd, redis::reply reply,
                         const char* expected) {
    testForValue(cmd, reply, std::string(expected));
}

inline void testForValue(std::string cmd, redis::reply reply, string expected) {
    testForError(cmd, reply);

    auto optString = reply.value().as<string>();
    EXPECT_TRUE(optString.has_value()) << "Expected string: " << expected;
    EXPECT_EQ(optString.value(), expected) << "Expected string: " << expected;
}

inline void testForValue(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value()) << cmd;
    EXPECT_EQ(optInt.value(), expected) << cmd;
}

inline void testForValue(std::string cmd, redis::reply reply, float expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optFloat = value.as<float>();

    EXPECT_TRUE(optFloat.has_value()) << cmd;
    EXPECT_FLOAT_EQ(optFloat.value(), expected) << cmd;
}

inline void testForValue(std::string cmd, redis::reply reply, double expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optDouble = value.as<double>();

    EXPECT_TRUE(optDouble.has_value()) << cmd;
    EXPECT_DOUBLE_EQ(optDouble.value(), expected) << cmd;
}

inline void testForValue(std::string cmd, redis::reply reply,
                         redis::hash expected) {
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

inline void testForValue(std::string cmd, redis::reply reply, bool expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();

    EXPECT_EQ(value.as<bool>().value(), expected) << cmd;
}

inline void testForGE(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value()) << cmd;
    EXPECT_GE(optInt.value(), expected) << cmd;
}

inline void testForGT(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value()) << cmd;
    EXPECT_GT(optInt.value(), expected) << cmd;
}

inline void testForLE(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value()) << cmd;
    EXPECT_LE(optInt.value(), expected) << cmd;
}

inline void testForLT(std::string cmd, redis::reply reply, int expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optInt = value.as<int>();
    EXPECT_TRUE(optInt.has_value()) << cmd;
    EXPECT_LT(optInt.value(), expected) << cmd;
}

template <class T>
void testForArray(std::string cmd, redis::reply reply, T expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optArray = value.as<redis::redis_array>();
    redis::redis_array array;

    EXPECT_TRUE(optArray.has_value()) << cmd;
    EXPECT_NO_THROW(array = optArray.value()) << cmd;
    EXPECT_EQ(array.size(), expected.size()) << cmd;
    for (int i = 0; i < array.size(); i++) {
        // EXPECT_EQ(array[i].type(), expected[i].type());
        EXPECT_EQ(array[i], expected[i]);
    }
}

template <class T>
void testForSortedArray(std::string cmd, redis::reply reply, T expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optArray = value.as<redis::redis_array>();
    redis::redis_array array;

    EXPECT_TRUE(optArray.has_value()) << cmd;
    EXPECT_NO_THROW(array = optArray.value()) << cmd;
    EXPECT_EQ(array.size(), expected.size()) << cmd;
    std::sort(array.begin(), array.end());
    for (int i = 0; i < array.size(); i++) {
        // EXPECT_EQ(array[i].type(), expected[i].type());
        EXPECT_EQ(array[i], expected[i]);
    }
}

inline void testForArraySize(std::string cmd, redis::reply reply,
                             size_t expected) {
    testForError(cmd, reply);

    redis::value value = reply.value();
    auto optArray = value.as<redis::redis_array>();
    redis::redis_array array;

    EXPECT_TRUE(optArray.has_value()) << cmd;
    EXPECT_NO_THROW(array = optArray.value()) << cmd;
    EXPECT_EQ(array.size(), expected) << cmd;
}

inline void testForSuccess(std::string cmd, redis::reply reply) {
    testForError(cmd, reply);

    redis::value value = reply.value();

    EXPECT_TRUE(value.as<bool>().value_or(false)) << cmd;
}

inline void testForType(std::string cmd, redis::reply reply,
                        redis::redis_type type) {
    testForError(cmd, reply);

    EXPECT_EQ(reply.value().type(), type);
}