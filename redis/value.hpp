#pragma once

#include <algorithm> // find_if_not
#include <cctype>    // isdigit
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

#include "helper_functions.hpp"
#include "redis/error.hpp"
#include "redis/message.hpp"

namespace redis {

class redis_value;
using bulk_string = std::vector<uint8_t>;
using redis_array = std::vector<redis_value>;
using string = std::string;

/**
 * @brief Used to define what is held by a redis_value.
 */
enum class redis_type : uint8_t {
    /// nil A redis null value
    nil = 0,
    /// simple_string A redis Simple String
    simple_string,
    /// error A redis error
    error,
    /// integer A redis integer
    integer,
    /// bulk_string A redis bulk string
    bulk_string,
    /// array An array of other redis values
    array

};

/**
 * @brief Used to hold the various responses that can be returned by a redis
 * value
 */
class redis_value {

  public:
    /**
     * @brief Creates a redis_value of type nil.
     */
    redis_value();

    /**
     * @brief Creates a redis_value of type simple_string
     * @param val The string to hold within the redis_value
     */
    redis_value(string val);

    /**
     * @brief Creates a redis_value of type error
     * @param val The error to hold within the redis_value
     */
    redis_value(error val);

    /**
     * @brief Creates a redis_value that holds a signed 64-bit integer
     * @param val The string to hold within the redis_value
     */
    redis_value(int64_t val);

    /**
     * @brief Creates a redis_value that holds a signed 64-bit integer
     * @param val The string to hold within the redis_value
     */
    redis_value(int val);

    /**
     * @brief Creates a redis_value of type bulk_string
     * @param val The string to hold within the redis_value
     */
    redis_value(bulk_string val);

    /**
     * @brief Creates a redis_value that is an array of redis_values
     * @param val The string to hold within the redis_value
     */
    redis_value(redis_array val);

    /**
     * @brief Equality operator for redis_value.
     * @returns bool true if the type and the value are the same.
     */
    bool operator==(const redis_value& rhs) const;

    /**
     * @brief Inequality operator for redis_value.
     * @returns bool true if either the type or the value are different.
     */
    bool operator!=(const redis_value& rhs) const;

    /**
     * @brief A template function that converts the redis_value into an optional
     * of the requested value type.
     */
    template <typename T> std::optional<T> as() const;

    /**
     * @brief A template function that converts the redis_value into the
     * requested type. throws an error on failure.
     */
    template <typename T> operator T() const;

    /**
     * @brief A template function that assigns the conversion of a redis_value
     * to the requested type. throws an error on failure.
     */
    template <typename T> T& operator=(const redis_value& rhs);

    /**
     * @brief Returns the redis_type of the redis_value
     */
    redis_type type() const;

  private:
    std::variant<std::nullptr_t, std::string, error, int64_t, bulk_string,
                 redis_array>
        value_;

    redis_type type_;
};

template <typename T> redis_value::operator T() const {
    std::optional<T> retVal = as<T>();
    if (!retVal.has_value()) {
        throw std::bad_variant_access();
    }
    return retVal.value();
}

template <typename T> inline std::optional<T> redis_value::as() const {
    return std::nullopt;
}

template <typename T> T& redis_value::operator=(const redis_value& rhs) {
    return (T)(rhs);
}

// Specialized template functions
template <> inline std::optional<string> redis_value::as<>() const {
    if (type_ == redis_type::simple_string &&
        std::holds_alternative<string>(value_)) {
        return std::get<string>(value_);
    }

    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        return vector_to_string(std::get<bulk_string>(value_));
    }

    if (type_ == redis_type::error && std::holds_alternative<error>(value_)) {
        return string(std::get<error>(value_).what());
    }

    return std::nullopt;
}

template <> inline std::optional<error> redis_value::as<>() const {
    if (type_ == redis_type::error && std::holds_alternative<error>(value_)) {
        return std::get<error>(value_);
    }

    return std::nullopt;
}

template <> inline std::optional<int64_t> redis_value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return std::get<int64_t>(value_);
    }

    // if the string is a number, convert it
    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        bulk_string value = std::get<bulk_string>(value_);
        try {
            string conversion = vector_to_string(value);
            return std::stoll(conversion);
        } catch (...) {
        }
    }

    return std::nullopt;
}

template <> inline std::optional<int> redis_value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return (int)std::get<int64_t>(value_);
    }

    // if the string is a number, convert it
    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        bulk_string value = std::get<bulk_string>(value_);
        try {
            string conversion = vector_to_string(value);
            return std::stoi(conversion);
        } catch (...) {
        }
    }

    return std::nullopt;
}

template <> inline std::optional<float> redis_value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return (float)std::get<int64_t>(value_);
    }

    // if the string is a number, convert it
    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        bulk_string value = std::get<bulk_string>(value_);

        try {
            string conversion = vector_to_string(value);
            return std::stof(conversion);
        } catch (...) {
        }
    }

    return std::nullopt;
}

template <> inline std::optional<double> redis_value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return (int)std::get<int64_t>(value_);
    }

    // if the string is a number, convert it
    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        bulk_string value = std::get<bulk_string>(value_);

        try {
            string conversion = vector_to_string(value);
            return std::stod(conversion);
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

template <> inline std::optional<bulk_string> redis_value::as<>() const {
    if (type_ == redis_type::simple_string &&
        std::holds_alternative<string>(value_)) {
        return string_to_vector(std::get<string>(value_));
    }

    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        return std::get<bulk_string>(value_);
    }

    return std::nullopt;
}

template <> inline std::optional<redis_array> redis_value::as<>() const {
    if (type_ == redis_type::array &&
        std::holds_alternative<redis_array>(value_)) {
        return std::get<redis_array>(value_);
    }

    return std::nullopt;
}

template <> inline std::optional<redis_message> redis_value::as<>() const {
    if (type_ != redis_type::array ||
        !std::holds_alternative<redis_array>(value_)) {
        return std::nullopt;
    }

    auto arr = std::get<redis_array>(value_);
    if (arr.size() < 3) {
        return std::nullopt;
    }

    auto message_type = arr[0].as<string>().value_or("");
    if (message_type != "message" && message_type != "pmessage") {
        return std::nullopt;
    }

    redis_message message;
    if (arr.size() == 3) {
        if (arr[0].as<string>().value_or("") != "message") {
            return std::nullopt;
        }

        message.channel = arr[1].as<string>().value_or("");
        message.contents = arr[2].as<string>().value_or("");
    }

    if (arr.size() == 4) {
        if (arr[0].as<string>().value_or("") != "pmessage") {
            return std::nullopt;
        }

        message.pattern = arr[1].as<string>().value_or("");
        message.channel = arr[2].as<string>().value_or("");
        message.contents = arr[3].as<string>().value_or("");
    }

    return message;
}

template <> inline std::optional<bool> redis_value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return (bool)(std::get<int64_t>(value_));
    }

    if (type_ == redis_type::simple_string &&
        std::holds_alternative<string>(value_)) {
        return (std::get<string>(value_) == "OK");
    }

    if (type_ == redis_type::error && std::holds_alternative<error>(value_)) {
        return false;
    }

    return std::nullopt;
}

} // namespace redis
