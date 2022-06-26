#include "redis/value.hpp"

#include <algorithm>
#include <stdexcept>

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

bool value::operator<(const value& rhs) const {
    if (type_ != rhs.type_) {
        return (type_ < rhs.type_);
    }

    switch (type_) {
    case redis_type::nil:
        return false;

    case redis_type::simple_string:
        return (std::get<std::string>(value_) <
                std::get<std::string>(rhs.value_));

    case redis_type::error:
        return (std::get<redis::error>(value_).what() <
                std::get<redis::error>(rhs.value_).what());

    case redis_type::integer:
        return (std::get<int64_t>(value_) < std::get<int64_t>(rhs.value_));

    case redis_type::bulk_string:
        return (std::get<redis::bulk_string>(value_) <
                std::get<redis::bulk_string>(rhs.value_));

    case redis_type::array:
        return (std::get<redis_array>(value_) <
                std::get<redis_array>(rhs.value_));

    default:
        throw std::out_of_range("redis value type is not supported");
    }
}

redis_type value::type() const { return type_; }

ostream& operator<<(ostream& os, const redis::value& val) {
    redis_array arrVal;

    switch (val.type()) {
    case redis_type::nil:
        os << "(nil)";
        break;

    case redis_type::simple_string:
        os << (std::string)val;
        break;

    case redis_type::error:
        os << ((error)val).what();
        break;

    case redis_type::integer:
        os << (int64_t)val;
        break;

    case redis_type::bulk_string:
        os << '[' << absl::StrJoin((redis::bulk_string)val, ",") << ']';
        break;

    case redis_type::array:
        arrVal = (redis_array)val;
        if (arrVal.empty()) {
            os << "[]";
            break;
        }

        {
            auto arrIt = arrVal.cbegin();
            os << '[' << *arrIt++;
            while (arrIt != arrVal.cend()) {
                os << *arrIt++;
            }
            os << ']';
        }

        break;

    default:
        os << "(unknown type)";
        break;
    }

    return os;
}

} // namespace redis
