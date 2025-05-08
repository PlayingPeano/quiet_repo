#pragma once
#include <cstdint>
#include <iostream>
#include <iomanip>

#include "constants.h"

namespace single_floating_point
{
    class SingleFloatingPoint
    {
    public:
        SingleFloatingPoint() = default;

        ~SingleFloatingPoint() = default;

        SingleFloatingPoint(const SingleFloatingPoint &) = default;

        SingleFloatingPoint &operator=(const SingleFloatingPoint &) = default;

        SingleFloatingPoint(SingleFloatingPoint &&) = default;

        SingleFloatingPoint &operator=(SingleFloatingPoint &&) = default;

        SingleFloatingPoint(int32_t bits, int8_t roundingType);

        void Print();

        static SingleFloatingPoint ProcessOperation(SingleFloatingPoint &lhs, SingleFloatingPoint &rhs,
                                                    char operationType);

    private:
        void Normalize();

        void MakeBits();

        void MakeRounding(uint64_t mantissa, uint64_t rem, int32_t numOfBitsInRem, int32_t bias);

        void MakeBiggest();

        void MakeSmallest();

    public:
        int32_t bits_{};
        int32_t exp_{};
        int32_t mantissa_{};
        int8_t sign_{};
        int8_t roundingType_{};
        bool isInf_{};
        bool isNan_{};
        bool isZero_{};
        bool isDenormalized_{};
    };
}
