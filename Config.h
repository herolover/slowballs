#pragma once

namespace slowballs
{

using real_t = float;

struct Config
{
    const int width;
    const int height;
    const int amount;
    const real_t radius;
    const real_t gravity;
    const real_t damping;
    const real_t response_force;
    const real_t penetration_ratio;
    const int iterations;

    constexpr auto double_radius() const
    {
        return radius * 2.0;
    }

    constexpr auto square_radius() const
    {
        return radius * radius;
    }

    constexpr auto square_min_distance() const
    {
        return double_radius() * double_radius();
    }

    constexpr auto min_x() const
    {
        return radius;
    }

    constexpr auto min_y() const
    {
        return radius;
    }

    constexpr auto max_x() const
    {
        return width - 1 - radius;
    }

    constexpr auto max_y() const
    {
        return height - 1 - radius;
    }

    constexpr int grid_cell_size() const
    {
        return double_radius();
    }

    constexpr int grid_width() const
    {
        return width / grid_cell_size();
    }

    constexpr int grid_height() const
    {
        return height / grid_cell_size();
    }

    constexpr auto grid_size() const
    {
        return grid_width() * grid_height();
    }
};

} // namespace slowballs
