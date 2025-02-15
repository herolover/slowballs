#pragma once

#include "config.h"

#include <array>

namespace slowballs
{

template<Config config_v>
struct SlowBalls
{
    static constexpr Config config = config_v;

    SlowBalls()
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

    template<typename Self>
    void update(this Self&& self)
    {
        self.check_collisions();

        for (int i = 0; i < config.amount; ++i)
        {
            self.pos_y[i] += config.gravity;

            const auto prev_x = self.pos_x[i];
            const auto prev_y = self.pos_y[i];
            self.pos_x[i] += (self.pos_x[i] - self.prev_pos_x[i]) * config.damping;
            self.pos_y[i] += (self.pos_y[i] - self.prev_pos_y[i]) * config.damping;
            self.prev_pos_x[i] = prev_x;
            self.prev_pos_y[i] = prev_y;

            self.check_bounds(i);
        }
    }

    template<typename Self>
    void render(this Self&& self, uint32_t* data, uint32_t value, int width)
    {
        for (int i = 0; i < self.config.amount; ++i)
        {
            data[static_cast<int>(self.pos_y[i]) * width + static_cast<int>(self.pos_x[i])] = value;
        }
    }

    void check_bounds(int i)
    {
        pos_x[i] = std::min(std::max(pos_x[i], config.min_x()), config.max_x());
        pos_y[i] = std::min(std::max(pos_y[i], config.min_y()), config.max_y());
    }

    alignas(64) std::array<real_t, config.amount> pos_x;
    alignas(64) std::array<real_t, config.amount> pos_y;
    alignas(64) std::array<real_t, config.amount> prev_pos_x;
    alignas(64) std::array<real_t, config.amount> prev_pos_y;
};

} // namespace slowballs
