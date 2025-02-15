#pragma once

#include "config.h"

#include <iostream>
#include <memory>
#include <vector>

#include <immintrin.h>

namespace slowballs
{

struct SlowBalls
{
    SlowBalls(const Config& config)
    : config(config)
    , pos_x(config.amount)
    , pos_y(config.amount)
    , prev_pos_x(config.amount)
    , prev_pos_y(config.amount)
    {
        const int width = config.max_x() - config.min_x();
        const int height = config.max_y() - config.min_y();
        for (int i = 0; i < config.amount; ++i)
        {
            const int offset = config.double_radius() * i;
            pos_x[i] = config.min_x() * ((offset / width % 2 == 0) ? 1.5f : 1.0f) + (offset % width);
            pos_y[i] = config.min_y() + (offset / width) * config.double_radius();
        }

        prev_pos_x = pos_x;
        prev_pos_y = pos_y;
    }

    void update()
    {
        check_collisions();

        for (int i = 0; i < config.amount; ++i)
        {
            pos_y[i] += config.gravity;

            const auto prev_x = pos_x[i];
            const auto prev_y = pos_y[i];
            pos_x[i] += (pos_x[i] - prev_pos_x[i]) * config.damping;
            pos_y[i] += (pos_y[i] - prev_pos_y[i]) * config.damping;
            prev_pos_x[i] = prev_x;
            prev_pos_y[i] = prev_y;

            check_bounds(i);
        }
    }

    void check_bounds(int i)
    {
        pos_x[i] = std::min(std::max(pos_x[i], config.min_x()), config.max_x());
        pos_y[i] = std::min(std::max(pos_y[i], config.min_y()), config.max_y());
    }

    virtual void check_collisions() = 0;

    Config config;
    alignas(64) std::vector<real_t> pos_x;
    alignas(64) std::vector<real_t> pos_y;
    alignas(64) std::vector<real_t> prev_pos_x;
    alignas(64) std::vector<real_t> prev_pos_y;
};

} // namespace slowballs
