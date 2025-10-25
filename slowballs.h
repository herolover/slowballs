#pragma once

#include "Config.h"
#include "HugePageAllocator.h"

#include <vector>

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

    virtual ~SlowBalls()
    {
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

    virtual void check_collisions() = 0;

    void render(uint32_t* data, uint32_t value, int width)
    {
        for (int i = 0; i < config.amount; ++i)
        {
            data[static_cast<int>(pos_y[i]) * width + static_cast<int>(pos_x[i])] = value;
        }
    }

    void check_bounds(int i)
    {
        pos_x[i] = std::min(std::max(pos_x[i], config.min_x()), config.max_x());
        pos_y[i] = std::min(std::max(pos_y[i], config.min_y()), config.max_y());
    }

    Config config;
    vector_t<real_t> pos_x;
    vector_t<real_t> pos_y;
    vector_t<real_t> prev_pos_x;
    vector_t<real_t> prev_pos_y;
};

} // namespace slowballs
