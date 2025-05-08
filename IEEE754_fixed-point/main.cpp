#include <iostream>

#include "fixed_point.h"
#include "process_input.h"

int main(int argc, char *argv[])
{
    // fixed_point::FixedPoint fixedPoint{};
    process_input::ProcessInput(argc, argv).Print();
    return EXIT_SUCCESS;
}
