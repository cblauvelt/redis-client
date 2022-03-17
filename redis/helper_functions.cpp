
#include "helper_functions.hpp"

namespace redis {

std::vector<uint8_t> string_to_vector(std::string_view value) {
    std::vector<uint8_t> retVal;

    auto it = value.begin();
    while (it != value.end()) {
        retVal.push_back(*it++);
    }

    return retVal;
}

std::string vector_to_string(const std::vector<uint8_t>& value) {
    std::string retVal;

    auto it = value.begin();
    while (it != value.end()) {
        retVal.push_back(*it++);
    }

    return retVal;
}

} // namespace redis
