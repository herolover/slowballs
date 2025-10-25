#pragma once

#include <SlowBallsBruteforce.h>

#include <algorithm>
#include <numeric>
#include <vector>

namespace slowballs
{

struct SlowBallsSweepAndPrune : SlowBallsBruteforce
{
    SlowBallsSweepAndPrune(const Config& config)
    : SlowBallsBruteforce(config)
    , indexes(config.amount)
    // , z_curve(config.amount)
    , tmp(config.amount)
    {
    }

    void check_collisions() override
    {
        // for (size_t i = 0; i < config.amount; ++i)
        // {
        //     const uint16_t grid_x = pos_x[i] / config.grid_cell_size();
        //     const uint16_t grid_y = pos_y[i] / config.grid_cell_size();
        //     z_curve[i] = (interleave(grid_x) >> 1) | (interleave(grid_y));
        // }

        std::ranges::iota(indexes, 0);
        std::ranges::sort(indexes, {}, [this](uint16_t i) {
            return pos_x[i];
        });

        permute(pos_x);
        permute(pos_y);
        permute(prev_pos_x);
        permute(prev_pos_y);

        for (size_t i = 0; i < config.amount; ++i)
        {
            for (size_t j = i + 1; j < config.amount; ++j)
            {
                if (pos_x[i] + config.radius > pos_x[j] - config.radius)
                {
                    resolve_collision(i, j);
                }
                else
                    break;
            }
        }
    }

    inline uint32_t interleave(uint16_t x)
    {
        uint32_t x32 = x;
        x32 = (x32 | (x32 << 8)) & 0x00FF00FF;
        x32 = (x32 | (x32 << 4)) & 0x0F0F0F0F;
        x32 = (x32 | (x32 << 2)) & 0x33333333;
        x32 = (x32 | (x32 << 1)) & 0x55555555;
        return x32;
    }

    void permute(vector_t<real_t>& items)
    {
        for (uint16_t i = 0; i < config.amount; ++i)
        {
            tmp[i] = items[indexes[i]];
        }
        items = tmp;
    }

    std::vector<uint16_t> indexes;
    vector_t<real_t> tmp;
};

} // namespace slowballs
