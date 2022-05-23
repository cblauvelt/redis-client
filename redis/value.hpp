#pragma once

#include <algorithm> // find_if_not
#include <cctype>    // isdigit
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

#include "redis/error.hpp"
#include "redis/helper_functions.hpp"
#include "redis/message.hpp"

namespace redis {

class value;
using bulk_string = std::vector<uint8_t>;
using redis_array = std::vector<redis::value>;
using hash = std::map<std::string, redis::value>;
using string = std::string;

/**
 * @brief Used to define what is held by a value.
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
class value {

  public:
    /**
     * @brief Creates a value of type nil.
     */
    value();

    /**
     * @brief Creates a value of type simple_string
     * @param val The string to hold within the value
     */
    value(string val);

    /**
     * @brief Creates a value of type error
     * @param val The error to hold within the value
     */
    value(error val);

    /**
     * @brief Creates a value that holds a signed 64-bit integer
     * @param val The string to hold within the value
     */
    value(int64_t val);

    /**
     * @brief Creates a value that holds a signed 64-bit integer
     * @param val The string to hold within the value
     */
    value(int val);

    /**
     * @brief Creates a value of type bulk_string
     * @param val The string to hold within the value
     */
    value(bulk_string val);

    /**
     * @brief Creates a value that is an array of redis_values
     * @param val The array to hold within the value
     */
    value(redis_array val);

    /**
     * @brief Creates a value that is hash of redis_values
     * @param val The hash to hold within the value
     */
    value(hash val);

    /**
     * @brief Equality operator for value.
     * @returns bool true if the type and the value are the same.
     */
    bool operator==(const value& rhs) const;

    /**
     * @brief Inequality operator for value.
     * @returns bool true if either the type or the value are different.
     */
    bool operator!=(const value& rhs) const;

    /**
     * @brief A template function that converts the value into an optional
     * of the requested value type.
     */
    template <typename T> std::optional<T> as() const;

    /**
     * @brief A template function that converts the value into the
     * requested type. throws an error on failure.
     */
    template <typename T> operator T() const;

    /**
     * @brief A template function that assigns the conversion of a value
     * to the requested type. throws an error on failure.
     */
    template <typename T> T& operator=(const value& rhs);

    /**
     * @brief Returns the redis_type of the value
     */
    redis_type type() const;

  private:
    std::variant<std::nullptr_t, std::string, error, int64_t, bulk_string,
                 redis_array>
        value_;

    redis_type type_;
};

template <typename T> value::operator T() const {
    std::optional<T> retVal = as<T>();
    if (!retVal.has_value()) {
        throw std::bad_cast();
    }
    return retVal.value();
}

template <typename T> inline std::optional<T> value::as() const {
    return std::nullopt;
}

template <typename T> T& value::operator=(const value& rhs) { return (T)(rhs); }

// Specialized template functions
template <> inline std::optional<string> value::as<>() const {
    if (type_ == redis_type::simple_string &&
        std::holds_alternative<string>(value_)) {
        return std::get<string>(value_);
    }

    if (type_ == redis_type::bulk_string &&
        std::holds_alternative<bulk_string>(value_)) {
        return vector_to_string(std::get<bulk_string>(value_));
    }

    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return std::to_string(std::get<int64_t>(value_));
    }

    if (type_ == redis_type::error && std::holds_alternative<error>(value_)) {
        return string(std::get<error>(value_).what());
    }

    return std::nullopt;
}

template <> inline std::optional<error> value::as<>() const {
    if (type_ == redis_type::error && std::holds_alternative<error>(value_)) {
        return std::get<error>(value_);
    }

    return std::nullopt;
}

template <> inline std::optional<int64_t> value::as<>() const {
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
            return std::stol(conversion);
        } catch (...) {
        }
    }

    return std::nullopt;
}

template <> inline std::optional<long long> value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return (long long)std::get<int64_t>(value_);
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

template <> inline std::optional<int> value::as<>() const {
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

template <> inline std::optional<float> value::as<>() const {
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

template <> inline std::optional<double> value::as<>() const {
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

template <> inline std::optional<redis::hash> value::as<>() const {
    if (type_ == redis_type::array &&
        std::holds_alternative<redis::redis_array>(value_)) {
        redis::redis_array arr = std::get<redis::redis_array>(value_);

        // requires an even number of keys
        if (arr.size() % 2) {
            return std::nullopt;
        }

        auto hash_map = redis::hash{};
        try {
            auto it = arr.cbegin();
            while (it != arr.cend()) {
                hash_map.insert({(string)*it++, *it++});
            }
        } catch (...) {
            return std::nullopt;
        }

        return hash_map;
    }

    return std::nullopt;
}

template <> inline std::optional<bulk_string> value::as<>() const {
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

template <> inline std::optional<redis_array> value::as<>() const {
    if (type_ == redis_type::array &&
        std::holds_alternative<redis_array>(value_)) {
        return std::get<redis_array>(value_);
    }

    return std::nullopt;
}

template <> inline std::optional<redis_message> value::as<>() const {
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

template <> inline std::optional<bool> value::as<>() const {
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
