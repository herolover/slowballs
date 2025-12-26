#pragma once

#include "SlowBallsBruteforce.h"

#include <immintrin.h>

#include <array>
#include <limits>
#include <memory>

namespace slowballs
{

struct SlowBallsGrid : SlowBallsBruteforce
{
    SlowBallsGrid(const Config& config)
    : SlowBallsBruteforce(config)
    , cell_count(config.grid_size() + config.grid_width() + 1)
    , cell_offset(config.grid_size() + config.grid_width() + 1)
    , cell_offset_copy(config.grid_size())
    , sorted_pos_x(config.amount)
    , sorted_pos_y(config.amount)
    , sorted_prev_pos_x(config.amount)
    , sorted_prev_pos_y(config.amount)
    {
    }

    void check_collisions()
    {
        std::fill(cell_count.begin(), cell_count.end(), 0);
        for (index_t i = 0; i < config.amount; ++i)
        {
            const int cell_x = pos_x[i] / config.grid_cell_size();
            const int cell_y = pos_y[i] / config.grid_cell_size();
            const int cell_pos = cell_y * config.grid_width() + cell_x;
            ++cell_count[cell_pos];
        }

        cell_offset[0] = 0;
        for (size_t i = 1; i < cell_offset.size(); ++i)
        {
            cell_offset[i] = cell_offset[i - 1] + cell_count[i - 1];
        }

        cell_offset_copy = cell_offset;
        for (index_t i = 0; i < config.amount; ++i)
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

        for (int iter = 0; iter < config.iterations; ++iter)
        {
#pragma omp parallel for
            for (int i = 0; i < config.grid_size(); ++i)
            {
                for (auto j = cell_offset[i]; j < cell_offset[i + 1]; ++j)
                {
                    auto x1 = _mm256_set1_ps(pos_x[j]);
                    auto y1 = _mm256_set1_ps(pos_y[j]);
                    auto diff_x = _mm256_set1_ps(0.0f);
                    auto diff_y = _mm256_set1_ps(0.0f);
                    resolve_collision(x1, y1, diff_x, diff_y, cell_offset[i]);
                    pos_x[j] += reduce_add(diff_x);
                    pos_y[j] += reduce_add(diff_y);

                    x1 = _mm256_set1_ps(pos_x[j]);
                    y1 = _mm256_set1_ps(pos_y[j]);
                    diff_x = _mm256_set1_ps(0.0f);
                    diff_y = _mm256_set1_ps(0.0f);
                    resolve_collision(x1, y1, diff_x, diff_y, cell_offset[i + config.grid_width() - 1]);
                    pos_x[j] += reduce_add(diff_x);
                    pos_y[j] += reduce_add(diff_y);
                }
            }
        }
    }

    void resolve_collision(__m256& x1, __m256& y1, __m256& diff_x1, __m256& diff_y1, size_t k)
    {
        static const auto min_distance = _mm256_set1_ps(config.square_min_distance());
        static const auto zero = _mm256_set1_ps(0.0f);
        static const auto double_radius = _mm256_set1_ps(config.double_radius());
        static const auto response_force = _mm256_set1_ps(config.response_force);

        if (k > config.amount - 8)
        {
            k = config.amount - 8;
        }

        auto x2 = _mm256_loadu_ps(&pos_x[k]);
        auto y2 = _mm256_loadu_ps(&pos_y[k]);

        auto diff_x = _mm256_sub_ps(x1, x2);
        auto diff_y = _mm256_sub_ps(y1, y2);

        auto square_distance = _mm256_add_ps(_mm256_mul_ps(diff_x, diff_x), _mm256_mul_ps(diff_y, diff_y));
        auto less_than_min_distance_mask = _mm256_cmp_ps(square_distance, min_distance, _CMP_LT_OQ);
        auto greater_than_zore_mask = _mm256_cmp_ps(square_distance, zero, _CMP_GT_OQ);
        auto condition_mask = _mm256_and_ps(less_than_min_distance_mask, greater_than_zore_mask);

        auto distance = _mm256_sqrt_ps(square_distance);
        auto diff_ratio = _mm256_mul_ps(_mm256_div_ps(response_force, distance), _mm256_sub_ps(double_radius, distance));
        diff_x = _mm256_mul_ps(diff_x, diff_ratio);
        diff_y = _mm256_mul_ps(diff_y, diff_ratio);

        _mm256_storeu_ps(&pos_x[k], _mm256_blendv_ps(x2, _mm256_sub_ps(x2, diff_x), condition_mask));
        _mm256_storeu_ps(&pos_y[k], _mm256_blendv_ps(y2, _mm256_sub_ps(y2, diff_y), condition_mask));

        diff_x1 = _mm256_add_ps(diff_x1, _mm256_blendv_ps(zero, diff_x, condition_mask));
        diff_y1 = _mm256_add_ps(diff_y1, _mm256_blendv_ps(zero, diff_y, condition_mask));
    }

    inline static real_t reduce_add(__m256 v)
    {
        v = _mm256_hadd_ps(v, v);
        v = _mm256_hadd_ps(v, v);

        real_t res[8];
        _mm256_store_ps(res, v);

        return res[0] + res[4];
    }

    std::vector<index_t> cell_count;
    std::vector<index_t> cell_offset;
    std::vector<index_t> cell_offset_copy;
    std::vector<real_t> sorted_pos_x;
    std::vector<real_t> sorted_pos_y;
    std::vector<real_t> sorted_prev_pos_x;
    std::vector<real_t> sorted_prev_pos_y;
};

} // namespace slowballs
