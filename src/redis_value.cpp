#include "redis_value.hpp"
#include "redis_message.hpp"

#include <algorithm>
#include <iostream>

namespace redis {

using std::cout;
using std::endl;

redis_value::redis_value()
    : value_()
    , type_(redis_type::nil) {}

redis_value::redis_value(string val)
    : value_(val)
    , type_(redis_type::simple_string) {}

redis_value::redis_value(redis_error val)
    : value_(std::move(val))
    , type_(redis_type::error) {}

redis_value::redis_value(int64_t val)
    : value_(std::move(val))
    , type_(redis_type::integer) {}

redis_value::redis_value(int val)
    : value_(std::move(val))
    , type_(redis_type::integer) {}

redis_value::redis_value(bulk_string val)
    : value_(std::move(val))
    , type_(redis_type::bulk_string) {}

redis_value::redis_value(redis_array val)
    : value_(std::move(val))
    , type_(redis_type::array) {}

bool redis_value::operator==(const redis_value& rhs) const {
    if (type_ != rhs.type_) {
        return false;
    }

    switch (value_.index()) {
    case 0: // null_ptr
        return true;

    case 1: // string
        return (std::get<std::string>(value_) ==
                std::get<std::string>(rhs.value_));

    case 2: // redis_error
        return (std::get<redis_error>(value_) ==
                std::get<redis_error>(rhs.value_));

    case 3: // int64_t
        return (std::get<int64_t>(value_) == std::get<int64_t>(rhs.value_));

    case 4: // bulk_string
        return std::equal(std::begin(std::get<bulk_string>(value_)),
                          std::end(std::get<bulk_string>(value_)),
                          std::begin(std::get<bulk_string>(rhs.value_)));

    case 5: // redis_array
        return std::equal(std::begin(std::get<redis_array>(value_)),
                          std::end(std::get<redis_array>(value_)),
                          std::begin(std::get<redis_array>(rhs.value_)));

    default:
        return false;
    }
}

bool redis_value::operator!=(const redis_value& rhs) const { return !(*this); }

redis_type redis_value::type() const { return type_; }

template <> std::optional<redis_error> redis_value::as<>() const {
    if (type_ == redis_type::error &&
        std::holds_alternative<redis_error>(value_)) {
        return std::get<redis_error>(value_);
    }

    return std::nullopt;
}

template <> std::optional<int64_t> redis_value::as<>() const {
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

template <> std::optional<int> redis_value::as<>() const {
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

template <> std::optional<float> redis_value::as<>() const {
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

template <> std::optional<double> redis_value::as<>() const {
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
        }
    }

    return std::nullopt;
}

template <> std::optional<bulk_string> redis_value::as<>() const {
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

template <> std::optional<redis_array> redis_value::as<>() const {
    if (type_ == redis_type::array &&
        std::holds_alternative<redis_array>(value_)) {
        return std::get<redis_array>(value_);
    }

    return std::nullopt;
}

template <> std::optional<redis_message> redis_value::as<>() const {
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

    return redis_message(arr);
}

template <> std::optional<bool> redis_value::as<>() const {
    if (type_ == redis_type::integer &&
        std::holds_alternative<int64_t>(value_)) {
        return (bool)(std::get<int64_t>(value_));
    }

    if (type_ == redis_type::simple_string &&
        std::holds_alternative<string>(value_)) {
        return (std::get<string>(value_) == "OK");
    }

    if (type_ == redis_type::error &&
        std::holds_alternative<redis_error>(value_)) {
        return false;
    }

    return std::nullopt;
}

} // namespace redis
