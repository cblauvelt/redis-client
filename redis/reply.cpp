
#include "redis/reply.hpp"

namespace redis {

reply::reply(const std::vector<uint8_t>& buffer) {
    std::tie(value_, error_, std::ignore) =
        parse_reply(buffer.begin(), buffer.end());
}

reply::reply(const std::error_code& error)
    : error_(error) {}

std::vector<uint8_t>::const_iterator
reply::load_data(std::vector<uint8_t>::const_iterator it,
                 const std::vector<uint8_t>::const_iterator end) {
    std::vector<uint8_t>::const_iterator retIt = it;
    std::tie(value_, error_, retIt) = parse_reply(it, end);
    return retIt;
}

redis_value reply::value() const { return value_; }

std::error_code reply::error() const { return error_; }

parse_response
reply::parse_reply(std::vector<uint8_t>::const_iterator it,
                   const std::vector<uint8_t>::const_iterator end) {
    redis_value value;
    std::error_code error;

    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, end);
    }

    // Discover return type
    switch (*it++) {
    case '+': // Simple String
        return parse_simple_string(it, end);
    case '-': // Error
        return parse_error(it, end);
    case '$': // Bulk String
        return parse_bulk_string(it, end);
    case '*': // Array
        return parse_array(it, end);
    case ':': // Integer
        return parse_integer(it, end);
    default:
        break;
    }

    return parse_response(redis_value(), error, end);
}

parse_response
reply::parse_simple_string(std::vector<uint8_t>::const_iterator it,
                           const std::vector<uint8_t>::const_iterator end) {
    string value;
    std::error_code error;

    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }

    while (it != end && *it != '\r') {
        value.push_back(*it++);
    }
    // consume the '\r\n'
    it += 2;

    return parse_response(redis_value(value), error, it);
}

parse_response
reply::parse_error(std::vector<uint8_t>::const_iterator it,
                   const std::vector<uint8_t>::const_iterator end) {
    string value;
    std::error_code error;

    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }

    while (it != end && *it != '\r') {
        value.push_back(*it++);
    }
    // if we reached the end before we got to '\r' it's a parse error
    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }
    // consume the '\r\n'
    it += 2;

    return parse_response(redis_value(redis::error(value)),
                          client_error_code::error, it);
}

parse_response
reply::parse_bulk_string(std::vector<uint8_t>::const_iterator it,
                         const std::vector<uint8_t>::const_iterator end) {
    string header;
    std::vector<uint8_t> value;
    std::error_code error;

    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }

    // parse header
    while (it != end && *it != '\r') {
        header.push_back(*it++);
    }

    // if we reached the end before we got to '\r' it's a parse error
    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }
    // consume the '\r\n'
    it += 2;
    int64_t stringSize = 0;
    try {
        // determine size of bulk string
        stringSize = stoll(header);
    } catch (const std::invalid_argument& ex) {
        return parse_response(redis_value(),
                              parse_error_code::malformed_message, it);
    } catch (const std::out_of_range& ex) {
        return parse_response(redis_value(),
                              parse_error_code::malformed_message, it);
    }

    // if the size of the string is -1 for null string or 0 for empty
    // string, there is nothing to copy
    if (stringSize == -1) {
        return parse_response(redis_value(), error, it);
    }

    // copy the string
    value.resize(stringSize);

    // copy the string into value
    try {
        copy(it, it + stringSize, value.begin());
    } catch (std::out_of_range& ex) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }
    // consume the bulk string and the '\r\n'
    it += stringSize + 2;

    return parse_response(redis_value(value), error, it);
}

parse_response
reply::parse_integer(std::vector<uint8_t>::const_iterator it,
                     const std::vector<uint8_t>::const_iterator end) {
    string message;
    int value;
    std::error_code error;

    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }

    while (it != end && *it != '\r') {
        message.push_back(*it++);
    }
    // if we reached the end before we got to '\r' it's a parse error
    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }
    // consume the '\r\n'
    it += 2;

    try {
        value = stoll(message);
        return parse_response(redis_value(value), error, it);
    } catch (...) {
        return parse_response(redis_value(), parse_error_code::out_of_range,
                              it);
    }
}

parse_response
reply::parse_array(std::vector<uint8_t>::const_iterator it,
                   const std::vector<uint8_t>::const_iterator end) {
    string header;
    std::error_code error;
    redis_array array;

    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }

    // parse the header
    while (it != end && *it != '\r') {
        header.push_back(*it++);
    }
    // return error if we're already at the end
    if (it == end) {
        return parse_response(redis_value(), parse_error_code::eof, it);
    }
    // consume the '\r\n'
    it += 2;

    // determine size of bulk string
    int64_t arraySize = stoll(header);

    redis_value tempValue;
    for (int64_t i = 0; i < arraySize; i++) {
        if (it == end) {
            return parse_response(redis_value(), parse_error_code::eof, it);
        }

        std::tie(tempValue, error, it) = parse_reply(it, end);
        if (error) {
            return parse_response(redis_value(), error, it);
        }
        array.push_back(tempValue);
    }
    // we don't need to consume the '\r\n' because that's done by
    // parse_reply

    return parse_response(redis_value(array), error, it);
}

} // namespace redis
