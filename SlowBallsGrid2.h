#pragma once

#include "SlowBallsBruteforce.h"

#include <immintrin.h>

#include <array>
#include <limits>

namespace slowballs
{

struct SlowBallsGrid : SlowBallsBruteforce
{
    SlowBallsGrid(const Config& config)
    : SlowBallsBruteforce(config)
    , cell_count(config.grid_size() + config.grid_width() + 1)
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

        static const auto cell_size = _mm256_set1_ps(config.grid_cell_size());
        static const auto grid_width = _mm256_set1_epi32(config.grid_width());
        for (index_t i = 0; i < config.amount; i += 8)
        {
            auto cell_x = _mm256_cvttps_epi32(_mm256_div_ps(_mm256_loadu_ps(&pos_x[i]), cell_size));
            auto cell_y = _mm256_cvttps_epi32(_mm256_div_ps(_mm256_loadu_ps(&pos_y[i]), cell_size));
            auto cell_pos = _mm256_add_epi32(_mm256_mullo_epi32(cell_y, grid_width), cell_x);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(&cell_index[i]), cell_pos);

            for (int j = 0; j < 8; ++j)
            {
                ++cell_count[cell_index[i + j]];
            }
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

        for (int iter = 0; iter < config.iterations; ++iter)
        {
#pragma omp parallel for
            for (int i = start_from_cell; i < config.grid_size(); ++i)
            {
                auto end_j = cell_offset[i + 1];
                for (auto j = cell_offset[i]; j < end_j; ++j)
                {
                    auto end = cell_offset[i + 2];
                    for (auto k = j + 1; k < end; ++k)
                    {
                        resolve_collision(j, k);
                    }

                    auto begin = cell_offset[i + config.grid_width() - 1];
                    end = cell_offset[i + config.grid_width() + 2];
                    for (auto k = begin; k < end; ++k)
                    {
                        resolve_collision(j, k);
                    }
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
