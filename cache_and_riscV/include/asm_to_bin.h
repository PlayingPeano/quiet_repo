#pragma once

#include <iostream>
#include <map>
#include <list>
#include <cstdint>
#include <vector>
#include <array>

namespace cache_constants
{
    const int MEM_SIZE = (1 << 18); //aka 2^(ADDR_LEN)
    const int CACHE_WAY = 32 / 8; //aka CACHE_LINE_COUNT / CACHE_SETS
    const int CACHE_TAG_LEN = 18 - 6 - 3; //aka ADDR_LEN - CACHE_OFFSET_LEN - CACHE_INDEX_LEN
    const int CACHE_OFFSET_LEN = 6; //aka log_2 (CACHE_LINE_SIZE)
    const int CACHE_SIZE = 32 * 64; //aka CACHE_LINE_COUNT * CACHE_LINE_SIZE
    const int CACHE_SETS = (1 << 3); //2^(CACHE_INDEX_LEN)
    const int ADDR_LEN = 18;
    const int CACHE_INDEX_LEN = 3;
    const int CACHE_LINE_SIZE = 64;
    const int CACHE_LINE_COUNT = 32;
} //cache_constants

namespace tables
{
    const int HEX = 16;

    const int DEC = 10;

    const int START_ADDRESS = 0x10000;

    struct Instruction_Args
    {
        std::string name;
        std::vector<uint32_t> args;
    };

    struct Instruction_constants
    {
        char type;
        std::string opcode;
    };

    inline std::map<std::string, uint32_t> REGISTERS_ENCODE = {
        {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4},
        {"t0", 5}, {"t1", 6}, {"t2", 7}, {"s0", 8}, {"fp", 8}, {"s1", 9},
        {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14},
        {"a5", 15}, {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19},
        {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23}, {"s8", 24},
        {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
        {"t5", 30}, {"t6", 31}
    };

    inline std::map<std::string, Instruction_constants> INSTRUCTIONS_TO_TYPE_N_OPCODE = {
        // RV32I
        {"lui", {'U', "0110111"}},
        {"auipc", {'U', "0010111"}},

        {"jal", {'J', "1101111"}},

        {"jalr", {'I', "1100111"}},

        {"beq", {'B', "1100011"}},
        {"bne", {'B', "1100011"}},
        {"blt", {'B', "1100011"}},
        {"bge", {'B', "1100011"}},
        {"bltu", {'B', "1100011"}},
        {"bgeu", {'B', "1100011"}},

        {"lb", {'I', "0000011"}},
        {"lh", {'I', "0000011"}},
        {"lw", {'I', "0000011"}},
        {"lbu", {'I', "0000011"}},
        {"lhu", {'I', "0000011"}},

        {"sb", {'S', "0100011"}},
        {"sh", {'S', "0100011"}},
        {"sw", {'S', "0100011"}},

        {"addi", {'I', "0010011"}},
        {"slti", {'I', "0010011"}},
        {"sltiu", {'I', "0010011"}},
        {"xori", {'I', "0010011"}},
        {"ori", {'I', "0010011"}},
        {"andi", {'I', "0010011"}},
        {"slli", {'I', "0010011"}},
        {"srli", {'I', "0010011"}},
        {"srai", {'I', "0010011"}},

        {"add", {'R', "0110011"}},
        {"sub", {'R', "0110011"}},
        {"sll", {'R', "0110011"}},
        {"slt", {'R', "0110011"}},
        {"sltu", {'R', "0110011"}},
        {"xor", {'R', "0110011"}},
        {"srl", {'R', "0110011"}},
        {"sra", {'R', "0110011"}},
        {"or", {'R', "0110011"}},
        {"and", {'R', "0110011"}},

        {"fence", {'I', "0001111"}},
        {"ecall", {'I', "1110011"}},
        {"ebreak", {'I', "1110011"}},

        // RV32M
        {"mul", {'R', "0110011"}},
        {"mulh", {'R', "0110011"}},
        {"mulhsu", {'R', "0110011"}},
        {"mulhu", {'R', "0110011"}},
        {"div", {'R', "0110011"}},
        {"divu", {'R', "0110011"}},
        {"rem", {'R', "0110011"}},
        {"remu", {'R', "0110011"}}
    };
} //tables

namespace asm_to_bin
{
    class AsmToBin
    {
    public:
        explicit AsmToBin(std::istream *_read);

        void benchmarkLru();

        void statistics() const;

    private:
        std::istream *read{};
        std::map<uint32_t, std::vector<std::string> > instructionsSequence{};
        std::map<std::string, uint32_t> labelToNextInstruction{};
        uint32_t registers[32]{};
        size_t endAddress{};
        size_t instrHitsLru{};
        size_t memHitsLru{};
        size_t instrMissesLru{};
        size_t memMissesLru{};
        size_t instrHitsPLru{};
        size_t memHitsPLru{};
        size_t instrMissesPLru{};
        size_t memMissesPLru{};
        std::list<uint32_t> lruCache[cache_constants::CACHE_LINE_SIZE]{};
        std::array<uint32_t, cache_constants::CACHE_WAY> pLruCache[cache_constants::CACHE_LINE_SIZE]{};
        std::array<bool, cache_constants::CACHE_WAY> pLruMRU[cache_constants::CACHE_LINE_SIZE]{};

        static uint32_t takeRegister(std::string &rsStr);

        static uint32_t takeImm(std::string &rsStr);

        void checkCache(uint32_t addr, bool isInstr);
    };
} //acm_to_bin
