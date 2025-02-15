#pragma once

#include "vec.h"
#include "params.h"

#include <algorithm>
#include <array>
#include <memory>
#include <iostream>
#include <numeric>

namespace slowballs {

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
            _pos[i].x = params.min_x() * ((offset / width % 2 == 0) ? 1.5 : 1) + (offset % width);
            _pos[i].y = params.min_y() + (offset / width) * params.double_radius();

        }

        _prev_pos = _pos;
    }

    void sort()
    {
        std::array<uint16_t, params.amount> index{};
        std::iota(index.begin(), index.end(), 0);

        std::sort(index.begin(), index.end(), [this](const auto i, const auto j) {
            const int i_x = _pos[i].x;
            const int i_y = _pos[i].y;
            const int j_x = _pos[j].x;
            const int j_y = _pos[j].y;
            return std::tie(i_x, i_y) < std::tie(j_x, j_y);
        });

        std::array<Vec, params.amount> new_pos;
        std::transform(index.begin(), index.end(), new_pos.begin(), [&](const auto i) {
            return _pos[i];
        });
        _pos = new_pos;

        std::transform(index.begin(), index.end(), new_pos.begin(), [&](const auto i) {
            return _prev_pos[i];
        });
        _prev_pos = new_pos;
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

    void check_bounds(int i)
    {
        _pos[i].clamp(params.min_x(), params.max_x(), params.min_y(), params.max_y());
    }

    void resolve_collision(Vec& a, Vec& b)
    {
        Vec diff = a - b;
        const real_t square_distance = diff.square_length();
        if (square_distance < params.square_min_distance())
        {
            const real_t distance = sqrtf(square_distance);
            const real_t distance_diff = params.double_radius() - distance;
            diff *= distance_diff * params.response_force / distance;
            a += diff;
            b -= diff;
        }
    }

    void check_collisions()
    {
        int min_grid_pos = std::numeric_limits<int>::max();
        _grid_size.fill(0);
        for (uint16_t i = 0; i < params.amount; ++i)
        {
            const int grid_x = _pos[i].x / params.grid_cell_size();
            const int grid_y = _pos[i].y / params.grid_cell_size();

            const int grid_pos = grid_y * params.grid_width() + grid_x;
            min_grid_pos = std::min(min_grid_pos, grid_pos);

            _grid[grid_pos][_grid_size[grid_pos]++] = i;
        }

        constexpr std::array<int, 4> neighbours = {1, params.grid_width() - 1, params.grid_width(), params.grid_width() + 1};

        for (int iter = 0; iter < params.iterations; ++iter)
        {
            for (int i = min_grid_pos; i < _grid.size(); ++i)
            {
                for (int j = 0; j < _grid_size[i]; ++j)
                {
                    for (int k = j + 1; k < _grid_size[i]; ++k)
                    {
                        resolve_collision(_pos[_grid[i][j]], _pos[_grid[i][k]]);
                    }

                    for (auto neighbour : neighbours)
                    {
                        if (i + neighbour < _grid.size())
                        {
                            for (int k = 0; k < _grid_size[i + neighbour]; ++k)
                            {
                                resolve_collision(_pos[_grid[i][j]], _pos[_grid[i + neighbour][k]]);
                            }
                        }
                    }
                }
            }
        }
    }

    std::array<uint8_t, params.grid_size()> _grid_size{};
    std::array<std::array<uint16_t, 4>, params.grid_size()> _grid;
    std::array<Vec, params.amount> _pos;
    std::array<Vec, params.amount> _prev_pos;
};

}
