#include <SDL.h>
#include <iostream>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
int main(int argc, char **argv) {
#pragma clang diagnostic pop

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
            "vulkan-test",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1280, 800,
            SDL_WINDOW_VULKAN);
    if (!window) {
        std::cerr << "Error creating window: " << SDL_GetError() << "\n";
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
