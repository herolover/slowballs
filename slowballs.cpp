#include "slowballs.h"

#include <algorithm>
#include <cassert>

namespace slowballs {

SlowBalls::SlowBalls(const Params& params)
    : _params(params)
    , _double_radius(params.radius * 2.0)
    , _square_radius(params.radius * params.radius)
    , _square_min_distance(_double_radius * _double_radius)
    , _min_x(params.radius)
    , _min_y(params.radius)
    , _max_x(params.width - 1 - params.radius)
    , _max_y(params.height - 1 - params.radius)
    , _x(params.amount)
    , _prev_x(params.amount)
    , _y(params.amount)
    , _prev_y(params.amount)
    , _grid_cell_size(_double_radius)
    , _grid_width(params.width / _grid_cell_size)
    , _grid_height(params.height / _grid_cell_size)
    , _grid(_grid_width * _grid_height)
    , _grid_size(_grid.size())
{
    for (int i = 0; i < _params.amount; ++i)
    {
        _x[i] = rand() % _params.width;
        _y[i] = rand() % _params.height;

        check_bounds(i);

        _prev_x[i] = _x[i];
        _prev_y[i] = _y[i];
    }

    check_collisions();

    for (int i = 0; i < _params.amount; ++i)
    {
        check_bounds(i);
        _prev_x[i] = _x[i];
        _prev_y[i] = _y[i];
    }
}

void SlowBalls::check_bounds(int i)
{
    if (_x[i] < _min_x)
    {
        _x[i] = _min_x;
    }
    else if (_x[i] > _max_x)
    {
        _x[i] = _max_x;
    }

    if (_y[i] < _min_y)
    {
        _y[i] = _min_y;
    }
    else if (_y[i] > _max_y)
    {
        _y[i] = _max_y;
    }
}

void SlowBalls::update()
{
    check_collisions();

    for (int i = 0; i < _params.amount; ++i)
    {
        _y[i] += _params.gravity;

        double prev_x = _x[i];
        double prev_y = _y[i];
        _x[i] += (_x[i] - _prev_x[i]) * _params.damping;
        _y[i] += (_y[i] - _prev_y[i]) * _params.damping;
        _prev_x[i] = prev_x;
        _prev_y[i] = prev_y;

        check_bounds(i);
    }
}

void SlowBalls::resolve_collision(int i, int j)
{
    auto& x_i = _x[i];
    auto& y_i = _y[i];

    auto& x_j = _x[j];
    auto& y_j = _y[j];

    double distance_x = x_i - x_j;
    double distance_y = y_i - y_j;
    if (distance_x == 0.0 && distance_y == 0.0)
    {
        distance_x = _double_radius * 0.5;
        distance_y = _double_radius * 0.5;
    }
    const double square_distance_x = distance_x * distance_x;
    const double square_distance_y = distance_y * distance_y;
    const double square_distance = square_distance_x + square_distance_y;

    if (square_distance < _square_min_distance)
    {
        const double distance = sqrt(square_distance);
        const double distance_diff = std::min(_double_radius - distance, _double_radius * _params.penetration_ratio);
        _x[i] += distance_x / distance * distance_diff * _params.response_force;
        _y[i] += distance_y / distance * distance_diff * _params.response_force;

        _x[j] -= distance_x / distance * distance_diff * _params.response_force;
        _y[j] -= distance_y / distance * distance_diff * _params.response_force;
    }
}

void SlowBalls::check_collisions()
{
    int min_grid_pos = std::numeric_limits<int>::max();
    std::fill(_grid_size.begin(), _grid_size.end(), 0);
    for (int i = 0; i < _params.amount; ++i)
    {
        const int grid_x = _x[i] / _grid_cell_size;
        const int grid_y = _y[i] / _grid_cell_size;

        const int grid_pos = grid_y * _grid_width + grid_x;
        min_grid_pos = std::min(min_grid_pos, grid_pos);

        _grid[grid_pos][_grid_size[grid_pos]++] = i;
    }

    static const std::array<int, 4> neighbours = {1, _grid_width - 1, _grid_width, _grid_width + 1};

    for (int iter = 0; iter < _params.iterations; ++iter)
    {
        for (int i = min_grid_pos; i < _grid.size(); ++i)
        {
            for (int j = 0; j < _grid_size[i]; ++j)
            {
                for (int k = j + 1; k < _grid_size[i]; ++k)
                {
                    resolve_collision(_grid[i][j], _grid[i][k]);
                }

                for (auto neighbour : neighbours)
                {
                    if (i + neighbour < _grid.size())
                    {
                        for (int k = 0; k < _grid_size[i + neighbour]; ++k)
                        {
                            resolve_collision(_grid[i][j], _grid[i + neighbour][k]);
                        }
                    }
                }
            }
        }
    }
}

}
