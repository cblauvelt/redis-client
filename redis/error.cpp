#include "redis/error.hpp"

namespace redis {

error::error(const std::string& what_arg)
    : std::runtime_error(what_arg) {}

error::error(const char* what_arg)
    : std::runtime_error(what_arg) {}

bool error::operator==(const error& rhs) const {
    std::string lhsWhat = this->what();
    std::string rhsWhat = rhs.what();

    return (lhsWhat == rhsWhat);
}

} // namespace redis
