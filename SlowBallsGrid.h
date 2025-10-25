#pragma once

#include "SlowBallsBruteforce.h"

#include <array>
#include <limits>

namespace slowballs
{

struct SlowBallsGrid : SlowBallsBruteforce
{
    SlowBallsGrid(const Config& config)
    : SlowBallsBruteforce(config)
    , cell_count(config.grid_size() + config.grid_size() + 1)
    , cell_offset(config.grid_size())
    , cell_offset_copy(config.grid_size())
    , sorted_pos_x(config.amount)
    , sorted_pos_y(config.amount)
    , sorted_prev_pos_x(config.amount)
    , sorted_prev_pos_y(config.amount)
    {
    }

    void check_collisions()
    {
        std::memset(cell_count.data(), 0, sizeof(uint16_t) * config.grid_size());
        for (uint16_t i = 0; i < config.amount; ++i)
        {
            const int cell_x = pos_x[i] / config.grid_cell_size();
            const int cell_y = pos_y[i] / config.grid_cell_size();
            const int cell_pos = cell_y * config.grid_width() + cell_x;
            ++cell_count[cell_pos];
        }

        cell_offset[0] = 0;
        for (size_t i = 1; i < config.grid_size(); ++i)
        {
            cell_offset[i] = cell_offset[i - 1] + cell_count[i - 1];
        }

        cell_offset_copy = cell_offset;
        for (uint16_t i = 0; i < config.amount; ++i)
        {
            const int cell_x = pos_x[i] / config.grid_cell_size();
            const int cell_y = pos_y[i] / config.grid_cell_size();
            const int cell_pos = cell_y * config.grid_width() + cell_x;

            const auto new_index = cell_offset_copy[cell_pos]++;
            sorted_pos_x[new_index] = pos_x[i];
            sorted_pos_y[new_index] = pos_y[i];
            sorted_prev_pos_x[new_index] = prev_pos_x[i];
            sorted_prev_pos_y[new_index] = prev_pos_y[i];
        }

        pos_x.swap(sorted_pos_x);
        pos_y.swap(sorted_pos_y);
        prev_pos_x.swap(sorted_prev_pos_x);
        prev_pos_y.swap(sorted_prev_pos_y);

        static const std::array<int, 4> neigbours = {1, config.grid_width() - 1, config.grid_width(), config.grid_width() + 1};

        for (int iter = 0; iter < config.iterations; ++iter)
        {
            for (size_t i = 0; i < config.grid_size(); ++i)
            {
                for (auto j = cell_offset[i]; j < cell_offset[i] + cell_count[i]; ++j)
                {
                    for (auto k = j + 1; k < cell_offset[i] + cell_count[i]; ++k)
                    {
                        resolve_collision(j, k);
                    }

                    for (auto neigbour : neigbours)
                    {
                        for (auto k = cell_offset[i + neigbour]; k < cell_offset[i + neigbour] + cell_count[i + neigbour]; ++k)
                        {
                            resolve_collision(j, k);
                        }
                    }
                }
            }
        }
    }

    std::vector<uint16_t> cell_count;
    std::vector<uint16_t> cell_offset;
    std::vector<uint16_t> cell_offset_copy;
    std::vector<real_t> sorted_pos_x;
    std::vector<real_t> sorted_pos_y;
    std::vector<real_t> sorted_prev_pos_x;
    std::vector<real_t> sorted_prev_pos_y;
};

} // namespace slowballs
