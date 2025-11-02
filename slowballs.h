#pragma once

#include "Config.h"
#include "HugePageAllocator.h"

#include <immintrin.h>

#include <array>
#include <vector>

namespace slowballs
{

struct SlowBalls
{
    SlowBalls(const Config& config)
    : config(config)
    , pos_x(config.amount)
    , pos_y(config.amount)
    , prev_pos_x(config.amount)
    , prev_pos_y(config.amount)
    {
        const int width = config.max_x() - config.min_x();
        const int height = config.max_y() - config.min_y();
        for (int i = 0; i < config.amount; ++i)
        {
            const int offset = config.double_radius() * i;
            pos_x[i] = config.min_x() * ((offset / width % 2 == 0) ? 1.5f : 1.0f) + (offset % width);
            pos_y[i] = height - (config.min_y() + (offset / width) * config.double_radius());
        }

        prev_pos_x = pos_x;
        prev_pos_y = pos_y;
    }

    virtual ~SlowBalls()
    {
    }

    void update()
    {
        check_collisions();

        static const auto gravity = _mm256_set1_ps(config.gravity);
        static const auto damping = _mm256_set1_ps(config.damping);

        static const auto min_x = _mm256_set1_ps(config.min_x());
        static const auto min_y = _mm256_set1_ps(config.min_y());
        static const auto max_x = _mm256_set1_ps(config.max_x());
        static const auto max_y = _mm256_set1_ps(config.max_y());

        for (int i = 0; i < config.amount; i += 8)
        {
            auto x = _mm256_loadu_ps(&pos_x[i]);
            auto y = _mm256_loadu_ps(&pos_y[i]);

            auto prev_x = _mm256_loadu_ps(&prev_pos_x[i]);
            auto prev_y = _mm256_loadu_ps(&prev_pos_y[i]);

            auto diff_x = _mm256_sub_ps(x, prev_x);
            auto diff_y = _mm256_sub_ps(_mm256_add_ps(y, gravity), prev_y);

            auto damp_diff_x = _mm256_mul_ps(diff_x, damping);
            auto damp_diff_y = _mm256_mul_ps(diff_y, damping);

            auto new_x = _mm256_add_ps(x, damp_diff_x);
            auto new_y = _mm256_add_ps(y, damp_diff_y);

            auto min_x_condition = _mm256_cmp_ps(new_x, min_x, _CMP_GT_OQ);
            auto min_y_condition = _mm256_cmp_ps(new_y, min_y, _CMP_GT_OQ);
            auto max_x_condition = _mm256_cmp_ps(new_x, max_x, _CMP_LT_OQ);
            auto max_y_condition = _mm256_cmp_ps(new_y, max_y, _CMP_LT_OQ);

            new_x = _mm256_blendv_ps(min_x, new_x, min_x_condition);
            new_y = _mm256_blendv_ps(min_y, new_y, min_y_condition);
            new_x = _mm256_blendv_ps(max_x, new_x, max_x_condition);
            new_y = _mm256_blendv_ps(max_y, new_y, max_y_condition);

            _mm256_storeu_ps(&pos_x[i], new_x);
            _mm256_storeu_ps(&pos_y[i], new_y);
            _mm256_storeu_ps(&prev_pos_x[i], x);
            _mm256_storeu_ps(&prev_pos_y[i], y);
        }
    }

    virtual void check_collisions() = 0;

    void render(uint32_t* data, uint32_t value, int width)
    {
        const auto w = _mm256_set1_epi32(width);

        std::array<uint32_t, 8> pixels{};
        for (int i = 0; i < config.amount; i += 8)
        {
            auto x = _mm256_cvttps_epi32(_mm256_loadu_ps(&pos_x[i]));
            auto y = _mm256_cvttps_epi32(_mm256_loadu_ps(&pos_y[i]));

            auto pixel_pos = _mm256_add_epi32(_mm256_mullo_epi32(y, w), x);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(pixels.data()), pixel_pos);

            for (auto pixel : pixels)
            {
                data[pixel] = value;
            }
        }
    }

    Config config;
    vector_t<real_t> pos_x;
    vector_t<real_t> pos_y;
    vector_t<real_t> prev_pos_x;
    vector_t<real_t> prev_pos_y;
};

} // namespace slowballs
