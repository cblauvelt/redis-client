#include "redis/errors.hpp"
#include "redis/reply.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using string = std::string;

TEST(RedisReply, SimpleString) {
    std::vector<uint8_t> inputBuffer =
        std::vector<uint8_t>{'+', 'P', 'O', 'N', 'G', '\r', '\n'};

    redis::reply reply1(inputBuffer);
    redis::reply reply2;
    auto it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());

    // Test first constructor
    EXPECT_FALSE(reply1.error());
    string result1 = reply1.value();
    EXPECT_EQ(result1, "PONG");

    // Test second constructor
    EXPECT_FALSE(reply2.error());
    string result2 = reply2.value();
    EXPECT_EQ(result2, "PONG");
    EXPECT_EQ(it, inputBuffer.end());
}

TEST(RedisReply, ErrorCode) {
    redis::reply reply(redis::client_error_code::write_error);

    EXPECT_EQ(reply.error(), redis::client_error_code::write_error);
    EXPECT_EQ(reply.error().message(),
              "There was an error while writing the command to the server");
    EXPECT_EQ(reply.value().type(), redis::redis_type::nil);
}

TEST(RedisReply, Error) {
    std::string input = "-WRONGTYPE Operation against a key holding the "
                        "wrong kind of value\r\n";
    std::vector<uint8_t> inputBuffer = redis::string_to_vector(input);

    redis::reply reply1(inputBuffer);
    redis::reply reply2;
    auto it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());

    // Test first constructor
    EXPECT_EQ(reply1.error(), redis::client_error_code::error);
    redis::error result1 = reply1.value();
    string testString = result1.what();
    EXPECT_EQ(testString, "WRONGTYPE Operation against a key holding the "
                          "wrong kind of value");

    // Test second constructor
    EXPECT_TRUE(reply2.error());
    string result2 = reply2.value();
    EXPECT_EQ(result2, "WRONGTYPE Operation against a key holding the "
                       "wrong kind of value");
    EXPECT_EQ(it, inputBuffer.end());
}

TEST(RedisReply, Redis_Bulk_string) {
    std::vector<uint8_t> inputBuffer =
        std::vector<uint8_t>{'$', '2', '\r', '\n', '4', '2', '\r', '\n'};

    redis::reply reply1(inputBuffer);
    redis::reply reply2;
    auto it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());

    // Test first constructor
    EXPECT_FALSE(reply1.error());
    std::string result1 = reply1.value();
    EXPECT_EQ(result1, "42");

    // Test second constructor
    EXPECT_FALSE(reply2.error());
    string result2 = reply2.value();
    EXPECT_EQ(result2, "42");
    EXPECT_EQ(it, inputBuffer.end());

    // test null strings
    inputBuffer =
        std::vector<uint8_t>{'$', '-', '1', '\r', '\n'}; // null string
    reply1 = redis::reply(inputBuffer);
    it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());
    EXPECT_FALSE(reply1.error());
    EXPECT_EQ(reply1.value().type(), redis::redis_type::nil);
    EXPECT_FALSE(reply2.error());
    EXPECT_EQ(reply2.value().type(), redis::redis_type::nil);

    // test empty strings
    inputBuffer =
        std::vector<uint8_t>{'$', '0', '\r', '\n', '\r', '\n'}; // empty string
    reply1 = redis::reply(inputBuffer);
    it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());
    EXPECT_FALSE(reply1.error());
    result1 = reply1.value().as<string>().value();
    EXPECT_EQ(result1, "");
    EXPECT_FALSE(reply2.error());
    result2 = reply2.value().as<string>().value();
    EXPECT_EQ(result2, "");
}

TEST(RedisReply, Integer) {
    std::vector<uint8_t> inputBuffer =
        std::vector<uint8_t>{':', '1', '0', '0', '0', '\r', '\n'};

    redis::reply reply1(inputBuffer);
    redis::reply reply2;
    auto it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());

    // Test first constructor
    EXPECT_FALSE(reply1.error());
    int result1 = reply1.value();
    EXPECT_EQ(result1, 1000);

    // Test second constructor
    EXPECT_FALSE(reply2.error());
    int result2 = reply2.value();
    EXPECT_EQ(result2, 1000);
    EXPECT_EQ(it, inputBuffer.end());
}

TEST(RedisReply, Array) {
    std::string input = "*3\r\n$3\r\nfoo\r\n$-1\r\n$3\r\nbar\r\n";
    std::vector<uint8_t> inputBuffer = redis::string_to_vector(input);

    redis::reply reply1(inputBuffer);
    redis::reply reply2;
    auto it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());

    // Test first constructor
    EXPECT_FALSE(reply1.error());
    redis::redis_array result1 = reply1.value();
    EXPECT_EQ(result1.size(), 3);
    EXPECT_EQ((string)(result1.at(0)), "foo");
    EXPECT_EQ(result1.at(1).type(), redis::redis_type::nil);
    EXPECT_EQ((string)(result1.at(2)), "bar");

    // Test second constructor
    EXPECT_FALSE(reply2.error());
    redis::redis_array result2 = reply2.value();
    EXPECT_EQ(result2.size(), 3);
    EXPECT_EQ((string)(result2.at(0)), "foo");
    EXPECT_EQ(result2.at(1).type(), redis::redis_type::nil);
    EXPECT_EQ((string)(result2.at(2)), "bar");
    EXPECT_EQ(it, inputBuffer.end());
}

TEST(RedisReply, PingResponseArray) {
    std::string input = "*2\r\n$4\r\npong\r\n$0\r\n\r\n";
    std::vector<uint8_t> inputBuffer = redis::string_to_vector(input);
    redis::redis_array expected = redis::redis_array{
        redis::value("pong"), redis::value(redis::bulk_string())};

    redis::reply reply1(inputBuffer);
    redis::reply reply2;
    auto it = reply2.load_data(inputBuffer.begin(), inputBuffer.end());

    // Test first constructor
    EXPECT_FALSE(reply1.error());
    redis::redis_array result1 = reply1.value();
    EXPECT_EQ(result1.size(), 2);
    EXPECT_EQ((string)(result1.at(0)), (string)expected.at(0));
    EXPECT_EQ(result1.at(1).type(), expected.at(1).type());
    EXPECT_EQ((string)(result1.at(1)), (string)(expected.at(1)));

    // Test second constructor
    EXPECT_FALSE(reply2.error());
    redis::redis_array result2 = reply2.value();
    EXPECT_EQ(result2.size(), 2);
    EXPECT_EQ((string)(result2.at(0)), (string)expected.at(0));
    EXPECT_EQ(result2.at(1).type(), expected.at(1).type());
    EXPECT_EQ((string)(result2.at(1)), (string)(expected.at(1)));
    EXPECT_EQ(it, inputBuffer.end());
}

TEST(RedisReply, Pipeline) {
    auto inputBuffer =
        std::vector<uint8_t>{':', '1',  '0',  '2', '4', '\r', '\n', '$',
                             '2', '\r', '\n', '4', '2', '\r', '\n'};

    redis::reply reply1;
    auto it = reply1.load_data(inputBuffer.cbegin(), inputBuffer.cend());

    // Test first reply
    EXPECT_FALSE(reply1.error());
    int result1 = reply1.value();
    EXPECT_EQ(result1, 1024);
    EXPECT_EQ(it, inputBuffer.cbegin() + 7);

    redis::reply reply2;
    it = reply2.load_data(it, inputBuffer.cend());

    // Test second reply
    EXPECT_FALSE(reply2.error());
    string result2 = reply2.value();
    EXPECT_EQ(result2, "42");
    EXPECT_EQ(it, inputBuffer.cend());

    // test null strings
    inputBuffer = std::vector<uint8_t>{':',  '1', '0', '2', '4',  '\r',
                                       '\n', '$', '-', '1', '\r', '\n'};
    it = reply1.load_data(inputBuffer.cbegin(), inputBuffer.cend());
    // Test first reply
    EXPECT_FALSE(reply1.error());
    result1 = reply1.value();
    EXPECT_EQ(result1, 1024);
    EXPECT_EQ(it, inputBuffer.cbegin() + 7);

    it = reply2.load_data(it, inputBuffer.cend());

    EXPECT_FALSE(reply2.error());
    EXPECT_EQ(reply2.value().type(), redis::redis_type::nil);
}

} // namespace