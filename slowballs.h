#pragma once

#include <array>
#include <memory>
#include <vector>

namespace slowballs {

struct Params
{
    const int width = 800;
    const int height = 600;
    const int amount = 50'000;
    const double radius = 2.0;
    const double gravity = 0.004;
    const double damping = 0.99;
    const double response_force = 0.4;
    const double penetration_ratio = 0.3;
    const int iterations = 2;
};

class SlowBalls
{
public:
    SlowBalls(const Params& params);

    inline int amount() const
    {
        return _params.amount;
    }

    inline const std::vector<double>& x() const
    {
        return _x;
    }

    inline const std::vector<double>& y() const
    {
        return _y;
    }

    void update();

private:
    void check_bounds(int i);
    void check_collisions();

    const Params _params;

    const double _double_radius;
    const double _square_radius;
    const double _square_min_distance;

    const double _min_x;
    const double _min_y;
    const double _max_x;
    const double _max_y;

    const int _grid_cell_size;
    const int _grid_width;
    const int _grid_height;

    std::vector<double> _prev_x;
    std::vector<double> _prev_y;

    std::vector<double> _x;
    std::vector<double> _y;

    std::vector<std::array<uint32_t, 5>> _grid;
    std::vector<uint8_t> _grid_size;
};

}
