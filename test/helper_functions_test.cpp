
#include "helper_functions.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(Helper_Functions, string_to_vector) {
    std::string test_string = "abcdefg";
    std::vector<uint8_t> test_vector = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

    EXPECT_EQ(test_vector, redis::string_to_vector(test_string));
}

TEST(Helper_Functions, vector_to_string) {
    std::string test_string = "abcdefg";
    std::vector<uint8_t> test_vector = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

    EXPECT_EQ(test_string, redis::vector_to_string(test_vector));
}

}