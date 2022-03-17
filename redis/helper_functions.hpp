#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace redis {

/**
 * @brief Converts a std::string to a std::vector<uint8_t>
 * @param value The string to convert.
 * @returns std::vector<uint8_t> The converted vector.
 */
std::vector<uint8_t> string_to_vector(std::string_view value);

/**
 * @brief Converts a std::vector<uint8_t> to a std::string.
 * @param value The std::vector<uint8_t> to convert.
 * @returns std::string The result of the conversion.
 */
std::string vector_to_string(const std::vector<uint8_t>& value);

} // namespace redis
