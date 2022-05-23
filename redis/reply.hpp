#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "redis/errors.hpp"
#include "redis/value.hpp"

namespace redis {

using namespace std;

/// The response returned from a call to parseReply
using parse_response = std::tuple<value, std::error_code,
                                  std::vector<std::uint8_t>::const_iterator>;

/**
 * @brief reply models a reply from the Redis Server
 */
class reply {

  public:
    /**
     * @brief Creates an empty reply.
     */
    reply() = default;

    /**
     * @brief Creates a reply from the pass buffer. Assumes that the entire
     * buffer will be consumed.
     * @param buffer The buffer that represents the reply.
     */
    reply(const std::vector<std::uint8_t>& buffer);

    /**
     * @brief Creates a reply with an empty value and an error.
     * @param error An error code that relates to the error.
     */
    reply(const std::error_code& error);

    /**
     * @brief Creates a reply from a buffer that begins with "it" and ends with
     * "end".
     * @param it An iterator that represents the beginning of the buffer.
     * @param end An Iterator that represents the end of the buffer.
     * @returns std::vector<std::uint8_t>::const_iterator An iterator that
     * points to the last point in the buffer consumed by the call to loadData.
     * If "it==end" the entire buffer was consumed.
     */
    std::vector<std::uint8_t>::const_iterator
    load_data(std::vector<std::uint8_t>::const_iterator it,
              const std::vector<std::uint8_t>::const_iterator end);

    /**
     * @returns The value contained in the reply, if any.
     */
    redis::value value() const;

    /**
     * @returns The error contained in the reply, if any.
     */
    std::error_code error() const;

  private:
    /**
     * @brief Used to parse a Redis reply of unknown type.
     * @param it An iterator that points to the start of the reply.
     * @param end An iterator that points to the end of the buffer.
     * Note that this is not necessarily the end of the message.
     */
    static parse_response
    parse_reply(std::vector<std::uint8_t>::const_iterator it,
                const std::vector<std::uint8_t>::const_iterator end);

    /**
     * @brief Used to parse a reply of type SimpleString.
     * @param it Points to the start of the string.
     * @param end Points to the end of the buffer.
     */
    static parse_response
    parse_simple_string(std::vector<std::uint8_t>::const_iterator it,
                        const std::vector<std::uint8_t>::const_iterator end);

    /**
     * @brief Used to parse a reply of type Error.
     * @param it Points to the start of the error.
     * @param end Points to the end of the buffer.
     */
    static parse_response
    parse_error(std::vector<std::uint8_t>::const_iterator it,
                const std::vector<std::uint8_t>::const_iterator end);

    /**
     * @brief Used to parse a reply of type BulkString.
     * @param it Points to the start of the string.
     * @param end Points to the end of the buffer.
     */
    static parse_response
    parse_bulk_string(std::vector<std::uint8_t>::const_iterator it,
                      const std::vector<std::uint8_t>::const_iterator end);

    /**
     * @brief Used to parse a reply of type Integer.
     * @param it Points to the start of the integer.
     * @param end Points to the end of the buffer.
     */
    static parse_response
    parse_integer(std::vector<std::uint8_t>::const_iterator it,
                  const std::vector<std::uint8_t>::const_iterator end);

    /**
     * @brief Used to parse a reply of type Array.
     * @param it Points to the start of the array.
     * @param end Points to the end of the buffer.
     */
    static parse_response
    parse_array(std::vector<std::uint8_t>::const_iterator it,
                const std::vector<std::uint8_t>::const_iterator end);

  private:
    redis::value value_;
    std::error_code error_;
};

/// Used for pipelining
using replies = std::vector<redis::reply>;

} // namespace redis
