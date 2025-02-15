#pragma once

#include "CONFIG.h"

#include <array>

namespace slowballs
{

template<Config CONFIG>
struct SlowBalls
{
    SlowBalls()
    {
        const int width = CONFIG.max_x() - CONFIG.min_x();
        const int height = CONFIG.max_y() - CONFIG.min_y();
        for (int i = 0; i < CONFIG.amount; ++i)
        {
            const int offset = CONFIG.double_radius() * i;
            pos_x[i] = CONFIG.min_x() * ((offset / width % 2 == 0) ? 1.5f : 1.0f) + (offset % width);
            pos_y[i] = CONFIG.min_y() + (offset / width) * CONFIG.double_radius();
        }

        prev_pos_x = pos_x;
        prev_pos_y = pos_y;
    }

    template<typename Self>
    void update(this Self&& self)
    {
        self.check_collisions();

        for (int i = 0; i < CONFIG.amount; ++i)
        {
            self.pos_y[i] += CONFIG.gravity;

            const auto prev_x = self.pos_x[i];
            const auto prev_y = self.pos_y[i];
            self.pos_x[i] += (self.pos_x[i] - self.prev_pos_x[i]) * CONFIG.damping;
            self.pos_y[i] += (self.pos_y[i] - self.prev_pos_y[i]) * CONFIG.damping;
            self.prev_pos_x[i] = prev_x;
            self.prev_pos_y[i] = prev_y;

            self.check_bounds(i);
        }
    }

    void render(uint32_t* data, uint32_t value, int width)
    {
        for (int i = 0; i < CONFIG.amount; ++i)
        {
            data[static_cast<int>(pos_y[i]) * width + static_cast<int>(pos_x[i])] = value;
        }
    }

    void check_bounds(int i)
    {
        pos_x[i] = std::min(std::max(pos_x[i], CONFIG.min_x()), CONFIG.max_x());
        pos_y[i] = std::min(std::max(pos_y[i], CONFIG.min_y()), CONFIG.max_y());
    }

    alignas(64) std::array<real_t, CONFIG.amount> pos_x;
    alignas(64) std::array<real_t, CONFIG.amount> pos_y;
    alignas(64) std::array<real_t, CONFIG.amount> prev_pos_x;
    alignas(64) std::array<real_t, CONFIG.amount> prev_pos_y;
};

} // namespace slowballs
