#pragma once

#include <utility>
#include <vector>
#include <string>

#include "half_floating_point.h"
#include "single_floating_point.h"
#include "constants.h"

namespace process_input
{
    half_floating_point::HalfFloatingPoint HalfProcessInput(int argc, char *argv[]);

    single_floating_point::SingleFloatingPoint SingleProcessInput(int argc, char *argv[]);
} //namespace process_input
