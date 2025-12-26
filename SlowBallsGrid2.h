#pragma once

#include "SlowBalls.h"

#include <immintrin.h>

#include <array>
#include <limits>

namespace slowballs
{

template<Config config>
struct SlowBallsGrid : SlowBalls<config>
{
    using Base = SlowBalls<config>;

    using Base::pos_x;
    using Base::pos_y;
    using Base::prev_pos_x;
    using Base::prev_pos_y;
    using Base::resolve_collision;

    SlowBallsGrid()
    : cell_count(config.grid_size() + config.grid_width() + 1)
    , cell_offset(config.grid_size() + config.grid_width() + 2)
    , sorted_pos_x(config.amount)
    , sorted_pos_y(config.amount)
    , sorted_prev_pos_x(config.amount)
    , sorted_prev_pos_y(config.amount)
    , cell_index(config.amount)
    {
    }

    void check_collisions()
    {
        std::fill(cell_count.begin(), cell_count.end(), 0);

        static const auto cell_size = _mm256_set1_ps(1.0 / config.grid_cell_size());
        static const auto grid_width = _mm256_set1_epi32(config.grid_width());
        for (int i = 0; i < config.amount; i += 8)
        {
            auto cell_x = _mm256_cvttps_epi32(_mm256_mul_ps(_mm256_loadu_ps(&pos_x[i]), cell_size));
            auto cell_y = _mm256_cvttps_epi32(_mm256_mul_ps(_mm256_loadu_ps(&pos_y[i]), cell_size));
            auto cell_pos = _mm256_add_epi32(_mm256_mullo_epi32(cell_y, grid_width), cell_x);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(&cell_index[i]), cell_pos);
        }
        for (index_t i = 0; i < config.amount; ++i)
        {
            ++cell_count[cell_index[i]];
        }

        cell_offset[0] = 0;
        size_t start_from_cell = 0;
        for (size_t i = 1; i < cell_offset.size(); ++i)
        {
            cell_offset[i] = cell_offset[i - 1] + cell_count[i - 1];
            if (cell_offset[i] != 0 && start_from_cell == 0)
            {
                start_from_cell = i - 1;
            }
        }

        cell_count = cell_offset;
        for (index_t i = 0; i < config.amount; ++i)
        {
            const auto new_index = cell_count[cell_index[i]]++;
            sorted_pos_x[new_index] = pos_x[i];
            sorted_pos_y[new_index] = pos_y[i];
            sorted_prev_pos_x[new_index] = prev_pos_x[i];
            sorted_prev_pos_y[new_index] = prev_pos_y[i];
        }

        pos_x.swap(sorted_pos_x);
        pos_y.swap(sorted_pos_y);
        prev_pos_x.swap(sorted_prev_pos_x);
        prev_pos_y.swap(sorted_prev_pos_y);

#pragma omp parallel
        for (int iter = 0; iter < config.iterations; ++iter)
        {
#pragma omp for nowait
            for (int i = start_from_cell; i < config.grid_size(); ++i)
            {
                auto end_j = cell_offset[i + 1];
                for (auto j = cell_offset[i]; j < end_j; ++j)
                {
                    auto x1 = pos_x[j];
                    auto y1 = pos_y[j];

                    auto end = cell_offset[i + 2];
                    for (auto k = j + 1; k < end; ++k)
                    {
                        resolve_collision(x1, y1, k);
                    }

                    auto begin = cell_offset[i + config.grid_width() - 1];
                    end = cell_offset[i + config.grid_width() + 2];
                    for (auto k = begin; k < end; ++k)
                    {
                        resolve_collision(x1, y1, k);
                    }

                    pos_x[j] = x1;
                    pos_y[j] = y1;
                }
            }
        }
    }

    std::vector<index_t> cell_count;
    std::vector<index_t> cell_offset;
    std::vector<index_t> cell_index;
    std::vector<real_t> sorted_pos_x;
    std::vector<real_t> sorted_pos_y;
    std::vector<real_t> sorted_prev_pos_x;
    std::vector<real_t> sorted_prev_pos_y;
};

} // namespace slowballs
