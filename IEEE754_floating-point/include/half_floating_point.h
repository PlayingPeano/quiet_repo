#pragma once
#include <cstdint>

#include "constants.h"

namespace half_floating_point
{
    class HalfFloatingPoint
    {
    public:
        HalfFloatingPoint() = default;

        ~HalfFloatingPoint() = default;

        HalfFloatingPoint(const HalfFloatingPoint &) = default;

        HalfFloatingPoint &operator=(const HalfFloatingPoint &) = default;

        HalfFloatingPoint(HalfFloatingPoint &&) = default;

        HalfFloatingPoint &operator=(HalfFloatingPoint &&) = default;

        HalfFloatingPoint(int16_t bits, int8_t roundingType);

        void Print();

        static HalfFloatingPoint ProcessOperation(HalfFloatingPoint &lhs, HalfFloatingPoint &rhs, char operationType);

    private:
        void Normalize();

        void MakeBits();

        void MakeRounding(uint64_t mantissa, uint64_t rem, int32_t numOfBitsInRem, int32_t bias);

        void MakeBiggest();

        void MakeSmallest();

    public:
        int16_t bits_{};
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
