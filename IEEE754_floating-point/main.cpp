#include <cstdint>
#include <string>

#include "half_floating_point.h"
#include "single_floating_point.h"
#include "process_input.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || argv[1] == nullptr)
    {
        return EXIT_FAILURE;
    }
    char c{};
    c = argv[1][0];
    if (c == 'h')
    {
        try
        {
            process_input::HalfProcessInput(argc, argv).Print();
        } catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    } else if (c == 'f')
    {
        try
        {
            process_input::SingleProcessInput(argc, argv).Print();
        } catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    } else
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
