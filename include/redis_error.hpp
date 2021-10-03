#pragma once

#include <stdexcept>
#include <string>

namespace redis {

/**
 * @brief An sub-type of std::runtime_error which represents a Redis Error.
 * This can be thrown as an exception since this also inherits from
 * std::exception.
 */
class redis_error : public std::runtime_error {

  public:
    explicit redis_error(const std::string& what_arg);
    explicit redis_error(const char* what_arg);

    bool operator==(const redis_error& rhs) const;
};

} // namespace redis