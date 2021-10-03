#include "redis_message.hpp"
#include "redis_value.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std;

TEST(redis_value, Null) {
    redis::redis_value value;

    EXPECT_EQ(value.type(), redis::redis_type::nil);
    EXPECT_FALSE(value.as<string>().has_value());
    EXPECT_FALSE(value.as<std::error_code>().has_value());
    EXPECT_FALSE(value.as<int64_t>().has_value());
    EXPECT_FALSE(value.as<int>().has_value());
    EXPECT_FALSE(value.as<float>().has_value());
    EXPECT_FALSE(value.as<double>().has_value());
    EXPECT_FALSE(value.as<redis::bulk_string>().has_value());
    EXPECT_FALSE(value.as<redis::redis_array>().has_value());
    EXPECT_FALSE(value.as<bool>().has_value());
}

TEST(redis_value, string) {
    redis::redis_value value("foo");
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::simple_string);
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_FALSE(int64Val.has_value());
    EXPECT_FALSE(intVal.has_value());
    EXPECT_FALSE(floatVal.has_value());
    EXPECT_FALSE(doubleVal.has_value());
    EXPECT_TRUE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_TRUE(boolVal.has_value());

    // string specific tests
    string testValue = value;
    redis::bulk_string bulkStringReference = redis::bulk_string{'f', 'o', 'o'};
    redis::bulk_string bulkStringTestValue = value;
    EXPECT_EQ(stringVal.value(), "foo");
    EXPECT_EQ(testValue, "foo");
    EXPECT_EQ(bulkStringVal.value(), bulkStringTestValue);
    EXPECT_EQ(bulkStringTestValue, bulkStringReference);
}

TEST(redis_value, Error) {
    redis::redis_value value(redis::redis_error(
        "WRONGTYPE Operation against a key holding the wrong kind of value"));
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::error);
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_TRUE(errorVal.has_value());
    EXPECT_FALSE(int64Val.has_value());
    EXPECT_FALSE(intVal.has_value());
    EXPECT_FALSE(floatVal.has_value());
    EXPECT_FALSE(doubleVal.has_value());
    EXPECT_FALSE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_TRUE(boolVal.has_value());

    // redis::redis_error specific tests
    string testValue = errorVal.value().what();
    EXPECT_EQ(
        testValue,
        "WRONGTYPE Operation against a key holding the wrong kind of value");

    // string specific tests
    testValue = (string)value;
    EXPECT_EQ(
        stringVal.value(),
        "WRONGTYPE Operation against a key holding the wrong kind of value");

    // bool specific tests
    bool success = value;
    EXPECT_FALSE(success);
}

TEST(redis_value, integer) {
    redis::redis_value value(42);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::integer);
    EXPECT_FALSE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_TRUE(int64Val.has_value());
    EXPECT_TRUE(intVal.has_value());
    EXPECT_TRUE(floatVal.has_value());
    EXPECT_TRUE(doubleVal.has_value());
    EXPECT_FALSE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_TRUE(boolVal.has_value());

    // int specific tests
    int testValue = value;
    EXPECT_EQ(intVal.value(), 42);
    EXPECT_EQ(testValue, 42);
}

TEST(redis_value, int64_t) {
    int64_t tempVal = -43;
    redis::redis_value value(tempVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::integer);
    EXPECT_FALSE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_TRUE(int64Val.has_value());
    EXPECT_TRUE(intVal.has_value());
    EXPECT_TRUE(floatVal.has_value());
    EXPECT_TRUE(doubleVal.has_value());
    EXPECT_FALSE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_TRUE(boolVal.has_value());

    // int64 specific tests
    int testValue = value;
    EXPECT_EQ(intVal.value(), -43);
    EXPECT_EQ(testValue, -43);
}

TEST(redis_value, Float_Double) {
    redis::bulk_string tempVal = redis::string_to_vector("2.5");
    redis::redis_value value(tempVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::bulk_string);
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_TRUE(int64Val.has_value());
    EXPECT_TRUE(intVal.has_value());
    EXPECT_TRUE(floatVal.has_value());
    EXPECT_TRUE(doubleVal.has_value());
    EXPECT_TRUE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_FALSE(boolVal.has_value());

    // int specific tests
    int testIntValue = value;
    EXPECT_EQ(intVal.value(), 2);
    EXPECT_EQ(testIntValue, 2);

    // float specific tests
    float testFloatValue = value;
    EXPECT_FLOAT_EQ(floatVal.value(), 2.5F);
    EXPECT_FLOAT_EQ(testFloatValue, 2.5F);

    // double specific tests
    double testDoubleValue = value;
    EXPECT_DOUBLE_EQ(doubleVal.value(), 2.5);
    EXPECT_DOUBLE_EQ(testDoubleValue, 2.5);
}

TEST(redis_value, Negative_Float_Double) {
    redis::bulk_string tempVal = redis::string_to_vector("-2.5");
    redis::redis_value value(tempVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::bulk_string);
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_TRUE(int64Val.has_value());
    EXPECT_TRUE(intVal.has_value());
    EXPECT_TRUE(floatVal.has_value());
    EXPECT_TRUE(doubleVal.has_value());
    EXPECT_TRUE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_FALSE(boolVal.has_value());

    // int specific tests
    int testIntValue = value;
    EXPECT_EQ(intVal.value(), -2);
    EXPECT_EQ(testIntValue, -2);

    // float specific tests
    float testFloatValue = value;
    EXPECT_FLOAT_EQ(floatVal.value(), -2.5F);
    EXPECT_FLOAT_EQ(testFloatValue, -2.5F);

    // double specific tests
    double testDoubleValue = value;
    EXPECT_DOUBLE_EQ(doubleVal.value(), -2.5);
    EXPECT_DOUBLE_EQ(testDoubleValue, -2.5);
}

TEST(redis_value, BulkString) {
    std::vector<uint8_t> charVal = {'a', 'b', 'c', 'd', 'e', 'f', 0, 'g'};
    redis::redis_value value(charVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::bulk_string);
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_FALSE(int64Val.has_value());
    EXPECT_FALSE(intVal.has_value());
    EXPECT_FALSE(floatVal.has_value());
    EXPECT_FALSE(doubleVal.has_value());
    EXPECT_TRUE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_FALSE(boolVal.has_value());

    // Bulkstring specific tests
    redis::bulk_string testValue = value;
    string stringTestValue = value;
    string convertedString = redis::vector_to_string(charVal);
    EXPECT_EQ(bulkStringVal.value(), charVal);
    EXPECT_EQ(testValue, charVal);
    EXPECT_EQ(stringVal.value(), convertedString);
    EXPECT_EQ(stringTestValue, convertedString);

    // Test conversion to int types
    std::vector<uint8_t> intString = {'4', '2'};
    value = redis::redis_value(intString);
    EXPECT_EQ(value.type(), redis::redis_type::bulk_string);
    EXPECT_TRUE(bulkStringVal.has_value());
    int intTestVal = value;
    EXPECT_EQ(intTestVal, 42);
    // Test a number that's too big
    intString = redis::string_to_vector("4200000000000");
    value = redis::redis_value(intString);
    EXPECT_TRUE(!value.as<int>().has_value());
    EXPECT_TRUE(value.as<int64_t>().has_value());
    EXPECT_EQ(value.as<int64_t>().value(), 4200000000000);
}

TEST(redis_value, EmptyBulkString) {
    std::vector<uint8_t> charVal;
    redis::redis_value value(charVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::bulk_string);
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_FALSE(int64Val.has_value());
    EXPECT_FALSE(intVal.has_value());
    EXPECT_FALSE(floatVal.has_value());
    EXPECT_FALSE(doubleVal.has_value());
    EXPECT_TRUE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_FALSE(boolVal.has_value());

    // Bulkstring specific tests
    redis::bulk_string testValue = value;
    string stringTestValue = value;
    string convertedString = redis::vector_to_string(charVal);
    EXPECT_EQ(bulkStringVal.value(), charVal);
    EXPECT_EQ(testValue, charVal);
    EXPECT_EQ(stringVal.value(), convertedString);
    EXPECT_EQ(stringTestValue, convertedString);
}

TEST(redis_value, Array) {
    std::vector<redis::redis_value> redisArray =
        std::vector<redis::redis_value>{
            redis::redis_value("string"), redis::redis_value(42),
            redis::redis_value(int64_t(4200)), redis::redis_value("string")};
    redis::redis_value value(redisArray);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::array);
    EXPECT_FALSE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_FALSE(int64Val.has_value());
    EXPECT_FALSE(intVal.has_value());
    EXPECT_FALSE(floatVal.has_value());
    EXPECT_FALSE(doubleVal.has_value());
    EXPECT_FALSE(bulkStringVal.has_value());
    EXPECT_TRUE(arrayVal.has_value());
    EXPECT_FALSE(boolVal.has_value());

    // Array specific tests
    redis::redis_array array = value;
    EXPECT_EQ(arrayVal.value().size(), 4);
    EXPECT_EQ(array.size(), 4);
}

TEST(redis_value, Redis_Message) {
    std::vector<redis::redis_value> redisArray =
        std::vector<redis::redis_value>{redis::redis_value("message"),
                                        redis::redis_value("stuff"),
                                        redis::redis_value("morestuff")};
    redis::redis_value value(redisArray);
    auto optMessage = value.as<redis::redis_message>();
    ASSERT_TRUE(optMessage.has_value());

    auto message = optMessage.value();
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.channel, "stuff");
    EXPECT_EQ(message.contents, "morestuff");

    redisArray = std::vector<redis::redis_value>{
        redis::redis_value("pmessage"), redis::redis_value("something.*"),
        redis::redis_value("something.otherthing"), redis::redis_value("42")};

    value = redis::redis_value(redisArray);
    optMessage = value.as<redis::redis_message>();
    ASSERT_TRUE(optMessage.has_value());

    message = optMessage.value();
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.channel, "something.otherthing");
    EXPECT_EQ(message.contents, "42");
}

TEST(redis_value, bool) {
    redis::redis_value value("OK");
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::redis_error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_TRUE((value.type() == redis::redis_type::integer ||
                 value.type() == redis::redis_type::simple_string));
    EXPECT_TRUE(stringVal.has_value());
    EXPECT_FALSE(errorVal.has_value());
    EXPECT_FALSE(int64Val.has_value());
    EXPECT_FALSE(intVal.has_value());
    EXPECT_FALSE(floatVal.has_value());
    EXPECT_FALSE(doubleVal.has_value());
    EXPECT_TRUE(bulkStringVal.has_value());
    EXPECT_FALSE(arrayVal.has_value());
    EXPECT_TRUE(boolVal.has_value());

    // bool specific tests
    EXPECT_TRUE((bool)value);
    value = redis::redis_value(0);
    boolVal = value.as<bool>();
    EXPECT_TRUE((value.type() == redis::redis_type::integer ||
                 value.type() == redis::redis_type::simple_string));
    EXPECT_TRUE(boolVal.has_value());
    EXPECT_FALSE((bool)value);
}

} // namespace