#pragma once

#include <ostream>

namespace slowballs
{

struct Vec
{
    using underlying_t = float;

    underlying_t x;
    underlying_t y;

    inline void operator+=(const Vec& other)
    {
        x += other.x;
        y += other.y;
    }

    inline void operator-=(const Vec& other)
    {
        x -= other.x;
        y -= other.y;
    }

    inline void operator*=(const underlying_t k)
    {
        x *= k;
        y *= k;
    }

    inline underlying_t square_length() const
    {
        return x * x + y * y;
    }

    inline void clamp(const underlying_t min_x, const underlying_t max_x, const underlying_t min_y, const underlying_t max_y)
    {
        if (x < min_x)
        {
            x = min_x;
        }
        else if (x > max_x)
        {
            x = max_x;
        }

        if (y < min_y)
        {
            y = min_y;
        }
        else if (y > max_y)
        {
            y = max_y;
        }
    }
};

inline Vec operator+(const Vec& a, const Vec& b)
{
    return Vec{a.x + b.x, a.y + b.y};
}

inline Vec operator-(const Vec& a, const Vec& b)
{
    return Vec{a.x - b.x, a.y - b.y};
}

template<class T>
inline Vec operator*(const Vec& a, const T k)
{
    return Vec{a.x * k, a.y * k};
}

inline std::ostream& operator<<(std::ostream& stream, const Vec& v)
{
    return stream << v.x << " " << v.y;
}

}
