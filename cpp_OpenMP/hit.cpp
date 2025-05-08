#include "hit.h"

#include <cmath>

bool hit_test(float x, float y, float z)
{
    return (x * x * x * x - 2 * x * x * x) + 4 * (y * y + z * z) <= 0;
}

const float* get_axis_range() 
{
    static const float tmp = 3.0f * sqrtf(3.0f) / 8.0f;
    static float range[6] = 
    {
        0.0f, 2.0f, 
        -tmp, tmp, 
        -tmp, tmp
    };
    return range;
}
