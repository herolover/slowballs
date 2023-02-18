#include "slowballs.h"

#include <SDL3/SDL.h>

#include <chrono>
#include <format>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return -1;
    }

    constexpr int width = 800;
    constexpr int height = 600;
    if (SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer) < 0)
    {
        return -1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    slowballs::SlowBalls balls({
        .width = surface->w,
        .height = surface->h,
        .amount = 80'000,
        .radius = 2.0,
        .gravity = 0.004,
        .damping = 0.99,
        .response_force = 0.4,
        .penetration_ratio = 0.3,
        .iterations = 2,
    });
    const auto& x = balls.x();
    const auto& y = balls.y();

    while (true)
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
        {
            break;
        }

        auto t1 = std::chrono::steady_clock::now();
        balls.update();
        auto t2 = std::chrono::steady_clock::now();

        SDL_LockSurface(surface);
        SDL_memset(surface->pixels, 32, surface->h * surface->pitch);

        for (int i = 0; i < balls.amount(); ++i)
        {
            uint32_t* data = static_cast<uint32_t*>(surface->pixels);
            data[static_cast<int>(y[i]) * surface->w + static_cast<int>(x[i])] = SDL_MapRGB(surface->format, 255, 255, 255);
        }

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
        auto t3 = std::chrono::steady_clock::now();

        std::cout << std::format("physics: {} ms  \nrender: {} ms  \r\033[F", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0, std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1000.0);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}