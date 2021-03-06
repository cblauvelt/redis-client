#include "redis/message.hpp"
#include "redis/value.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std;

TEST(value, Null) {
    redis::value value;

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

TEST(value, string) {
    redis::value value("foo");
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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

TEST(value, Error) {
    redis::value value(redis::error(
        "WRONGTYPE Operation against a key holding the wrong kind of value"));
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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

TEST(value, integer) {
    redis::value value(1042);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::integer);
    EXPECT_TRUE(stringVal.has_value());
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
    EXPECT_EQ(intVal.value(), 1042);
    EXPECT_EQ(testValue, 1042);
}

TEST(value, int64_t) {
    int64_t tempVal = -1043;
    redis::value value(tempVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto boolVal = value.as<bool>();

    EXPECT_EQ(value.type(), redis::redis_type::integer);
    EXPECT_TRUE(stringVal.has_value());
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
    EXPECT_EQ(intVal.value(), -1043);
    EXPECT_EQ(testValue, -1043);
}

TEST(value, Float_Double) {
    redis::bulk_string tempVal = redis::string_to_vector("2.5");
    redis::value value(tempVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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

TEST(value, Negative_Float_Double) {
    redis::bulk_string tempVal = redis::string_to_vector("-2.5");
    redis::value value(tempVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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

TEST(value, BulkString) {
    std::vector<uint8_t> charVal = {'a', 'b', 'c', 'd', 'e', 'f', 0, 'g'};
    redis::value value(charVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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
    value = redis::value(intString);
    EXPECT_EQ(value.type(), redis::redis_type::bulk_string);
    EXPECT_TRUE(bulkStringVal.has_value());
    int intTestVal = value;
    EXPECT_EQ(intTestVal, 42);
    // Test a number that's too big
    intString = redis::string_to_vector("4200000000000");
    value = redis::value(intString);
    EXPECT_TRUE(!value.as<int>().has_value());
    EXPECT_TRUE(value.as<int64_t>().has_value());
    EXPECT_EQ(value.as<int64_t>().value(), 4200000000000);
}

TEST(value, EmptyBulkString) {
    std::vector<uint8_t> charVal;
    redis::value value(charVal);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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

TEST(value, Array) {
    std::vector<redis::value> redisArray = std::vector<redis::value>{
        redis::value("string"), redis::value(42), redis::value(int64_t(4200)),
        redis::value("string")};
    redis::value value(redisArray);
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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
    EXPECT_EQ(redisArray, (redis::redis_array)value);
}

TEST(value, Hash) {
    auto test_hash = redis::hash{{"field1", redis::value(42)},
                                 {"field2", redis::value("Hello")},
                                 {"field3", redis::value("World")}};
    redis::value value(test_hash);

    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
    auto int64Val = value.as<int64_t>();
    auto intVal = value.as<int>();
    auto floatVal = value.as<float>();
    auto doubleVal = value.as<double>();
    auto bulkStringVal = value.as<redis::bulk_string>();
    auto arrayVal = value.as<redis::redis_array>();
    auto hashVal = value.as<redis::hash>();
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
    EXPECT_TRUE(hashVal.has_value());
    EXPECT_FALSE(boolVal.has_value());

    // Array specific tests
    redis::redis_array array = value;
    EXPECT_EQ(array.size(), 6);
    EXPECT_EQ(array.size(), 6);
    EXPECT_EQ(test_hash, (redis::hash)value);
}

TEST(value, Redis_Message) {
    std::vector<redis::value> redisArray = std::vector<redis::value>{
        redis::value("message"), redis::value("stuff"),
        redis::value("morestuff")};
    redis::value value(redisArray);
    auto optMessage = value.as<redis::redis_message>();
    ASSERT_TRUE(optMessage.has_value());

    auto message = optMessage.value();
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.channel, "stuff");
    EXPECT_EQ(message.contents, "morestuff");

    redisArray = std::vector<redis::value>{
        redis::value("pmessage"), redis::value("something.*"),
        redis::value("something.otherthing"), redis::value("42")};

    value = redis::value(redisArray);
    optMessage = value.as<redis::redis_message>();
    ASSERT_TRUE(optMessage.has_value());

    message = optMessage.value();
    EXPECT_TRUE(message.valid());
    EXPECT_FALSE(message.empty());
    EXPECT_EQ(message.channel, "something.otherthing");
    EXPECT_EQ(message.contents, "42");
}

TEST(value, bool) {
    redis::value value("OK");
    auto stringVal = value.as<string>();
    auto errorVal = value.as<redis::error>();
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
    value = redis::value(0);
    boolVal = value.as<bool>();
    EXPECT_TRUE((value.type() == redis::redis_type::integer ||
                 value.type() == redis::redis_type::simple_string));
    EXPECT_TRUE(boolVal.has_value());
    EXPECT_FALSE((bool)value);
}

TEST(equality, all) {
    redis::value nilValue;
    redis::value intValue(1042);
    redis::value stringValue("Hello");
    redis::value bsValue(redis::bulk_string{'W', 'o', 'r', 'l', 'd'});
    redis::value arrValue(redis::redis_array{
        redis::value("field1"), redis::value(42), redis::value("field2"),
        redis::value("Hello"), redis::value("field3"), redis::value("World")});
    redis::value hashValue(redis::hash{{"field1", redis::value(42)},
                                       {"field2", redis::value("Hello")},
                                       {"field3", redis::value("World")}});

    EXPECT_EQ(nilValue, redis::value());

    EXPECT_EQ(intValue, redis::value(1042));
    EXPECT_NE(intValue, redis::value(42));

    EXPECT_EQ(stringValue, redis::value("Hello"));
    EXPECT_NE(stringValue, redis::value("World"));

    EXPECT_EQ(bsValue,
              redis::value(redis::bulk_string{'W', 'o', 'r', 'l', 'd'}));
    EXPECT_EQ(bsValue, redis::value("World"));
    EXPECT_NE(bsValue,
              redis::value(redis::bulk_string{'H', 'e', 'l', 'l', 'o'}));

    EXPECT_EQ(arrValue, redis::value(redis::redis_array{
                            redis::value("field1"), redis::value(42),
                            redis::value("field2"), redis::value("Hello"),
                            redis::value("field3"), redis::value("World")}));
    EXPECT_EQ(
        arrValue,
        redis::value(redis::redis_array{
            redis::value("field1"), redis::value(42), redis::value("field2"),
            redis::value(redis::bulk_string{'H', 'e', 'l', 'l', 'o'}),
            redis::value("field3"), redis::value("World")}));

    EXPECT_EQ(hashValue,
              redis::value(redis::hash{{"field1", redis::value(42)},
                                       {"field2", redis::value("Hello")},
                                       {"field3", redis::value("World")}}));
    EXPECT_NE(hashValue,
              redis::value(redis::hash{{"field1", redis::value(1042)},
                                       {"field2", redis::value("Hello")},
                                       {"field3", redis::value("World")}}));

    EXPECT_EQ(arrValue,
              redis::value(redis::hash{{"field1", redis::value(42)},
                                       {"field2", redis::value("Hello")},
                                       {"field3", redis::value("World")}}));

    EXPECT_NE(intValue, nilValue);
    EXPECT_NE(intValue, stringValue);
    EXPECT_NE(intValue, bsValue);
    EXPECT_NE(intValue, arrValue);
    EXPECT_NE(intValue, hashValue);
    EXPECT_NE(stringValue, nilValue);
    EXPECT_NE(stringValue, bsValue);
}

} // namespace