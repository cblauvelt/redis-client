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
#include "redis_error.hpp"

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
    redis_value(redis_error val);

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
    std::variant<std::nullptr_t, std::string, redis_error, int64_t, bulk_string,
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

    if (type_ == redis_type::error &&
        std::holds_alternative<redis_error>(value_)) {
        return string(std::get<redis_error>(value_).what());
    }

    return std::nullopt;
}

} // namespace redis
