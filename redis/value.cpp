#include "redis/value.hpp"

#include <algorithm>

namespace redis {

redis_value::redis_value()
    : value_()
    , type_(redis_type::nil) {}

redis_value::redis_value(string val)
    : value_(val)
    , type_(redis_type::simple_string) {}

redis_value::redis_value(error val)
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
}

bool redis_value::operator!=(const redis_value& rhs) const { return !(*this); }

redis_type redis_value::type() const { return type_; }

} // namespace redis
