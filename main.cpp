#include "SlowBalls.h"
#include "SlowBallsBruteforce.h"
#include "SlowBallsBruteforceSimd.h"

#include <SDL3/SDL.h>

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
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

    constexpr slowballs::Config config{
        .width = width * 125 / 100,
        .height = height * 125 / 100,
        .amount = 5'000,
        .radius = 3.5,
        .gravity = 0.004,
        .damping = 0.99,
        .response_force = 0.4,
        .penetration_ratio = 0.3,
        .iterations = 2,
    };

    std::unique_ptr<slowballs::SlowBalls> balls = std::make_unique<slowballs::SlowBallsBruteforceSimd>(config);

    while (true)
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT
            || event.type == SDL_EVENT_KEY_DOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
            break;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                break;
            }
            else if (event.key.keysym.sym == SDLK_1)
            {
                balls = std::make_unique<slowballs::SlowBallsBruteforce>(config);
            }
            else if (event.key.keysym.sym == SDLK_2)
            {
                balls = std::make_unique<slowballs::SlowBallsBruteforceSimd>(config);
            }
        }

        auto t1 = std::chrono::steady_clock::now();
        balls->update();
        auto t2 = std::chrono::steady_clock::now();

        SDL_LockSurface(surface);
        SDL_memset(surface->pixels, 32, surface->h * surface->pitch);

        for (int i = 0; i < balls->pos_x.size(); ++i)
        {
            uint32_t* data = static_cast<uint32_t*>(surface->pixels);
            data[static_cast<int>(balls->pos_y[i]) * surface->w + static_cast<int>(balls->pos_x[i])] = SDL_MapRGB(surface->format, 255, 255, 255);
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
