#include "redis_message.hpp"
#include "redis_value.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using string = std::string;

TEST(redis_message, Null) {
    redis::redis_message message;
    EXPECT_TRUE(message.empty());
}

TEST(redis_message, MessageFromStrings) {
    std::vector<string> messageParts =
        std::vector<string>{"message", "something.otherthing", "42"};
    redis::redis_message message(messageParts);
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.channel, "something.otherthing");
    EXPECT_EQ(message.contents, "42");
}

TEST(redis_message, MessageFromRedisArray) {
    redis::redis_array messageParts = redis::redis_array{
        redis::redis_value("message"),
        redis::redis_value("something.otherthing"), redis::redis_value("42")};
    redis::redis_message message(messageParts);
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.channel, "something.otherthing");
    EXPECT_EQ(message.contents, "42");
}

TEST(redis_message, PmessageFromStrings) {
    std::vector<string> messageParts = std::vector<string>{
        "pmessage", "something.*", "something.otherthing", "42"};
    redis::redis_message message(messageParts);
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.pattern, "something.*");
    EXPECT_EQ(message.channel, "something.otherthing");
    EXPECT_EQ(message.contents, "42");
}

TEST(redis_message, PmessageFromRedisArray) {
    redis::redis_array messageParts = redis::redis_array{
        redis::redis_value("pmessage"), redis::redis_value("something.*"),
        redis::redis_value("something.otherthing"), redis::redis_value("42")};
    redis::redis_message message(messageParts);
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.pattern, "something.*");
    EXPECT_EQ(message.channel, "something.otherthing");
    EXPECT_EQ(message.contents, "42");
}

TEST(redis_message, BadMessageFromStrings) {
    std::vector<string> messageParts =
        std::vector<string>{"not-a-message", "something.otherthing", "42"};
    redis::redis_message message(messageParts);
    EXPECT_FALSE(message.valid());
    EXPECT_TRUE(message.empty());
    EXPECT_EQ(message.pattern, "");
    EXPECT_EQ(message.channel, "");
    EXPECT_EQ(message.contents, "");
}

TEST(redis_message, BadMessageFromRedisArray) {
    redis::redis_array messageParts = redis::redis_array{
        redis::redis_value("not-a-message"),
        redis::redis_value("something.otherthing"), redis::redis_value("42")};
    redis::redis_message message(messageParts);
    EXPECT_FALSE(message.valid());
    EXPECT_TRUE(message.empty());
    EXPECT_EQ(message.pattern, "");
    EXPECT_EQ(message.channel, "");
    EXPECT_EQ(message.contents, "");
}

} // namespace