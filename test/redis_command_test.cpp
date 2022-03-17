#include "redis/command.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using string = std::string;

TEST(Redis_Command, Null_Commands) {
    redis::command nullCommand("");
    EXPECT_TRUE(nullCommand.empty());
    EXPECT_EQ(nullCommand.commands().size(), 0);

    nullCommand = redis::command("   ");
    EXPECT_TRUE(nullCommand.empty());
    EXPECT_EQ(nullCommand.commands().size(), 0);
}

TEST(Redis_Command, PING) {
    // Test ideal case
    redis::command command("PING");
    EXPECT_FALSE(command.empty());
    EXPECT_EQ(command.commands().size(), 1);
    EXPECT_EQ(command.serialized_command(), "PING\r\n");

    // Test with leading or lagging white space
    command = redis::command("  PING  ");
    EXPECT_FALSE(command.empty());
    EXPECT_EQ(command.commands().size(), 1);
    EXPECT_EQ(command.serialized_command(), "PING\r\n");

    EXPECT_EQ(command, redis::command("PING"));
}

TEST(Redis_Command, GET_Key) {
    // Test ideal case
    redis::command command("GET temp");
    EXPECT_FALSE(command.empty());
    std::vector<string> commands{"GET", "temp"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nGET\r\n$4\r\ntemp\r\n");

    // Test with extra white space
    command = redis::command("GET  temp ");
    EXPECT_FALSE(command.empty());
    commands = std::vector<string>{"GET", "temp"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nGET\r\n$4\r\ntemp\r\n");

    // Test with with quotes
    command = redis::command("GET  \"temp with quotes\" ");
    EXPECT_FALSE(command.empty());
    commands = std::vector<string>{"GET", "temp with quotes"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nGET\r\n$16\r\ntemp with quotes\r\n");

    // Test with with just 1 quote
    command = redis::command("GET  \"temp with quotes ");
    EXPECT_FALSE(command.empty());
    commands = std::vector<string>{"GET", "temp with quotes "};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nGET\r\n$17\r\ntemp with quotes \r\n");

    // Test with the commands already broken down
    commands = std::vector<string>{"GET", "temp"};
    command = redis::command(commands);
    EXPECT_FALSE(command.empty());
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nGET\r\n$4\r\ntemp\r\n");
}

TEST(Redis_Command, SET_Key_Value) {
    // Test ideal case
    redis::command command("GET key value");
    EXPECT_FALSE(command.empty());
    std::vector<string> commands{"GET", "key", "value"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*3\r\n$3\r\nGET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");

    // Test second ideal case
    commands = std::vector<string>{"GET", "key", "value"};
    command = redis::command(commands);
    EXPECT_FALSE(command.empty());
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*3\r\n$3\r\nGET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");

    // Test with extra white space
    command = redis::command("GET  key  value ");
    EXPECT_FALSE(command.empty());
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*3\r\n$3\r\nGET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");

    // Test with with quotes
    command =
        redis::command("GET  \"key with quotes\"  \"value with quotes\" ");
    EXPECT_FALSE(command.empty());
    commands =
        std::vector<string>{"GET", "key with quotes", "value with quotes"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*3\r\n$3\r\nGET\r\n$15\r\nkey with quotes\r\n$17\r\nvalue with "
              "quotes\r\n");

    // Test with with just 1 quote
    command = redis::command("GET  \"key with quotes\"  \"value with quotes ");
    EXPECT_FALSE(command.empty());
    commands =
        std::vector<string>{"GET", "key with quotes", "value with quotes "};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*3\r\n$3\r\nGET\r\n$15\r\nkey with quotes\r\n$18\r\nvalue with "
              "quotes \r\n");
}

TEST(Redis_Command, DEL_Key) {
    // Test ideal case
    redis::command command("DEL temp");
    EXPECT_FALSE(command.empty());
    std::vector<string> commands{"DEL", "temp"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nDEL\r\n$4\r\ntemp\r\n");

    // Test with extra white space
    command = redis::command("DEL  temp ");
    EXPECT_FALSE(command.empty());
    commands = std::vector<string>{"DEL", "temp"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nDEL\r\n$4\r\ntemp\r\n");

    // Test with with quotes
    command = redis::command("DEL  \"temp with quotes\" ");
    EXPECT_FALSE(command.empty());
    commands = std::vector<string>{"DEL", "temp with quotes"};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nDEL\r\n$16\r\ntemp with quotes\r\n");

    // Test with with just 1 quote
    command = redis::command("DEL  \"temp with quotes ");
    EXPECT_FALSE(command.empty());
    commands = std::vector<string>{"DEL", "temp with quotes "};
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nDEL\r\n$17\r\ntemp with quotes \r\n");

    // Test with the commands already broken down
    commands = std::vector<string>{"DEL", "temp"};
    command = redis::command(commands);
    EXPECT_FALSE(command.empty());
    EXPECT_EQ(command.commands(), commands);
    EXPECT_EQ(command.serialized_command(),
              "*2\r\n$3\r\nDEL\r\n$4\r\ntemp\r\n");
}

} // namespace