#pragma once

#include "SlowBallsBruteforceSimd.h"

#include <limits>
#include <stdexcept>

namespace slowballs
{

template<Config CONFIG>
struct SlowBallsGridSimd : SlowBallsBruteforceSimd<CONFIG>
{
    using SlowBallsBruteforceSimd<CONFIG>::SlowBallsBruteforceSimd;
    using SlowBallsBruteforceSimd<CONFIG>::reduce_add;
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

            const auto grid_index = grid_size[grid_pos]++;
            grid_source_index[grid_pos][grid_index] = i;
            grid_pos_x[grid_pos][grid_index] = pos_x[i];
            grid_pos_y[grid_pos][grid_index] = pos_y[i];
        }

        constexpr std::array<int, 5> neighbours = {0, 1, CONFIG.grid_width() - 1, CONFIG.grid_width(), CONFIG.grid_width() + 1};
        for (int iter = 0; iter < CONFIG.iterations; ++iter)
        {
            for (int i = min_grid_pos; i < CONFIG.grid_size(); ++i)
            {
                for (int j = 0; j < grid_size[i]; ++j)
                {
                    auto x1 = _mm256_set1_ps(grid_pos_x[i][j]);
                    auto y1 = _mm256_set1_ps(grid_pos_y[i][j]);
                    auto diff_x1 = _mm256_set1_ps(0);
                    auto diff_y1 = _mm256_set1_ps(0);

                    for (const auto neighbour : neighbours)
                    {
                        resolve_collisions(i + neighbour, x1, y1, diff_x1, diff_y1);
                    }

                    grid_pos_x[i][j] += reduce_add(diff_x1);
                    grid_pos_y[i][j] += reduce_add(diff_y1);
                }
            }
        }

        for (int i = min_grid_pos; i < CONFIG.grid_size(); ++i)
        {
            for (int j = 0; j < grid_size[i]; ++j)
            {
                pos_x[grid_source_index[i][j]] = grid_pos_x[i][j];
                pos_y[grid_source_index[i][j]] = grid_pos_y[i][j];
            }
        }
    }

    inline static __m256 create_grid_size_mask(uint16_t size)
    {
        auto indexes = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
        auto size_vector = _mm256_set1_ps(size);

        return _mm256_cmp_ps(indexes, size_vector, _CMP_LT_OQ);
    }

    inline void resolve_collisions(uint16_t j, __m256& x1, __m256& y1, __m256& diff_x1, __m256& diff_y1)
    {
        if (j >= CONFIG.grid_size())
        {
            return;
        }

        auto min_distance = _mm256_set1_ps(CONFIG.square_min_distance());
        auto zero = _mm256_set1_ps(0.0f);
        auto double_radius = _mm256_set1_ps(CONFIG.double_radius());
        auto response_force = _mm256_set1_ps(CONFIG.response_force);

        auto x2 = _mm256_load_ps(&grid_pos_x[j][0]);
        auto y2 = _mm256_load_ps(&grid_pos_y[j][0]);
        auto diff_x = _mm256_sub_ps(x1, x2);
        auto diff_y = _mm256_sub_ps(y1, y2);

        auto square_distance = _mm256_add_ps(_mm256_mul_ps(diff_x, diff_x), _mm256_mul_ps(diff_y, diff_y));
        auto less_than_min_distance_mask = _mm256_cmp_ps(square_distance, min_distance, _CMP_LT_OQ);
        auto greater_than_zore_mask = _mm256_cmp_ps(square_distance, zero, _CMP_GT_OQ);
        auto grid_size_mask = create_grid_size_mask(grid_size[j]);
        auto condition_mask = _mm256_and_ps(grid_size_mask, _mm256_and_ps(less_than_min_distance_mask, greater_than_zore_mask));

        auto distance = _mm256_sqrt_ps(square_distance);
        auto diff_ratio = _mm256_mul_ps(_mm256_div_ps(response_force, distance), _mm256_sub_ps(double_radius, distance));
        diff_x = _mm256_mul_ps(diff_x, diff_ratio);
        diff_y = _mm256_mul_ps(diff_y, diff_ratio);

        _mm256_store_ps(&grid_pos_x[j][0], _mm256_blendv_ps(x2, _mm256_sub_ps(x2, diff_x), condition_mask));
        _mm256_store_ps(&grid_pos_y[j][0], _mm256_blendv_ps(y2, _mm256_sub_ps(y2, diff_y), condition_mask));

        diff_x1 = _mm256_add_ps(diff_x1, _mm256_blendv_ps(zero, diff_x, condition_mask));
        diff_y1 = _mm256_add_ps(diff_y1, _mm256_blendv_ps(zero, diff_y, condition_mask));
    }

    alignas(64) std::array<std::array<uint16_t, 8>, CONFIG.grid_size()> grid_source_index;
    alignas(64) std::array<uint16_t, CONFIG.grid_size()> grid_size;
    alignas(64) std::array<std::array<real_t, 8>, CONFIG.grid_size()> grid_pos_x;
    alignas(64) std::array<std::array<real_t, 8>, CONFIG.grid_size()> grid_pos_y;
};

} // namespace slowballs
