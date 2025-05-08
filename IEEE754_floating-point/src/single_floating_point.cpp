#include "single_floating_point.h"
#include "constants.h"

#include <iostream>
#include <iomanip>


namespace single_floating_point
{
    SingleFloatingPoint::SingleFloatingPoint(int32_t bits, int8_t roundingType)
    {
        roundingType_ = roundingType;
        bits_ = bits;
        sign_ = (bits < 0) ? -1 : 1;
        int32_t mask = (~((1u << 31u) | ((1u << 23u) - 1u)));
        exp_ = ((bits & mask) >> 23) - 127;
        mantissa_ = static_cast<int32_t>(((1u << 23u) - 1u & bits) << 1);
        if (exp_ == -127 && mantissa_ != 0)
        {
            isDenormalized_ = true;
        } else if (mantissa_ == -127 && exp_ == 0)
        {
            isZero_ = true;
        } else if (exp_ == 128 && mantissa_ != 0)
        {
            isNan_ = true;
        } else if (exp_ == 128 && mantissa_ == 0)
        {
            isInf_ = true;
        }
    }

    void SingleFloatingPoint::Print()
    {
        if (isDenormalized_)
        {
            Normalize();
            std::cout << ((sign_ < 0) ? "-" : "") << "0x1." << std::hex << std::setw(6) << std::setfill('0') <<
                    mantissa_;
            std::cout << std::dec << 'p' << ((exp_ >= 0) ? "+" : "") << exp_ << " ";

            std::cout << "0x";
            std::cout << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << bits_ << std::endl;
        } else if (isZero_)
        {
            if (sign_ < 0)
            {
                std::cout << "-0x0.000000p+0 0x80000000" << std::endl;
            } else
            {
                std::cout << "0x0.000000p+0 0x00000000" << std::endl;
            }
        } else if (isNan_)
        {
            std::cout << "nan ";
            std::cout << "0x";
            std::cout << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << bits_ << std::endl;
        } else if (isInf_)
        {
            if (sign_ < 0)
            {
                std::cout << "-inf 0xFF800000" << std::endl;
            } else
            {
                std::cout << "inf 0x7F800000" << std::endl;
            }
        } else
        {
            std::cout << ((sign_ < 0) ? "-" : "") << "0x1." << std::hex << std::setw(6) << std::setfill('0') <<
                    mantissa_;
            std::cout << std::dec << 'p' << ((exp_ >= 0) ? "+" : "") << exp_ << " ";

            std::cout << "0x";
            std::cout << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << bits_ << std::endl;
        }
    }

    SingleFloatingPoint SingleFloatingPoint::ProcessOperation(SingleFloatingPoint &lhs, SingleFloatingPoint &rhs,
                                                              char operationType)
    {
        SingleFloatingPoint result;
        if (operationType == '*')
        {
            if (lhs.isNan_)
            {
                return lhs;
            }
            if (rhs.isNan_)
            {
                return rhs;
            }
            if ((lhs.isZero_ && rhs.isInf_) || (lhs.isInf_ && rhs.isZero_))
            {
                result.bits_ = static_cast<int32_t>(0xFFC00000);
                result.isNan_ = true;
                return result;
            }
            if (lhs.isZero_ || rhs.isZero_)
            {
                result.isZero_ = true;
                result.sign_ = lhs.sign_ * rhs.sign_;
                return result;
            }
            if (lhs.isInf_ || rhs.isInf_)
            {
                result.isInf_ = true;
                result.sign_ = lhs.sign_ * rhs.sign_;
                return result;
            }

            result.sign_ = lhs.sign_ * rhs.sign_;
            lhs.Normalize();
            rhs.Normalize();
            result.exp_ = lhs.exp_ + rhs.exp_;
            int32_t mask = 1 << 24;
            uint64_t mantissa = static_cast<uint64_t>(lhs.mantissa_ | mask) * static_cast<uint64_t>(
                                    rhs.mantissa_ | mask);

            result.roundingType_ = lhs.roundingType_;

            int32_t countForRem{};
            mantissa >>= 2ull;
            uint64_t rem = mantissa & ((1ull << 23ull) - 1ull);
            mantissa >>= 23ull;
            countForRem += 23;

            bool extraBitWasAdded = false;
            if (mantissa & (1ull << 24ull))
            {
                ++result.exp_;
                rem |= ((mantissa & 1ull) << 23ull);
                mantissa >>= 1ull;
                extraBitWasAdded = true;
                ++countForRem;
            }
            mantissa <<= 1ull;
            if (result.exp_ > 127)
            {
                if ((result.roundingType_ == rounding_types_constants::ROUND_TO_CLOSEST_EVEN) ||
                    (result.roundingType_ == rounding_types_constants::ROUND_TO_POSITIVE_INF && result.sign_ > 0) ||
                    (result.roundingType_ == rounding_types_constants::ROUND_TO_NEGATIVE_INF && result.sign_ < 0))
                {
                    result.isInf_ = true;
                } else
                {
                    result.MakeBiggest();
                }
            } else if (result.exp_ < -126)
            {
                int32_t bias = -126 - result.exp_;
                if (bias <= 24)
                {
                    mantissa >>= 1ull;
                    rem |= (mantissa & ((1ull << bias) - 1ull)) << (23ull + (extraBitWasAdded ? 1ull : 0ull));
                    mantissa >>= bias;
                    mantissa <<= bias + 1ull;
                    countForRem += bias;
                    result.MakeRounding(mantissa, rem, countForRem, bias);
                } else
                {
                    if (result.roundingType_ == rounding_types_constants::ROUND_TO_POSITIVE_INF && result.sign_ > 0)
                    {
                        result.exp_ = -149;
                        result.bits_ = 0x00000001;
                        result.mantissa_ = 0;
                    } else if (result.roundingType_ == rounding_types_constants::ROUND_TO_NEGATIVE_INF &&
                               result.sign_ < 0)
                    {
                        result.exp_ = -149;
                        result.bits_ = static_cast<int32_t>(0x80000001);
                        result.mantissa_ = 0;
                    } else
                    {
                        result.MakeSmallest();
                    }
                }
            } else
            {
                result.MakeRounding(mantissa, rem, countForRem, 0);
            }
            result.MakeBits();
            return result;
        }
        return result;
    }

    void SingleFloatingPoint::Normalize()
    {
        if (!isDenormalized_)
        {
            return;
        }
        int32_t mask = 1 << 24;
        while ((mask & mantissa_) == 0)
        {
            mantissa_ <<= 1;
            --exp_;
            if (mantissa_ == 0)
            {
                throw std::invalid_argument("Invalid mantissa");
            }
        }
        ++exp_;
        mantissa_ ^= mask;

        isDenormalized_ = false;
    }

    void SingleFloatingPoint::MakeBits()
    {
        bits_ = 0;
        bits_ |= mantissa_ >> 1;
        if (exp_ >= -126)
        {
            bits_ |= (exp_ + 127) << 23;
        } else
        {
            bits_ |= (-126 + 127) << 23;
            bits_ >>= -126 - exp_;
        }
        if (sign_ < 0)
        {
            bits_ |= static_cast<int32_t>(1u << 31);
        }
    }

    void SingleFloatingPoint::MakeRounding(uint64_t mantissa, uint64_t rem, int32_t numOfBitsInRem, int32_t bias)
    {
        if (roundingType_ == rounding_types_constants::ROUND_TO_ZERO)
        {
            mantissa ^= (1ull << 24ull);
            mantissa &= ~1ull;
            mantissa_ = static_cast<int32_t>(mantissa);
            if (exp_ <= -150)
            {
                MakeSmallest();
            }
            return;
        }

        bool needPlusOne = false;
        if (roundingType_ == rounding_types_constants::ROUND_TO_CLOSEST_EVEN)
        {
            if (rem & (1ull << (numOfBitsInRem - 1ull)))
            {
                if (rem & ((1ull << (numOfBitsInRem - 1ull)) - 1ull))
                {
                    needPlusOne = true;
                } else
                {
                    if (mantissa & (1ull << (1ull + bias)))
                    {
                        needPlusOne = true;
                    }
                }
            }
        } else if (roundingType_ == rounding_types_constants::ROUND_TO_POSITIVE_INF)
        {
            if (rem != 0 && sign_ > 0)
            {
                needPlusOne = true;
            }
        } else
        {
            if (rem != 0 && sign_ < 0)
            {
                needPlusOne = true;
            }
        }
        mantissa ^= (1ull << 24ull);
        if (needPlusOne)
        {
            mantissa >>= 1ull + bias;
            ++mantissa;
            mantissa <<= 1ull + bias;

            if (mantissa & (1ull << 25ull))
            {
                mantissa ^= (1ull << 25ull);
                ++exp_;
            } else if (mantissa & (1ull << 24ull))
            {
                if (exp_ == 127)
                {
                    isInf_ = true;
                    return;
                }
                // if (bias)
                // {
                    mantissa ^= (1ull << 24ull);
                    ++exp_;
                // }
            }
        }
        mantissa &= ~1ull;
        mantissa_ = static_cast<int32_t>(mantissa);
        if (exp_ <= -150)
        {
            MakeSmallest();
        }
    }

    void SingleFloatingPoint::MakeBiggest()
    {
        exp_ = 127;
        mantissa_ = (1 << 24) - 1;
        mantissa_ ^= 1;
        isDenormalized_ = false;
    }

    void SingleFloatingPoint::MakeSmallest()
    {
        exp_ = -126;
        mantissa_ = 0;
        isDenormalized_ = false;
        bits_ = 0;
        isZero_ = true;
    }
}
