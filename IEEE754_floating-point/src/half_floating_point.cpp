#include "half_floating_point.h"

#include <iostream>
#include <iomanip>

namespace half_floating_point
{
    HalfFloatingPoint::HalfFloatingPoint(int16_t bits, int8_t roundingType)
    {
        roundingType_ = roundingType;
        bits_ = bits;
        sign_ = (bits < 0) ? -1 : 1;
        auto mask = static_cast<int16_t>(~((1u << 15u) | ((1u << 10u) - 1u)));
        exp_ = ((bits & mask) >> 10) - 15;
        mantissa_ = static_cast<int32_t>(((1u << 10u) - 1u & bits) << 2);

        if (exp_ == -15 && mantissa_ != 0)
        {
            isDenormalized_ = true;
        } else if (exp_ == -15 && mantissa_ == 0)
        {
            isZero_ = true;
        } else if (exp_ == 16 && mantissa_ != 0)
        {
            isNan_ = true;
        } else if (exp_ == 16 && mantissa_ == 0)
        {
            isInf_ = true;
        }
    }

    void HalfFloatingPoint::Print()
    {
        if (isDenormalized_)
        {
            Normalize();
            std::cout << ((sign_ < 0) ? "-" : "") << "0x1." << std::hex << std::setw(3) << std::setfill('0') <<
                    mantissa_;
            std::cout << 'p' << std::dec << ((exp_ >= 0) ? "+" : "") << exp_ << " ";

            std::cout << "0x";
            std::cout << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << bits_ << std::endl;
        } else if (isZero_)
        {
            if (sign_ < 0)
            {
                std::cout << "-0x0.000p+0 0x8000" << std::endl;
            } else
            {
                std::cout << "0x0.000p+0 0x0000" << std::endl;
            }
        } else if (isNan_)
        {
            std::cout << "nan ";
            std::cout << "0x";
            std::cout << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << bits_ << std::endl;
        } else if (isInf_)
        {
            if (sign_ < 0)
            {
                std::cout << "-inf 0xFC00" << std::endl;
            } else
            {
                std::cout << "inf 0x7C00" << std::endl;
            }
        } else
        {
            std::cout << ((sign_ < 0) ? "-" : "") << "0x1." << std::hex << std::setw(3) << std::setfill('0') <<
                    mantissa_;
            std::cout << 'p' << std::dec << ((exp_ >= 0) ? "+" : "") << exp_ << " ";

            std::cout << "0x";
            std::cout << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << bits_ << std::endl;
        }
    }

    HalfFloatingPoint HalfFloatingPoint::ProcessOperation(HalfFloatingPoint &lhs, HalfFloatingPoint &rhs,
                                                          char operationType)
    {
        HalfFloatingPoint result;
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
                result.bits_ = static_cast<int16_t>(0xFE00);
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
            int32_t mask = 1 << 12;
            uint64_t mantissa = static_cast<uint64_t>(lhs.mantissa_ | mask) * static_cast<uint64_t>(
                                    rhs.mantissa_ | mask);

            result.roundingType_ = lhs.roundingType_;

            int32_t countForRem{};
            mantissa >>= 4ull;
            uint64_t rem = mantissa & ((1ull << 10ull) - 1ull);
            mantissa >>= 10ull;
            countForRem += 10;

            int32_t extraBitCnt{};
            if (mantissa & (1ull << 12ull))
            {
                ++result.exp_;
                rem |= ((mantissa & 1ull) << 10ull);
                mantissa >>= 1ull;
                ++extraBitCnt;
                ++countForRem;
            }
            if (mantissa & (1ull << 11ull))
            {
                ++result.exp_;
                rem |= ((mantissa & 1ull) << (10ull + extraBitCnt));
                mantissa >>= 1ull;
                ++extraBitCnt;
                ++countForRem;
            }
            mantissa <<= 2ull;
            if (result.exp_ > 15)
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
            } else if (result.exp_ < -14)
            {
                int32_t bias = -14 - result.exp_;
                if (bias <= 12)
                {
                    mantissa >>= 2ull;
                    rem |= (mantissa & ((1ull << bias) - 1ull)) << (10ull + extraBitCnt);
                    mantissa >>= bias;
                    mantissa <<= bias + 2ull;
                    countForRem += bias;
                    result.MakeRounding(mantissa, rem, countForRem, bias);
                } else
                {
                    if (result.roundingType_ == rounding_types_constants::ROUND_TO_POSITIVE_INF && result.sign_ > 0)
                    {
                        result.exp_ = -24;
                        result.bits_ = 0x0001;
                        result.mantissa_ = 0;
                    } else if (result.roundingType_ == rounding_types_constants::ROUND_TO_NEGATIVE_INF &&
                               result.sign_ < 0)
                    {
                        result.exp_ = -24;
                        result.bits_ = static_cast<int16_t>(0x8001);
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

    void HalfFloatingPoint::Normalize()
    {
        if (!isDenormalized_)
        {
            return;
        }
        int32_t mask = 1 << 12;
        while ((mask & mantissa_) == 0)
        {
            mantissa_ <<= 1;
            --exp_;
            if (mantissa_ == 0)
            {
                throw std::invalid_argument("invalid mantissa");
            }
        }
        ++exp_;
        mantissa_ ^= mask;
        isDenormalized_ = false;
    }

    void HalfFloatingPoint::MakeBits()
    {
        bits_ = 0;
        bits_ |= mantissa_ >> 2;
        if (exp_ >= -14)
        {
            bits_ |= (exp_ + 15) << 10;
        } else
        {
            bits_ |= (-14 + 15) << 10;
            bits_ >>= -14 - exp_;
        }
        if (sign_ < 0)
        {
            bits_ |= static_cast<int16_t>(1u << 15u);
        }
    }

    void HalfFloatingPoint::MakeRounding(uint64_t mantissa, uint64_t rem, int32_t numOfBitsInRem, int32_t bias)
    {
        mantissa ^= (1ull << 12ull);
        mantissa &= ~3ull;
        if (roundingType_ == rounding_types_constants::ROUND_TO_ZERO)
        {
            mantissa_ = static_cast<int16_t>(mantissa);

            if (exp_ <= -25)
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
                    if (mantissa & (1ull << (2ull + bias)))
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
        if (needPlusOne)
        {
            if (bias == 12ull)
            {
                mantissa = 0ull;
                exp_ += 2ull;
                mantissa_ = static_cast<int32_t>(mantissa);
                return;
            }
            if (bias == 11ull)
            {
                mantissa = 0ull;
                exp_ += 1ull;
                mantissa_ = static_cast<int32_t>(mantissa);
                return;
            }
            if (bias == 10ull)
            {
                mantissa = 0ull;
                exp_ += 1ull;
                mantissa_ = static_cast<int32_t>(mantissa);
                return;
            }

            mantissa >>= 2ull + bias;
            ++mantissa;
            mantissa <<= 2ull + bias;
            if (mantissa & (1ull << 12ull) && exp_ == 15)
            {
                isInf_ = true;
                return;
            }
            if (mantissa & (1ull << 12ull))
            {
                mantissa ^= (1ull << 12ull);
                ++exp_;
                mantissa_ = static_cast<int32_t>(mantissa);
                return;
            }
        }
        mantissa &= ~3ull;
        mantissa_ = static_cast<int32_t>(mantissa);
        if (exp_ <= -25)
        {
            MakeSmallest();
        }
    }

    void HalfFloatingPoint::MakeBiggest()
    {
        exp_ = 15;
        mantissa_ = (1 << 12) - 1;
        mantissa_ ^= 3;
        isDenormalized_ = false;
    }

    void HalfFloatingPoint::MakeSmallest()
    {
        exp_ = -14;
        mantissa_ = 0;
        isDenormalized_ = false;
        bits_ = 0;
        isZero_ = true;
    }
}
