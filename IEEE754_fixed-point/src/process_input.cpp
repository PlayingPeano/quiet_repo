#include "process_input.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <regex>
#include <vector>

namespace process_input_constants
{
    const std::regex A_DOT_B_REGEX(R"(^\d+\.\d+$)");
    const int8_t MIN_ROUND_TYPE = 0;
    const int8_t MAX_ROUND_TYPE = 3;
    const char SEPARATION_SYMBOL = '.';
    const std::string AVAILABLE_OPERATIONS = "+-*/";
    const int32_t HEX = 16;
}

namespace process_input
{
    fixed_point::FixedPoint ProcessInput(int argc, char *argv[])
    {
        if (argc != 4 && argc != 6)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        assert(argc == 4 || argc == 6);

        std::string sepArgv(argv[1]);
        std::string roundTypeArgv(argv[2]);
        std::string leftBytesArgv(argv[3]);

        if (!std::regex_match(sepArgv, process_input_constants::A_DOT_B_REGEX))
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }

        int32_t A{};
        int32_t B{};
        int8_t roundType{};
        int32_t bytes{};


        std::stringstream ss(sepArgv);
        std::string partA{};
        std::string partB{};
        if (!std::getline(ss, partA, process_input_constants::SEPARATION_SYMBOL))
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        if (!std::getline(ss, partB, process_input_constants::SEPARATION_SYMBOL))
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }

        A = std::stoi(partA);
        B = std::stoi(partB);

        roundType = static_cast<int8_t>(std::stoi(roundTypeArgv));
        if (roundType < process_input_constants::MIN_ROUND_TYPE || roundType > process_input_constants::MAX_ROUND_TYPE)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        bytes = static_cast<int32_t>(std::stoul(leftBytesArgv, nullptr, process_input_constants::HEX));

        fixed_point::FixedPoint left(A, B, bytes, roundType);

        if (argc == 4)
        {
            return left;
        }

        std::string operationArgv(argv[4]);
        std::string bytesRight(argv[5]);

        int8_t operation{};

        if (operationArgv.size() != 1 || process_input_constants::AVAILABLE_OPERATIONS.find(operationArgv) ==
            std::string::npos)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        operation = static_cast<int8_t>(operationArgv[0]);
        bytes = static_cast<int32_t>(std::stoul(bytesRight, nullptr, process_input_constants::HEX));

        fixed_point::FixedPoint right(A, B, bytes, roundType);

        return fixed_point::FixedPoint::ProcessOperation(left, right, operation);
    }
} //namespace process_input
