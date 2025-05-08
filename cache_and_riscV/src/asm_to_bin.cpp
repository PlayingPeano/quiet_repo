#include "asm_to_bin.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>

namespace asm_to_bin
{
    AsmToBin::AsmToBin(std::istream *_read) : read(_read)
    {
        std::vector<std::string> tokens;

        std::stringstream buffer;
        buffer << read->rdbuf();
        std::string buffer_str = buffer.str();
        char *cstr = new char [buffer_str.size() + 1];
        std::strcpy(cstr, buffer_str.c_str());
        char *token;
        token = std::strtok(cstr, " ,\n");
        while (token != nullptr)
        {
            tokens.emplace_back(token);
            token = std::strtok(nullptr, " ,\n");
        }

        size_t k = tables::START_ADDRESS - 4;
        for (const auto &el: tokens)
        {
            if (tables::INSTRUCTIONS_TO_TYPE_N_OPCODE.find(el) != tables::INSTRUCTIONS_TO_TYPE_N_OPCODE.end())
            {
                k += 4;
                instructionsSequence[k].push_back(el);
            } else if (el == "ecall" || el == "ebreak")
            {
                k += 4;
                instructionsSequence[k].emplace_back("addi");
                instructionsSequence[k].emplace_back("zero");
                instructionsSequence[k].emplace_back("zero");
                instructionsSequence[k].emplace_back("0");
            } else if (el.front() == '.')
            {
                labelToNextInstruction[el] = k + 4;
            } else
            {
                instructionsSequence[k].push_back(el);
            }
        }
        endAddress = k;

        for (size_t i = 0; i < cache_constants::CACHE_LINE_SIZE; ++i)
        {
            for (size_t j = 0; j < cache_constants::CACHE_WAY; ++j)
            {
                pLruCache[i][j] = -123424;
            }
        }
    }

    void AsmToBin::benchmarkLru()
    {
        long long PC = tables::START_ADDRESS;
        while (true)
        {
            if (PC > endAddress || PC < tables::START_ADDRESS)
            {
                break;
            }
            const std::string &instr = instructionsSequence[PC].front();
            checkCache(PC, true);

            if (instr == "lui")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t imm = takeImm(instructionsSequence[PC][2]);
                registers[rd] = imm << 12;
                PC += 4;
            } else if (instr == "auipc")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t imm = takeImm(instructionsSequence[PC][2]);
                registers[rd] = PC + (imm << 12);
                PC += 4;
            } else if (instr == "jal")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][2]);
                registers[rd] = PC + 4;
                PC += imm;
            } else if (instr == "jalr")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                uint32_t t = PC + 4;
                PC = (registers[rs1] + imm) & ~1u;
                registers[rd] = t;
            } else if (instr == "beq")
            {
                uint32_t rs1 = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                if (registers[rs1] == registers[rs2])
                {
                    PC += imm;
                } else
                {
                    PC += 4;
                }
            } else if (instr == "bne")
            {
                uint32_t rs1 = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                if (registers[rs1] != registers[rs2])
                {
                    PC += imm;
                } else
                {
                    PC += 4;
                }
            } else if (instr == "blt")
            {
                uint32_t rs1 = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                if ((int32_t) registers[rs1] < (int32_t) registers[rs2])
                {
                    PC += imm;
                } else
                {
                    PC += 4;
                }
            } else if (instr == "bge")
            {
                uint32_t rs1 = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                if ((int32_t) registers[rs1] >= (int32_t) registers[rs2])
                {
                    PC += imm;
                } else
                {
                    PC += 4;
                }
            } else if (instr == "bltu")
            {
                uint32_t rs1 = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                if (registers[rs1] < registers[rs2])
                {
                    PC += imm;
                } else
                {
                    PC += 4;
                }
            } else if (instr == "bgeu")
            {
                uint32_t rs1 = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                if (registers[rs1] >= registers[rs2])
                {
                    PC += imm;
                } else
                {
                    PC += 4;
                }
            } else if (instr == "lb" || instr == "lh" || instr == "lw" || instr == "lbu" || instr == "lhu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][2]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][3]);
                uint32_t addr = registers[rs1] + imm;
                checkCache(addr, false);
                registers[rd] = 0;
                PC += 4;
            } else if (instr == "sb" || instr == "sh" || instr == "sw")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][2]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][3]);
                uint32_t addr = registers[rs1] + imm;
                checkCache(addr, false);
                PC += 4;
            } else if (instr == "addi")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                registers[rd] = (int32_t) registers[rs1] + imm;
                PC += 4;
            } else if (instr == "slti")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                registers[rd] = ((int32_t) registers[rs1] < imm) ? 1 : 0;
                PC += 4;
            } else if (instr == "sltiu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t imm = takeImm(instructionsSequence[PC][3]);
                registers[rd] = (registers[rs1] < imm) ? 1 : 0;
                PC += 4;
            } else if (instr == "xori")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                registers[rd] = registers[rs1] ^ imm;
                PC += 4;
            } else if (instr == "ori")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                registers[rd] = registers[rs1] | imm;
                PC += 4;
            } else if (instr == "andi")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                int32_t imm = (int32_t) takeImm(instructionsSequence[PC][3]);
                registers[rd] = registers[rs1] & imm;
                PC += 4;
            } else if (instr == "slli")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t imm = takeImm(instructionsSequence[PC][3]) & 0x1F;
                registers[rd] = registers[rs1] << imm;
                PC += 4;
            } else if (instr == "srli")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t imm = takeImm(instructionsSequence[PC][3]) & 0x1F;
                registers[rd] = (uint32_t) registers[rs1] >> imm;
                PC += 4;
            } else if (instr == "srai")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t shamt = takeImm(instructionsSequence[PC][3]) & 0x1F;
                registers[rd] = (int32_t) registers[rs1] >> (int32_t) shamt;
                PC += 4;
            } else if (instr == "add")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = (int32_t) registers[rs1] + (int32_t) registers[rs2];
                PC += 4;
            } else if (instr == "sub")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = (int32_t) registers[rs1] - (int32_t) registers[rs2];
                PC += 4;
            } else if (instr == "sll")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]) & 0x1F;
                registers[rd] = registers[rs1] << registers[rs2];
                PC += 4;
            } else if (instr == "slt")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = ((int32_t) registers[rs1] < (int32_t) registers[rs2]) ? 1 : 0;
                PC += 4;
            } else if (instr == "sltu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = (registers[rs1] < registers[rs2]) ? 1 : 0;
                PC += 4;
            } else if (instr == "xor")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = registers[rs1] ^ registers[rs2];
                PC += 4;
            } else if (instr == "srl")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]) & 0x1F;
                registers[rd] = (uint32_t) registers[rs1] >> registers[rs2];
                PC += 4;
            } else if (instr == "sra")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]) & 0x1F;
                registers[rd] = (int32_t) registers[rs1] >> (int32_t) registers[rs2];
                PC += 4;
            } else if (instr == "or")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = registers[rs1] | registers[rs2];
                PC += 4;
            } else if (instr == "and")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                registers[rd] = registers[rs1] & registers[rs2];
                PC += 4;
            } else if (instr == "fence")
            {
                PC += 4;
            } else if (instr == "mul")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1 = takeRegister(instructionsSequence[PC][2]);
                uint32_t rs2 = takeRegister(instructionsSequence[PC][3]);
                int64_t a = (int32_t) registers[rs1];
                int64_t b = (int32_t) registers[rs2];
                int64_t res = a * b;
                registers[rd] = (uint32_t) (res & 0xFFFFFFFF);
                PC += 4;
            } else if (instr == "mulh")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int64_t a = (int32_t) registers[takeRegister(instructionsSequence[PC][2])];
                int64_t b = (int32_t) registers[takeRegister(instructionsSequence[PC][3])];
                int64_t res = a * b;
                registers[rd] = (uint32_t) ((res >> 32) & 0xFFFFFFFF);
                PC += 4;
            } else if (instr == "mulhsu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int64_t a = (int32_t) registers[takeRegister(instructionsSequence[PC][2])];
                uint64_t b = (uint32_t) registers[takeRegister(instructionsSequence[PC][3])];
                int64_t res = (int64_t) a * (uint64_t) b;
                uint64_t upper = (uint64_t) (res >> 32);
                registers[rd] = (uint32_t) (upper & 0xFFFFFFFF);
                PC += 4;
            } else if (instr == "mulhu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint64_t a = (uint32_t) registers[takeRegister(instructionsSequence[PC][2])];
                uint64_t b = (uint32_t) registers[takeRegister(instructionsSequence[PC][3])];
                uint64_t res = (uint64_t) a * (uint64_t) b;
                uint64_t upper = (uint64_t) (res >> 32ull);
                registers[rd] = (uint32_t) (upper & 0xFFFFFFFF);
                PC += 4;
            } else if (instr == "div")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int32_t rs1v = (int32_t) registers[takeRegister(instructionsSequence[PC][2])];
                int32_t rs2v = (int32_t) registers[takeRegister(instructionsSequence[PC][3])];
                if (rs2v == 0)
                {
                    registers[rd] = 0xFFFFFFFF;
                } else
                {
                    registers[rd] = (int32_t) (rs1v / rs2v);
                }
                PC += 4;
            } else if (instr == "divu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1v = registers[takeRegister(instructionsSequence[PC][2])];
                uint32_t rs2v = registers[takeRegister(instructionsSequence[PC][3])];
                if (rs2v == 0)
                {
                    registers[rd] = 0xFFFFFFFF;
                } else
                {
                    registers[rd] = rs1v / rs2v;
                }
                PC += 4;
            } else if (instr == "rem")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                int32_t rs1v = (int32_t) registers[takeRegister(instructionsSequence[PC][2])];
                int32_t rs2v = (int32_t) registers[takeRegister(instructionsSequence[PC][3])];
                if (rs2v == 0)
                {
                    registers[rd] = rs1v;
                } else
                {
                    registers[rd] = (int32_t) (rs1v % rs2v);
                }
                PC += 4;
            } else if (instr == "remu")
            {
                uint32_t rd = takeRegister(instructionsSequence[PC][1]);
                uint32_t rs1v = registers[takeRegister(instructionsSequence[PC][2])];
                uint32_t rs2v = registers[takeRegister(instructionsSequence[PC][3])];
                if (rs2v == 0)
                {
                    registers[rd] = rs1v;
                } else
                {
                    registers[rd] = rs1v % rs2v;
                }
                PC += 4;
            } else
            {
                break;
            }
            std::string zero = "zero";
            registers[takeRegister(zero)] = 0;
        }
    }

    uint32_t AsmToBin::takeRegister(std::string &rsStr)
    {
        return tables::REGISTERS_ENCODE[rsStr];
    }

    uint32_t AsmToBin::takeImm(std::string &rsStr)
    {
        if (rsStr.starts_with("0x"))
        {
            return std::stoi(rsStr.substr(2, rsStr.size() - 2), nullptr, tables::HEX);
        } else
        {
            return std::stoi(rsStr, nullptr, tables::DEC);
        }
    }

    void AsmToBin::checkCache(uint32_t addr, bool isInstr)
    {
        addr >>= cache_constants::CACHE_OFFSET_LEN;
        uint32_t ind = addr & ((1 << cache_constants::CACHE_INDEX_LEN) - 1);
        addr >>= cache_constants::CACHE_INDEX_LEN;
        uint32_t tag = addr & ((1 << cache_constants::CACHE_TAG_LEN) - 1); {
            //LRU
            auto el = std::find(lruCache[ind].begin(), lruCache[ind].end(), tag);
            if (el != lruCache[ind].end())
            {
                lruCache[ind].splice(lruCache[ind].begin(), lruCache[ind], el);
                if (isInstr)
                {
                    ++instrHitsLru;
                } else
                {
                    ++memHitsLru;
                }
            } else
            {
                lruCache[ind].push_front(tag);
                if (lruCache[ind].size() > 4)
                {
                    lruCache[ind].pop_back();
                }
                if (isInstr)
                {
                    ++instrMissesLru;
                } else
                {
                    ++memMissesLru;
                }
            }
        } {
            //PLRUm
            size_t i{};
            auto el = std::find(pLruCache[ind].begin(), pLruCache[ind].end(), tag);
            if (el != pLruCache[ind].end())
            {
                i = el - pLruCache[ind].begin();
                pLruMRU[ind][i] = true;
                if (isInstr)
                {
                    ++instrHitsPLru;
                } else
                {
                    ++memHitsPLru;
                }
            } else
            {
                for (; i < cache_constants::CACHE_WAY; ++i)
                {
                    if (!pLruMRU[ind][i])
                    {
                        pLruMRU[ind][i] = true;
                        pLruCache[ind][i] = tag;
                        break;
                    }
                }
                if (isInstr)
                {
                    ++instrMissesPLru;
                } else
                {
                    ++memMissesPLru;
                }
            }

            if (std::find(pLruMRU[ind].begin(), pLruMRU[ind].end(), false) == pLruMRU[ind].end())
            {
                for (auto &u: pLruMRU[ind])
                {
                    u = false;
                }
                pLruMRU[ind][el - pLruCache[ind].begin()] = true;
            }
        }
    }

    void AsmToBin::statistics() const
    {
        auto total = static_cast<double>(instrHitsLru + memHitsLru + instrMissesLru + memMissesLru);
        double hitRate = static_cast<double>(instrHitsLru + memHitsLru) / total * 100;
        double hitInstRate = static_cast<double>(instrHitsLru) / static_cast<double>(instrHitsLru + instrMissesLru) *
                             100;

        double hitDatRate = static_cast<double>(memHitsLru) / static_cast<double>(memHitsLru + memMissesLru) *
                            100;

        auto p_total = static_cast<double>(instrHitsPLru + memHitsPLru + instrMissesPLru + memMissesPLru);
        double p_hitRate = static_cast<double>(instrHitsPLru + memHitsPLru) / p_total * 100;
        double p_hitInstRate = static_cast<double>(instrHitsPLru) / static_cast<double>(instrHitsPLru + instrMissesPLru)
                               *
                               100;

        double p_hitDatRate = static_cast<double>(memHitsPLru) / static_cast<double>(memHitsPLru + memMissesPLru) *
                              100;


        printf("replacement\thit rate\thit rate (inst)\thit rate (data)\n"
               "        LRU\t%3.5f%%\t%3.5f%%\t%3.5f%%\n"
               "       pLRU\t%3.5f%%\t%3.5f%%\t%3.5f%%\n", hitRate, hitInstRate, hitDatRate, p_hitRate, p_hitInstRate,
               p_hitDatRate);
        // printf("replacement\thit rate\thit rate (inst)\thit rate (data)\n"
        //        "        LRU\t%3.5f%%\t%3.5f%%\t%3.5f%%\n"
        //        "       pLRU\tunsupported\tunsupported\tunsupported\n", hitRate, hitInstRate, hitDatRate);
    }
} //asm_to_bin
