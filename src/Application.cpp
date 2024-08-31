#include <SDL.h>
#include <SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"

int main(int argc, char** argv) {
#pragma clang diagnostic pop

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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
    std::vector<const char*> enabledExtensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, enabledExtensions.data())) {
        std::cerr << "Error at getting Vulkan extensions: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<const char*> enabledLayers = {
        "VK_LAYER_KHRONOS_validation",
    };

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

    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    uint32_t devicesCount = 0;
    vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr);
    if (devicesCount == 0) {
        std::cerr << "Failed to find GPUs with Vulkan support!\n";
        return 1;
    }

    std::vector<VkPhysicalDevice> physicalDevices(devicesCount);
    vkEnumeratePhysicalDevices(instance, &devicesCount, physicalDevices.data());
    if (devicesCount > 1) {
        std::cout << "Found " << devicesCount << " GPUs\n";
        for (uint32_t i = 0; i < devicesCount; ++i) {
            VkPhysicalDeviceProperties properties = {};
            vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
            std::cout << "GPU " << i << ": " << properties.deviceName << "\n";
        }
    }
    physicalDevice = physicalDevices[0];

    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkDevice device;

    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
                                             queueFamilies.data());

    uint32_t graphicsQueueIndex = 0;
    for (uint32_t i = 0; i < queueFamiliesCount; ++i) {
        VkQueueFamilyProperties queueFamily = queueFamilies[i];
        if (queueFamily.queueCount > 0) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueIndex = i;
                break;
            }
        }
    }

    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = priorities;

    VkPhysicalDeviceFeatures enabledFeatures = {};
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    VkQueue queue;
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    uint32_t swapchainWidth;
    uint32_t swapchainHeight;
    VkFormat swapchainFormat;

    VkSurfaceKHR surface;

    SDL_Vulkan_CreateSurface(window, instance, &surface);
    VkBool32 supportsPresent = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsQueueIndex, surface,
                                         &supportsPresent);
    if (!supportsPresent) {
        std::cerr << "Graphics queue does not support present!\n";
        return 1;
    }

    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> availableFormats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount,
                                         availableFormats.data());
    if (formatsCount <= 0) {
        std::cerr << "No surface formats available!\n";
    }

    VkFormat imageFormat = availableFormats[0].format;
    VkColorSpaceKHR imageColorSpace = availableFormats[0].colorSpace;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
        surfaceCapabilities.currentExtent.width = surfaceCapabilities.minImageExtent.width;
    }
    if (surfaceCapabilities.currentExtent.height == 0xFFFFFFFF) {
        surfaceCapabilities.currentExtent.height = surfaceCapabilities.minImageExtent.height;
    }
    if (surfaceCapabilities.maxImageCount == 0) {
        surfaceCapabilities.maxImageCount = 8;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = 3;
    swapchainCreateInfo.imageFormat = imageFormat;
    swapchainCreateInfo.imageColorSpace = imageColorSpace;
    swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = imageUsageFlags;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);

    swapchainFormat = imageFormat;
    swapchainWidth = surfaceCapabilities.currentExtent.width;
    swapchainHeight = surfaceCapabilities.currentExtent.height;

    uint32_t imagesCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, nullptr);

    swapchainImages.resize(imagesCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, swapchainImages.data());

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

    vkDeviceWaitIdle(device);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
