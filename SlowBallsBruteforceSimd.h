#pragma once

#include "SlowBalls.h"

#include <immintrin.h>

namespace slowballs
{

template<Config CONFIG>
struct SlowBallsBruteforceSimd : SlowBalls<CONFIG>
{
    using SlowBalls<CONFIG>::SlowBalls;
    using SlowBalls<CONFIG>::pos_x;
    using SlowBalls<CONFIG>::pos_y;

    void check_collisions()
    {
        auto min_distance = _mm256_set1_ps(CONFIG.square_min_distance());
        auto zero = _mm256_set1_ps(0.0f);
        auto double_radius = _mm256_set1_ps(CONFIG.double_radius());
        auto response_force = _mm256_set1_ps(CONFIG.response_force);

        for (uint16_t i = 0; i < CONFIG.amount; ++i)
        {
            auto x1 = _mm256_set1_ps(pos_x[i]);
            auto y1 = _mm256_set1_ps(pos_y[i]);
            auto diff_x1 = _mm256_set1_ps(0.0f);
            auto diff_y1 = _mm256_set1_ps(0.0f);
            for (uint16_t j = i / 8 * 8; j < CONFIG.amount; j += 8)
            {
                auto x2 = _mm256_loadu_ps(&pos_x[j]);
                auto y2 = _mm256_loadu_ps(&pos_y[j]);
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

                _mm256_store_ps(&pos_x[j], _mm256_blendv_ps(x2, _mm256_sub_ps(x2, diff_x), condition_mask));
                _mm256_store_ps(&pos_y[j], _mm256_blendv_ps(y2, _mm256_sub_ps(y2, diff_y), condition_mask));

                diff_x1 = _mm256_add_ps(diff_x1, _mm256_blendv_ps(zero, diff_x, condition_mask));
                diff_y1 = _mm256_add_ps(diff_y1, _mm256_blendv_ps(zero, diff_y, condition_mask));
            }

            pos_x[i] += reduce_add(diff_x1);
            pos_y[i] += reduce_add(diff_y1);
        }
    }

    inline float reduce_add(__m256 v)
    {
        v = _mm256_hadd_ps(v, v);
        v = _mm256_hadd_ps(v, v);

        float res[8];
        _mm256_store_ps(res, v);

        return res[0] + res[4];
    }
};

} // namespace slowballs
