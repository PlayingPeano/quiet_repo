#include <cstdio>
#include <string>
#include <fstream>
#include <numbers>
#include <thread>
#include <cstring>
#include <iomanip>
#include <omp.h>
#include "hit.h"
#include <random>
#include <iostream>
#include <atomic>
#include <cstdint>

struct Point{
	float x, y, z;
};

Point getPointFromTwo(const uint64_t &a, const uint64_t &b, const float* ranges) 
{
    const uint32_t* ptr = reinterpret_cast<const uint32_t*>(&a);
    const uint32_t x_bits = ptr[0];
    const uint32_t y_bits = ptr[1];
    const uint32_t z_bits = reinterpret_cast<const uint32_t*>(&b)[0];

    const auto to_float = [](uint32_t bits) 
    {
        return (bits >> 8) * 0x1.0p-24f;
    };

    return 
    {
        ranges[0] + to_float(x_bits) * (ranges[1] - ranges[0]),
        ranges[2] + to_float(y_bits) * (ranges[3] - ranges[2]),
        ranges[4] + to_float(z_bits) * (ranges[5] - ranges[4])
    };
}

struct xorshiftr128plus_state {
    uint64_t s[2];
};

uint64_t getRandom(struct xorshiftr128plus_state *state)
{ 
	uint64_t x = state->s[0];
	uint64_t const y = state->s[1];
	state->s[0] = y;
	x ^= x << 23;
	x ^= x >> 17;
	x ^= y;
	state->s[1] = x + y;
	return x;
}

//realization 1
uint32_t Monte_Carlo_Simple(uint32_t n)
{
    uint32_t hits{};
    const float *ranges = get_axis_range();
    
    double start = omp_get_wtime();
    Point point{};
    xorshiftr128plus_state state{};
    std::random_device rd;
    state.s[0] = rd();
    state.s[1] = rd();
    for (uint32_t i = 0; i < n; ++i)
    {      
        point = getPointFromTwo(getRandom(&state), getRandom(&state), ranges);  
        hits += hit_test(point.x, point.y, point.z);
    }
    double finish = omp_get_wtime();

    printf("Time (%i thread(s)): %g ms\n", 0, (finish - start) * 1000);
    return hits;
}

//realization 2
uint32_t Monte_Carlo_OMP_static(uint32_t n, uint32_t threads_num, uint32_t chunk_size)
{
    const float *ranges = get_axis_range();
    uint32_t hits{};

    omp_set_num_threads(threads_num);

    double start = omp_get_wtime();
#pragma omp parallel
    {
        uint32_t local_hits{};
        Point point{};
        xorshiftr128plus_state state;
        std::random_device rd;
        state.s[0] = rd();
        state.s[1] = rd();

#pragma omp for schedule(static, chunk_size)
        for (uint32_t i = 0; i < n; ++i)
        {
            point = getPointFromTwo(getRandom(&state), getRandom(&state), ranges);  
            local_hits += hit_test(point.x, point.y, point.z);
        }
#pragma omp atomic
        hits += local_hits;
    }
    double finish = omp_get_wtime();
    printf("Time (%i thread(s)): %g ms\n", threads_num, (finish - start) * 1000);

    return hits;
}

uint32_t Monte_Carlo_OMP_dynamic(uint32_t n, uint32_t threads_num, uint32_t chunk_size)
{
    const float *ranges = get_axis_range();
    uint32_t hits{};

    omp_set_num_threads(threads_num);

    double start = omp_get_wtime();
#pragma omp parallel
    {
        uint32_t local_hits{};
        Point point{};
        xorshiftr128plus_state state{};
        std::random_device rd;
        state.s[0] = rd();
        state.s[1] = rd();
#pragma omp for schedule(dynamic, chunk_size)
        for (uint32_t i = 0; i < n; ++i)
        {
            point = getPointFromTwo(getRandom(&state), getRandom(&state), ranges);  
            local_hits += hit_test(point.x, point.y, point.z);
        }

#pragma omp atomic
        hits += local_hits;
    }
    double finish = omp_get_wtime();
    printf("Time (%i thread(s)): %g ms\n", threads_num, (finish - start) * 1000);

    return hits;
}

//realization 3
uint32_t Monte_Carlo_OMP_manual_static(uint32_t n, uint32_t threads_num, uint32_t chunk_size)
{
    const float *ranges = get_axis_range();
    uint32_t hits{};

    omp_set_num_threads(threads_num);
    uint32_t block = (n + chunk_size - 1) / chunk_size;

    double start = omp_get_wtime();
#pragma omp parallel
    {
        uint32_t local_hits{};
        Point point{};

        uint32_t thread_id = omp_get_thread_num();
        xorshiftr128plus_state state{};
        std::random_device rd;
        state.s[0] = rd();
        state.s[1] = rd();
        for (uint32_t i = thread_id; i < block; i += threads_num)
        {
            for (uint32_t k = i * chunk_size; k < (i + 1) * chunk_size && k < n; ++k)
            {
                point = getPointFromTwo(getRandom(&state), getRandom(&state), ranges);    
                local_hits += hit_test(point.x, point.y, point.z);
            }
        }
#pragma omp atomic
        hits += local_hits;
    }
    double finish = omp_get_wtime();
    printf("Time (%i thread(s)): %g ms\n", threads_num, (finish - start) * 1000);

    return hits;
}

uint32_t Monte_Carlo_OMP_manual_dynamic(uint32_t n, uint32_t threads_num, uint32_t chunk_size)
{
    std::atomic<uint32_t> free_ind = 0;
    const float *ranges = get_axis_range();
    uint32_t hits{};

    omp_set_num_threads(threads_num);

    double start = omp_get_wtime();
#pragma omp parallel
    {
        xorshiftr128plus_state state{};
        std::random_device rd;
        state.s[0] = rd();
        state.s[1] = rd();
        uint32_t local_hits{};
        uint32_t local_ind{};
        Point point{};

        while (true)
        {
            local_ind = free_ind.fetch_add(chunk_size, std::memory_order_relaxed);
            for (uint32_t i = local_ind; i < n && i < local_ind + chunk_size; ++i)
            {
                point = getPointFromTwo(getRandom(&state), getRandom(&state), ranges);    
                local_hits += hit_test(point.x, point.y, point.z);
            }
            if (local_ind + chunk_size >= n)
            {
                break;
            }
        }
        #pragma omp atomic
        hits += local_hits;
    }
    double finish = omp_get_wtime();
    printf("Time (%i thread(s)): %g ms\n", threads_num, (finish - start) * 1000);

    return hits;
}

const float V_MAX = 9.0f * 3.0 / 8.0f;

int main(int argc, char **argv)
{
    uint32_t realization_type{};
    uint32_t points_num{};
    uint32_t chunk_size = 0;
    uint32_t threads_num = omp_get_max_threads();
    std::string kind = "dynamic";
    std::string out_file_name = "output.txt";

    for (uint32_t i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--realization") == 0)
        {
            ++i;
            realization_type = std::atol(argv[i]);
        }
        else if (std::strcmp(argv[i], "--threads") == 0)
        {
            ++i;        
            threads_num = std::atol(argv[i]);        
        }
        else if (std::strcmp(argv[i], "--input") == 0)
        {
            ++i;
            std::ifstream in(argv[i]);
            in >> points_num;
        }
        else if (std::strcmp(argv[i], "--output") == 0)
        {
            ++i;
            out_file_name = argv[i];
        }
        else if (std::strcmp(argv[i], "--chunk_size"))
        {
            ++i;
            chunk_size = std::atol(argv[i]);
        }
        else if (std::strcmp(argv[i], "--kind"))
        {
            ++i;
            kind = argv[i];
        }
    }
    
    if (chunk_size == 0)
    {
        chunk_size = 4096;
    }

    uint32_t hits{};
    if (realization_type == 1)
    {
        hits = Monte_Carlo_Simple(points_num);
    }
    else if (realization_type == 2)
    {
        if (kind == "dynamic")
        {
            hits = Monte_Carlo_OMP_dynamic(points_num, threads_num, chunk_size);
        }
        else
        {
            hits = Monte_Carlo_OMP_static(points_num, threads_num, chunk_size);
        }
    }
    else
    {
        if (kind == "dynamic")
        {
            hits = Monte_Carlo_OMP_manual_dynamic(points_num, threads_num, chunk_size);
        }
        else
        {
            hits = Monte_Carlo_OMP_manual_static(points_num, threads_num, chunk_size);
        }
        
    }

    std::ofstream out(out_file_name);
    out << std::setprecision(6) << static_cast<float>(hits) / static_cast<float>(points_num) * V_MAX << std::endl;

    return 0;
}
