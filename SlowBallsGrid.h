#pragma once

#include "SlowBallsBruteforce.h"

#include <array>
#include <limits>

namespace slowballs
{

template<Config CONFIG>
struct SlowBallsGrid : SlowBallsBruteforce<CONFIG>
{
    using SlowBallsBruteforce<CONFIG>::SlowBallsBruteforce;
    using SlowBallsBruteforce<CONFIG>::resolve_collision;
    using SlowBalls<CONFIG>::pos_x;
    using SlowBalls<CONFIG>::pos_y;

    void check_collisions()
    {
        int min_grid_pos = std::numeric_limits<int>::max();
        grid_size.fill(0);
        for (int i = 0; i < CONFIG.amount; ++i)
        {
            const int grid_x = pos_x[i] / CONFIG.grid_cell_size();
            const int grid_y = pos_y[i] / CONFIG.grid_cell_size();

            const int grid_pos = std::min(grid_y * CONFIG.grid_width() + grid_x, CONFIG.grid_size() - 1);
            min_grid_pos = std::min(min_grid_pos, grid_pos);

            grid[grid_pos][grid_size[grid_pos]++] = i;
        }

        constexpr std::array<int, 4> neighbours = {1, CONFIG.grid_width() - 1, CONFIG.grid_width(), CONFIG.grid_width() + 1};

        for (int iter = 0; iter < CONFIG.iterations; ++iter)
        {
            for (int i = min_grid_pos; i < grid.size(); ++i)
            {
                for (int j = 0; j < grid_size[i]; ++j)
                {
                    for (int k = j + 1; k < grid_size[i]; ++k)
                    {
                        resolve_collision(grid[i][j], grid[i][k]);
                    }

                    for (auto neighbour : neighbours)
                    {
                        if (i + neighbour < grid.size())
                        {
                            for (int k = 0; k < grid_size[i + neighbour]; ++k)
                            {
                                resolve_collision(grid[i][j], grid[i + neighbour][k]);
                            }
                        }
                    }
                }
            }
        }
    }

    std::array<uint16_t, CONFIG.grid_size()> grid_size;
    std::array<std::array<uint16_t, 4>, CONFIG.grid_size()> grid;
};

} // namespace slowballs
