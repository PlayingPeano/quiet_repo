#include "fixed_point.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>


namespace help_functions
{
    int32_t RoundAfterMultiplication(uint64_t bytes, int64_t A, int64_t B, int8_t roundType)
    {
        if (roundType == round_types::TO_ZERO)
        {
            uint64_t add{};
            if ((bytes & ((1ull << (64ull - (2 * A + B))) - 1ull)) != 0)
            {
                if (bytes & (1ull << 63ull))
                {
                    add = 1;
                }
            }
            bytes >>= 32ull - (A + B);
            bytes >>= 32ull - (A + B);
            bytes >>= B;
            bytes += add;
            bytes <<= 32ull - (A + B);
            return static_cast<int32_t>(static_cast<uint32_t>(bytes));
        }
        if (roundType == round_types::TO_CLOSEST_EVEN)
        {
            bool needPlusOne = false;
            uint64_t maskFirstUselessBit = (B == 0ull) ? 0ull : ((1ull << (64ull - (2 * A + B) - 1ull)));
            uint64_t maskAllAfterFirstUselessBit = (B == 0ull) ? 0ull : ((1ull << (64ull - (2 * A + B) - 1ull)) - 1ull);
            uint64_t maskFirstHelpfulBit = ((1ull << (64ull - (2 * A + B))));
            if (bytes & maskFirstUselessBit)
            {
                if (bytes & maskAllAfterFirstUselessBit)
                {
                    needPlusOne = true;
                }
                else
                {
                    if (bytes & maskFirstHelpfulBit)
                    {
                        needPlusOne = true;
                    }
                }
            }
            bytes >>= 32ull - (A + B);
            bytes >>= 32ull - (A + B);
            bytes >>= B;
            if (needPlusOne)
            {
                ++bytes;
            }
            bytes <<= 32ull - (A + B);
            return static_cast<int32_t>(static_cast<uint32_t>(bytes));
        }
        if (roundType == round_types::TO_POSITIVE_INF)
        {
            uint64_t add{};
            if (bytes & ((1ull << (64ull - (2 * A + B))) - 1ull))
            {
                add = 1ull;
            }
            bytes >>= 32ull - (A + B);
            bytes >>= 32ull - (A + B);
            bytes >>= B;
            bytes += add;
            bytes <<= 32ull - (A + B);
            return static_cast<int32_t>(static_cast<uint32_t>(bytes));
        }
        if (roundType == round_types::TO_NEGATIVE_INF)
        {
            bytes >>= 32ull - (A + B);
            bytes >>= 32ull - (A + B);
            bytes >>= B;
            bytes <<= 32ull - (A + B);
            return static_cast<int32_t>(static_cast<uint32_t>(bytes));
        }
        return 0;
    }
} //namespace help_functions

namespace fixed_point
{
    FixedPoint::FixedPoint(int32_t A, int32_t B, int32_t bytes, int8_t roundType) : A_(A), B_(B),
        roundType_(roundType)
    {
        auto copiedBytes = static_cast<uint32_t>(bytes);
        copiedBytes <<= 32 - (A + B);
        bytes_ = static_cast<int32_t>(copiedBytes);
        Round(); //Maybe I should Round at the beginning
    }

    void FixedPoint::Round()
    {
        if (roundType_ == round_types::TO_ZERO)
        {
            RoundToZero();
            return;
        }
        if (roundType_ == round_types::TO_CLOSEST_EVEN)
        {
            RoundToClosestEven();
            return;
        }
        if (roundType_ == round_types::TO_POSITIVE_INF)
        {
            RoundToPositiveInf();
            return;
        }
        if (roundType_ == round_types::TO_NEGATIVE_INF)
        {
            RoundToNegativeInf();
            return;
        }
        throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
    }

    void FixedPoint::Print()
    {
        Round();
        bool isNegative = false;
        uint64_t copiedBytes{};
        if (bytes_ < 0)
        {
            isNegative = true;
            copiedBytes = static_cast<uint64_t>(-bytes_);
        } else
        {
            copiedBytes = static_cast<uint64_t>(bytes_);
        }

        copiedBytes >>= (32ul - (A_ + B_));

        uint64_t tail = static_cast<int32_t>(copiedBytes);
        tail &= (1ul << B_) - 1ull;
        uint64_t fractionalPartFourDigits = ((tail * 10000ull) >> B_);
        tail = (10000ull * tail) - (((tail * 10000ull) >> B_) << B_);
        //checking if there is something after first 4 digits

        uint64_t add{};
        if (roundType_ == round_types::TO_CLOSEST_EVEN)
        {
            add = PrintRoundToClosestEven(fractionalPartFourDigits, tail, B_);
        } else if (roundType_ == round_types::TO_POSITIVE_INF)
        {
            add = PrintRoundToPositiveInf(fractionalPartFourDigits, tail, isNegative);
        } else if (roundType_ == round_types::TO_NEGATIVE_INF)
        {
            add = PrintRoundToNegativeInf(fractionalPartFourDigits, tail, isNegative);
        }

        auto integerPart = static_cast<uint32_t>(
            ((copiedBytes) & (((1ull << (A_ + B_)) - 1))) >> B_);
        copiedBytes &= (1ull << B_) - 1ull;
        auto fractionalPart = static_cast<uint32_t>((copiedBytes * 1000ull) >> B_);

        if (fractionalPart + add == 1000ull)
        {
            fractionalPart = 0ull;
            if (isNegative && copiedBytes & (1ull << (A_ + B_ - 1)))
            {
                isNegative = false;
                copiedBytes = -copiedBytes;
            }
            if (!isNegative && integerPart == (1ull << (A_ - 1ull)) - 1ull)
            {
                integerPart = (1ull << (A_ - 1ull));
                fractionalPart = 0;
                isNegative = true;
            } else if (isNegative && integerPart == (1ull << (A_ - 1ull)))
            {
                integerPart = (1ull << (A_ - 1ull)) - 1ull;
                fractionalPart = (1ull << (B_ - 1ull)) - 1ull;
                isNegative = false;
            } else
            {
                integerPart += static_cast<uint32_t>(add);
            }
        } else
        {
            fractionalPart += static_cast<uint32_t>(add);
        }

        if (isNegative && (integerPart != 0ul || fractionalPart != 0ul))
        {
            std::cout << '-';
        }
        std::cout << integerPart << "." << std::setfill('0') << std::setw(3) << fractionalPart << std::endl;
    }

    uint64_t FixedPoint::PrintRoundToClosestEven(uint64_t &fractionalPartFourDigits, uint64_t &tail,
                                                 int64_t B)
    {
        uint64_t lastValidDigit = (fractionalPartFourDigits / 10) % 10;
        if (((fractionalPartFourDigits % 10) == 5))
        {
            if ((!tail && (lastValidDigit & 1)) || tail)
            {
                return 1ull;
            }
        }
        if (((fractionalPartFourDigits % 10ull) > 5ull))
        {
            return 1ull;
        }
        return 0ull;
    }

    uint64_t FixedPoint::PrintRoundToPositiveInf(uint64_t &fractionalPartFourDigits, uint64_t &tail,
                                                 bool isNegative)
    {
        if (!tail && ((fractionalPartFourDigits % 10ull) == 0) || isNegative)
        {
            return 0ull;
        }
        return 1ull;
    }

    uint64_t FixedPoint::PrintRoundToNegativeInf(uint64_t &fractionalPartFourDigits, uint64_t &tail,
                                                 bool isNegative)
    {
        if (!tail && ((fractionalPartFourDigits % 10ull) == 0) || !isNegative)
        {
            return 0ull;
        }
        return 1ull;
    }


    FixedPoint FixedPoint::ProcessOperation(FixedPoint left, FixedPoint right, int8_t operation)
    {
        if (static_cast<char>(operation) == '*')
        {
            return Multiply(left, right);
        }
        if (static_cast<char>(operation) == '+')
        {
            return Sum(left, right);
        }
        if (static_cast<char>(operation) == '-')
        {
            return Subtract(left, right);
        }
        return FixedPoint{};
    }

    void FixedPoint::RoundToZero()
    {
        uint32_t mask = ~((1ul << (32ul - (A_ + B_))) - 1ul);
        bytes_ &= static_cast<int32_t>(mask);
    }

    void FixedPoint::RoundToClosestEven()
    {
        bool needPlusOne = false;
        uint32_t mask = 1ul << (32ul - (A_ + B_));
        if (bytes_ & mask)
        {
            for (mask >>= 1; mask != 0; mask >>= 1)
            {
                if (bytes_ & mask)
                {
                    needPlusOne = true;
                }
            }
        }
        RoundToZero();
        if (needPlusOne)
        {
            ++bytes_;
        }
    }

    void FixedPoint::RoundToPositiveInf()
    {
        if (bytes_ & ((1ul << (32ul - (A_ + B_))) - 1ul))
        {
            return;
        }
        bool needPlusOne = false;
        uint32_t mask = 1ul << (32ul - (A_ + B_) - 1);
        for (; mask != 0; mask >>= 1)
        {
            if (bytes_ & mask)
            {
                needPlusOne = true;
            }
        }
        RoundToZero();
        if (needPlusOne)
        {
            ++bytes_;
        }
    }

    void FixedPoint::RoundToNegativeInf()
    {
        if (!(bytes_ & ((1ul << (32ul - (A_ + B_))) - 1ul)))
        {
            return;
        }
        bool needMinusOne = false;
        uint32_t mask = 1ul << (32ul - (A_ + B_) - 1);
        for (; mask != 0; mask >>= 1)
        {
            if (bytes_ & mask)
            {
                needMinusOne = true;
            }
        }
        RoundToZero();
        if (needMinusOne)
        {
            --bytes_;
        }
    }


    FixedPoint FixedPoint::Multiply(FixedPoint left, FixedPoint right)
    {
        left.Round();
        right.Round();
        FixedPoint result{};
        result.A_ = left.A_;
        result.B_ = left.B_;
        result.roundType_ = left.roundType_;
        int64_t resBytes = static_cast<int64_t>(left.bytes_) * static_cast<int64_t>(right.bytes_);
        result.bytes_ = help_functions::RoundAfterMultiplication(static_cast<uint64_t>(resBytes), left.A_, left.B_,
                                                                 left.roundType_);
        result.Round();
        return result;
    }

    FixedPoint FixedPoint::Sum(FixedPoint left, FixedPoint right)
    {
        left.Round();
        right.Round();
        FixedPoint result{};
        result.A_ = left.A_;
        result.B_ = left.B_;
        result.roundType_ = left.roundType_;
        result.bytes_ = left.bytes_ + right.bytes_;
        result.Round();
        return result;
    }

    FixedPoint FixedPoint::Subtract(FixedPoint left, FixedPoint right)
    {
        left.Round();
        right.Round();
        FixedPoint result{};
        result.A_ = left.A_;
        result.B_ = left.B_;
        result.roundType_ = left.roundType_;
        auto minusRight = static_cast<uint32_t>(right.bytes_);
        minusRight = (~minusRight) + 1;
        FixedPoint tmp(right.A_, right.B_, static_cast<int32_t>(minusRight), right.roundType_);
        result.bytes_ = left.bytes_ + static_cast<int32_t>(minusRight);
        result.Round();
        return result;
    }
} //namespace fixed_point
