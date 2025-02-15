#pragma once

#include "SlowBalls.h"

namespace slowballs
{

template<Config config>
struct SlowBallsBruteforce : SlowBalls<config>
{
    using SlowBalls<config>::SlowBalls;
    using SlowBalls<config>::pos_x;
    using SlowBalls<config>::pos_y;
    
    void resolve_collision(const uint16_t i, const uint16_t j)
    {
        auto& x1 = pos_x[i];
        auto& y1 = pos_y[i];
        auto& x2 = pos_x[j];
        auto& y2 = pos_y[j];
        auto diff_x = x1 - x2;
        auto diff_y = y1 - y2;
        const real_t square_distance = diff_x * diff_x + diff_y * diff_y;
        if (square_distance < config.square_min_distance() && square_distance > 0.0f)
        {
            const real_t distance = sqrtf(square_distance);
            const real_t distance_diff = config.double_radius() - distance;
            diff_x *= distance_diff * config.response_force / distance;
            diff_y *= distance_diff * config.response_force / distance;
            x1 += diff_x;
            y1 += diff_y;
            x2 -= diff_x;
            y2 -= diff_y;
        }
    }

    void check_collisions()
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
