#pragma once
#include <cstdint>
#include <string>

namespace error_messages
{
    const std::string INVALID_ARGUMENTS = "Invalid arguments";
}

namespace round_types
{
    const int8_t TO_ZERO = static_cast<int8_t>(0);
    const int8_t TO_CLOSEST_EVEN = static_cast<int8_t>(1);
    const int8_t TO_POSITIVE_INF = static_cast<int8_t>(2);
    const int8_t TO_NEGATIVE_INF = static_cast<int8_t>(3);
}

namespace help_functions
{
    int32_t RoundAfterMultiplication(uint64_t bytes, int64_t A, int64_t B, int8_t roundType);
}

namespace fixed_point
{
    class FixedPoint
    {
    private:
        void RoundToZero();

        void RoundToClosestEven();

        void RoundToPositiveInf();

        void RoundToNegativeInf();

        static uint64_t PrintRoundToClosestEven(uint64_t &fractionalPartFourDigits, uint64_t &tail, int64_t B);

        static uint64_t PrintRoundToPositiveInf(uint64_t &fractionalPartFourDigits, uint64_t &tail, bool isNegative);

        static uint64_t PrintRoundToNegativeInf(uint64_t &fractionalPartFourDigits, uint64_t &tail, bool isNegative);

        static FixedPoint Multiply(FixedPoint left, FixedPoint right);

        static FixedPoint Sum(FixedPoint left, FixedPoint right);

        static FixedPoint Subtract(FixedPoint left, FixedPoint right);

    public:
        int32_t A_{};
        int32_t B_{};
        int32_t bytes_{};
        int8_t roundType_{};

        FixedPoint() = default;

        FixedPoint(int32_t A_, int32_t B_, int32_t bytes_, int8_t roundType_);

        ~FixedPoint() = default;

        void Round();

        void Print();

        static FixedPoint ProcessOperation(FixedPoint left, FixedPoint right, int8_t operation);
    };
} //namespace fixed_point
