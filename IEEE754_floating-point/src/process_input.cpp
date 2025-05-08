#include "process_input.h"
#include "constants.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <regex>
#include <vector>


namespace process_input_constants
{
    const int8_t MIN_ROUND_TYPE = 0;
    const int8_t MAX_ROUND_TYPE = 3;
    const std::string AVAILABLE_OPERATIONS = "+-*/";
    const int32_t HEX = 16;
}

namespace process_input
{
    half_floating_point::HalfFloatingPoint HalfProcessInput(int argc, char *argv[])
    {
        if (argc != 4 && argc != 6)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        assert(argc == 4 || argc == 6);

        std::string precisionArgv(argv[1]);
        std::string roundTypeArgv(argv[2]);
        std::string leftBytesArgv(argv[3]);

        if (precisionArgv[0] != 'h' && precisionArgv[0] != 'f')
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        assert(precisionArgv[0] == 'h' || precisionArgv[0] == 'f');

        int8_t roundType{};
        int16_t bits{};
        roundType = static_cast<int8_t>(std::stoi(roundTypeArgv));
        if (roundType < process_input_constants::MIN_ROUND_TYPE || roundType > process_input_constants::MAX_ROUND_TYPE)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        } //ВЫНЕСТИ В ОТДЕЛЬНУЮ
        assert(roundType >= process_input_constants::MIN_ROUND_TYPE &&
            roundType <= process_input_constants::MAX_ROUND_TYPE);


        bits = static_cast<int16_t>(std::stoul(leftBytesArgv, nullptr, process_input_constants::HEX));

        half_floating_point::HalfFloatingPoint left(bits, roundType);

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
        assert(
            operationArgv[0] == '*' || operationArgv[0] == '/' || operationArgv[0] == '+' || operationArgv[0] == '-');
        operation = static_cast<int8_t>(operationArgv[0]);

        bits = static_cast<int16_t>(std::stoul(bytesRight, nullptr, process_input_constants::HEX));

        half_floating_point::HalfFloatingPoint right(bits, roundType);

        return half_floating_point::HalfFloatingPoint::ProcessOperation(left, right, operation);
    }


    single_floating_point::SingleFloatingPoint SingleProcessInput(int argc, char *argv[])
    {
        if (argc != 4 && argc != 6)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        assert(argc == 4 || argc == 6);

        std::string precisionArgv(argv[1]);
        std::string roundTypeArgv(argv[2]);
        std::string leftBytesArgv(argv[3]);

        if (precisionArgv[0] != 'h' && precisionArgv[0] != 'f')
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        assert(precisionArgv[0] == 'h' || precisionArgv[0] == 'f');

        int8_t roundType{};
        int32_t bits{};
        roundType = static_cast<int8_t>(std::stoi(roundTypeArgv));
        if (roundType < process_input_constants::MIN_ROUND_TYPE || roundType > process_input_constants::MAX_ROUND_TYPE)
        {
            throw std::invalid_argument(error_messages::INVALID_ARGUMENTS);
        }
        assert(roundType >= process_input_constants::MIN_ROUND_TYPE &&
            roundType <= process_input_constants::MAX_ROUND_TYPE);


        bits = static_cast<int32_t>(std::stoul(leftBytesArgv, nullptr, process_input_constants::HEX));

        single_floating_point::SingleFloatingPoint left(bits, roundType);

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
        assert(
            operationArgv[0] == '*' || operationArgv[0] == '/' || operationArgv[0] == '+' || operationArgv[0] == '-');
        operation = static_cast<int8_t>(operationArgv[0]);

        bits = static_cast<int32_t>(std::stoul(bytesRight, nullptr, process_input_constants::HEX));

        single_floating_point::SingleFloatingPoint right(bits, roundType);

        return single_floating_point::SingleFloatingPoint::ProcessOperation(left, right, operation);
    }
} //namespace process_input
