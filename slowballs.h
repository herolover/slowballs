#pragma once

#include "vec.h"
#include "params.h"

#include <array>
#include <memory>
#include <iostream>

namespace slowballs {

struct GridCell
{
    using index_t = uint32_t;

    std::array<index_t, 4> indexes;
    uint8_t size = 0;

    inline void add(const index_t index)
    {
        indexes[size++] = index;
    }
};

template<Params params>
class SlowBalls
{
public:
    SlowBalls()
    {
        constexpr int width = params.max_x() - params.min_x();
        constexpr int height = params.max_y() - params.min_y();
        for (int i = 0; i < params.amount; ++i)
        {
            const int offset = params.double_radius() * i;
            _pos[i].x = params.min_x() * ((offset / width % 2 == 0) ? 2 : 1) + (offset % width);
            _pos[i].y = params.min_y() + (offset / width) * params.double_radius();

        }

        _prev_pos = _pos;
    }

    void update()
    {
        check_collisions();

        for (int i = 0; i < params.amount; ++i)
        {
            _pos[i].y += params.gravity;

            const Vec prev_pos = _pos[i];
            _pos[i] += (_pos[i] - _prev_pos[i]) * params.damping;
            _prev_pos[i] = prev_pos;

            check_bounds(i);
        }
    }

    const auto& pos() const
    {
        return _pos;
    }

private:
    void check_bounds(int i)
    {
        _pos[i].clamp(params.min_x(), params.max_x(), params.min_y(), params.max_y());
    }

    void resolve_collision(int i, int j)
    {
        Vec diff = _pos[i] - _pos[j];
        const real_t square_distance = diff.square_length();
        if (square_distance < params.square_min_distance())
        {
            const real_t distance = sqrtf(square_distance);
            const real_t distance_diff = params.double_radius() - distance;
            diff *= distance_diff * params.response_force / distance;
            _pos[i] += diff;
            _pos[j] -= diff;
        }
    }

    void check_collisions()
    {
        int min_grid_pos = std::numeric_limits<int>::max();
        std::memset(&_grid[0], 0, _grid.size() * sizeof(GridCell));
        for (int i = 0; i < params.amount; ++i)
        {
            const int grid_x = _pos[i].x / params.grid_cell_size();
            const int grid_y = _pos[i].y / params.grid_cell_size();

            const int grid_pos = grid_y * params.grid_width() + grid_x;
            min_grid_pos = std::min(min_grid_pos, grid_pos);

            _grid[grid_pos].add(i);
        }

        static const std::array<int, 4> neighbours = {1, params.grid_width() - 1, params.grid_width(), params.grid_width() + 1};

        for (int iter = 0; iter < params.iterations; ++iter)
        {
            for (int i = min_grid_pos; i < _grid.size(); ++i)
            {
                for (int j = 0; j < _grid[i].size; ++j)
                {
                    for (int k = j + 1; k < _grid[i].size; ++k)
                    {
                        resolve_collision(_grid[i].indexes[j], _grid[i].indexes[k]);
                    }

                    for (auto neighbour : neighbours)
                    {
                        if (i + neighbour < _grid.size())
                        {
                            for (int k = 0; k < _grid[i + neighbour].size; ++k)
                            {
                                resolve_collision(_grid[i].indexes[j], _grid[i + neighbour].indexes[k]);
                            }
                        }
                    }
                }
            }
        }
    }

    std::array<Vec, params.amount> _pos;
    std::array<Vec, params.amount> _prev_pos;

    std::array<GridCell, params.grid_size()> _grid;
};

}
