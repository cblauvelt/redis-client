#include "redis/value.hpp"

#include <algorithm>

namespace redis {

value::value()
    : value_()
    , type_(redis_type::nil) {}

value::value(string val)
    : value_(val)
    , type_(redis_type::simple_string) {}

value::value(error val)
    : value_(std::move(val))
    , type_(redis_type::error) {}

value::value(int64_t val)
    : value_(std::move(val))
    , type_(redis_type::integer) {}

value::value(int val)
    : value_(std::move(val))
    , type_(redis_type::integer) {}

value::value(bulk_string val)
    : value_(std::move(val))
    , type_(redis_type::bulk_string) {}

value::value(redis_array val)
    : value_(std::move(val))
    , type_(redis_type::array) {}

value::value(hash val)
    : value_()
    , type_(redis_type::array) {
    redis::redis_array arr;
    for (auto [key, value] : val) {
        arr.emplace_back(std::move(key));
        arr.emplace_back(std::move(value));
    }
    value_ = std::move(arr);
}

bool value::operator==(const value& rhs) const {
    // convert bulk_string to simple string
    if (type_ == redis_type::simple_string &&
        rhs.type_ == redis_type::bulk_string) {
        return (std::get<std::string>(value_) ==
                vector_to_string(std::get<bulk_string>(rhs.value_)));
    }

    if (rhs.type_ == redis_type::simple_string &&
        type_ == redis_type::bulk_string) {
        return (std::get<std::string>(rhs.value_) ==
                vector_to_string(std::get<bulk_string>(value_)));
    }

    // If the types don't match they're not equivalent
    if (type_ != rhs.type_) {
        return false;
    }

    switch (value_.index()) {
    case 0: // null_ptr
        return true;

    case 1: // string
        return (std::get<std::string>(value_) ==
                std::get<std::string>(rhs.value_));

    case 2: // error
        return (std::get<error>(value_) == std::get<error>(rhs.value_));

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

    return true;
}

bool value::operator!=(const value& rhs) const { return !(*this == rhs); }
// bool value::operator!=(const value& rhs) const { return (type_ != rhs.type_);
// }

redis_type value::type() const { return type_; }

} // namespace redis
