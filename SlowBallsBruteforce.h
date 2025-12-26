#pragma once

#include "SlowBalls.h"

#include <cmath>

namespace slowballs
{

struct SlowBallsBruteforce : SlowBalls
{
    using SlowBalls::SlowBalls;

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
