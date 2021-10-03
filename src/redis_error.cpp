#include "redis_error.hpp"

namespace redis {

redis_error::redis_error(const std::string& what_arg)
    : std::runtime_error(what_arg) {}

redis_error::redis_error(const char* what_arg)
    : std::runtime_error(what_arg) {}

bool redis_error::operator==(const redis_error& rhs) const {
    std::string lhsWhat = this->what();
    std::string rhsWhat = rhs.what();

    return (lhsWhat == rhsWhat);
}

} // namespace redis
