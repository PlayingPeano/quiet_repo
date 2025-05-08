#include <cstring>
#include <fstream>

#include "asm_to_bin.h"

int main(int argv, char **argc)
{
    if (argv != 3)
    {
        return 1;
    }
    if (strcmp(argc[1], "--bin") == 0)
    {
        // std::ifstream ifs(argc[2]);
        // asm_to_bin::AsmToBin binBuilder(&ifs);
        // // std::ofstream ofs(argc[2]);
        // // binBuilder.makeBinFile(&ofs);
        std::cout << "Compiling asm code is not supported" << std::endl;
    } else if (strcmp(argc[1], "--asm") == 0)
    {
        std::ifstream ifs(argc[2]);
        asm_to_bin::AsmToBin binBuilder(&ifs);
        binBuilder.benchmarkLru();
        binBuilder.statistics();
    } else
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
