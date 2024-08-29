#include <SDL.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"

int main(int argc, char **argv) {
#pragma clang diagnostic pop

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 800, SDL_WINDOW_VULKAN);
    if (!window) {
        std::cerr << "Error creating window: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    uint32_t extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
        std::cerr << "Error at getting amount of Vulkan extensions: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::vector<const char *> enabledExtensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, enabledExtensions.data())) {
        std::cerr << "Error at getting Vulkan extensions: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<const char *> enabledLayers = {"VK_LAYER_KHRONOS_validation"};

    VkApplicationInfo applicationInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
    createInfo.ppEnabledLayerNames = enabledLayers.data();
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);

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

    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
