#pragma once

#include "SlowBalls.h"

#include <cmath>

namespace slowballs
{

struct SlowBallsBruteforce : SlowBalls
{
    using SlowBalls::SlowBalls;

    void resolve_collision(const index_t i, const index_t j)
    {
        auto& x1 = pos_x[i];
        auto& y1 = pos_y[i];
        auto& x2 = pos_x[j];
        auto& y2 = pos_y[j];
        auto diff_x = x1 - x2;
        auto diff_y = y1 - y2;
        // vectorize square distance
        const real_t square_distance = diff_x * diff_x + diff_y * diff_y;
        if (square_distance < config.square_min_distance()) [[likely]]
        {
            const real_t distance = sqrtf(square_distance);
            const real_t ratio = (config.double_radius() - distance) * config.response_force / distance;
            diff_x *= ratio;
            diff_y *= ratio;
            x1 += diff_x;
            x2 -= diff_x;
            y1 += diff_y;
            y2 -= diff_y;
        }
    }

    void check_collisions() override
    {
        for (int i = 0; i < config.amount; ++i)
        {
            for (int j = i; j < config.amount; ++j)
            {
                resolve_collision(i, j);
            }
        }
    }
};

} // namespace slowballs
