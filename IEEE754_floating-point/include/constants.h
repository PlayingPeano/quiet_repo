#pragma once
#include <cstdint>
#include <string>

namespace rounding_types_constants
{
    const int8_t ROUND_TO_ZERO = 0;
    const int8_t ROUND_TO_CLOSEST_EVEN = 1;
    const int8_t ROUND_TO_POSITIVE_INF = 2;
    const int8_t ROUND_TO_NEGATIVE_INF = 3;
}

namespace error_messages
{
    const std::string INVALID_ARGUMENTS = "Invalid arguments";
}