#include "redis/message.hpp"
#include "redis/value.hpp"

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
    std::vector<string> messageParts =
        std::vector<string>{"message", "something.otherthing", "42"};

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
    std::vector<string> messageParts = std::vector<string>{
        "pmessage", "something.*", "something.otherthing", "42"};
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
    std::vector<string> messageParts =
        std::vector<string>{"not-a-message", "something.otherthing", "42"};
    redis::redis_message message(messageParts);
    EXPECT_FALSE(message.valid());
    EXPECT_TRUE(message.empty());
    EXPECT_EQ(message.pattern, "");
    EXPECT_EQ(message.channel, "");
    EXPECT_EQ(message.contents, "");
}

} // namespace