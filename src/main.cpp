/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan test application.
 * ---------------------------------------------------------------- */

#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- *
   Globals.
 * ---------------------------------------------------------------- */

// Window
static const char* WINDOW_NAME = "Vulkan test";
static const int WINDOW_WIDTH  = 720;
static const int WINDOW_HEIGHT = 576;

// Surface
static const VkFormat SURFACE_FORMAT             = VK_FORMAT_R8G8B8A8_UNORM;
static const VkColorSpaceKHR SURFACE_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Presentation
static const VkPresentModeKHR PRESENT_MODE = VK_PRESENT_MODE_FIFO_KHR; // v-sync

// Swap chain
static const int SWAP_CHAIN_IMAGE_COUNT = 2;

int main()
{
    /* ------------------------------------------------------------ *
       Vulkan GLFW window.
     * ------------------------------------------------------------ */

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME,
        nullptr, nullptr);
    if (!window)
    {
        std::cerr << __FUNCTION__
                  << ": Failed to create Vulkan GLFW window"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Vulkan extensions.
     * ------------------------------------------------------------ */

    // Get the extensions count.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           nullptr);

    // Get the extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           extensions.data());

    // Get the extensions that GLFW window requires. On Windows
    // these are VK_KHR_surface and VK_KHR_win32_surface.
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(
        &glfwExtensionCount);

    // Check that the required extensions exits.
    for (uint32_t e = 0; e < glfwExtensionCount; ++e)
    {
        const char* glfwExtension = glfwExtensions[e];
        const auto it = std::find_if(
            extensions.begin(),
            extensions.end(),
            [glfwExtension](const VkExtensionProperties& ex)
        {
            return std::string(ex.extensionName) ==
                   std::string(glfwExtension);
        });

        if (it == extensions.end())
        {
            std::cerr << __FUNCTION__
                      << ": Vulkan implementation does not support "
                         "extension"
                      << glfwExtension
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    /* ------------------------------------------------------------ *
       Vulkan instance.
     * ------------------------------------------------------------ */

    // Let the implementation know that this application does not
    // use any famous 3rd party engine.
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = WINDOW_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "kuuEngine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    // Create the instance with extensions that GLFW requires.
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = glfwExtensionCount;
    instanceInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceInfo.enabledLayerCount       = 0;

    VkInstance instance;
    VkResult result = vkCreateInstance(&instanceInfo,
                                       nullptr,
                                       &instance);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create Vulkan instance"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Vulkan surface.
     * ------------------------------------------------------------ */

    // Create the GLFW vulkan surface
    VkSurfaceKHR surface;
    result = glfwCreateWindowSurface(instance,
                                     window,
                                     nullptr,
                                     &surface);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan surface"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Vulkan physical device.
     * ------------------------------------------------------------ */

    // Get the device count.
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance,
                               &physicalDeviceCount,
                               nullptr);

    if (physicalDeviceCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find devices with Vulkan support"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Get the devices.
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance,
                               &physicalDeviceCount,
                               physicalDevices.data());

    // Device is valid if it has VK_KHR_swapchain extension and
    // supports the global surface format and present mode.
    std::vector<const char*> physicalDeviceExtensionNames;
    physicalDeviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Search the valid physical devices.
    std::vector<VkPhysicalDevice> validPhysicalDevices;
    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
    {
#if 0
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
#endif
        // Get the surface format count.
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                             surface,
                                             &formatCount,
                                             nullptr);
        if (formatCount == 0)
            continue;

        // Get the surface formats.
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                             surface,
                                             &formatCount,
                                             formats.data());

        // Check that the device supports the surface mode.
        const auto it = std::find_if(
            formats.begin(),
            formats.end(),
            [](const VkSurfaceFormatKHR& fmt)
        {
            return fmt.format     == SURFACE_FORMAT &&
                   fmt.colorSpace == SURFACE_COLOR_SPACE;
        });
        if (it == formats.end())
            continue;

        // Get the present modes count.
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            nullptr);

        if (presentModeCount == 0)
            continue;

        // Get the present modes.
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                  surface,
                                                  &presentModeCount,
                                                  presentModes.data());

        // Check that the device supports the present mode.
        const auto it2 = std::find_if(
            presentModes.begin(),
            presentModes.end(),
            [](const VkPresentModeKHR& mode)
        { return mode == PRESENT_MODE; });
        if (it2 == presentModes.end())
            continue;

        // Get the surface capabilites.
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
                                                  surface,
                                                  &capabilities);

        // Check the window extent is supported.
        if (WINDOW_WIDTH  < capabilities.minImageExtent.width  ||
            WINDOW_WIDTH  > capabilities.maxImageExtent.width  ||
            WINDOW_HEIGHT < capabilities.minImageExtent.height ||
            WINDOW_HEIGHT > capabilities.maxImageExtent.height)
        {
            continue;
        }

        // Check the image count is supported.
        if (SWAP_CHAIN_IMAGE_COUNT < capabilities.minImageCount ||
            SWAP_CHAIN_IMAGE_COUNT > capabilities.maxImageCount)
        {
            continue;
        }

        // Get the extension count
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice,
                                             nullptr,
                                             &extensionCount,
                                             nullptr);

        // Get the extensions
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice,
                                             nullptr,
                                             &extensionCount,
                                             extensions.data());

        // Check that the device supports all the extensions.
        int found = 0;
        for (const VkExtensionProperties& ex : extensions)
        {
            for (const char* name : physicalDeviceExtensionNames)
                if (std::string(ex.extensionName) == std::string(name))
                    found++;
        }

        if (found != physicalDeviceExtensionNames.size())
            continue;

        // OK, device is valid.
        validPhysicalDevices.push_back(physicalDevice);
    }

    if (validPhysicalDevices.size() == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find valid physical device "
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Auto-select the first device.
    VkPhysicalDevice physicalDevice = validPhysicalDevices.front();

    /* ------------------------------------------------------------ *
       Vulkan queue families (graphics and present)
     * ------------------------------------------------------------ */

    // Get the queue family count.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                             &queueFamilyCount,
                                             nullptr);

    if (queueFamilyCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find Vulkan queue families"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Get the queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                             &queueFamilyCount,
                                             queueFamilies.data());

    // Find the graphics and present queue family indices.
    uint32_t graphicsQueueFamilyIndex = -1;
    uint32_t presentQueueFamilyIndex  = -1;
    for (int i = 0; i < queueFamilies.size(); ++i)
    {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
        if (queueFamily.queueCount <= 0)
            continue;

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsQueueFamilyIndex = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                                             i,
                                             surface,
                                             &presentSupport);
        if (presentSupport)
            presentQueueFamilyIndex = i;
    }

    if (graphicsQueueFamilyIndex == -1)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find Vulkan graphics queue family"
                  << std::endl;

        return EXIT_FAILURE;
    }

    if (presentQueueFamilyIndex == -1)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find Vulkan present queue family"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Vulkan logical device.
     * ------------------------------------------------------------ */

    // Create the queue create infos for graphics and present queues.
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos(2);

    queueInfos[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[0].queueFamilyIndex = graphicsQueueFamilyIndex;
    queueInfos[0].queueCount       = 1;
    queueInfos[0].pQueuePriorities = &queuePriority;

    queueInfos[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[1].queueFamilyIndex = presentQueueFamilyIndex;
    queueInfos[1].queueCount       = 1;
    queueInfos[1].pQueuePriorities = &queuePriority;

    // Logical device with graphics and present queues.
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.queueCreateInfoCount    = 2;
    deviceInfo.pEnabledFeatures        = &deviceFeatures;
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(physicalDeviceExtensionNames.size());
    deviceInfo.ppEnabledExtensionNames = physicalDeviceExtensionNames.data();
    deviceInfo.enabledLayerCount       = 0;

    // Create the logical device.
    VkDevice device;
    result = vkCreateDevice(physicalDevice,
                            &deviceInfo,
                            nullptr,
                            &device);
    if (result != VK_SUCCESS)
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan logical device"
                  << std::endl;

    /* ------------------------------------------------------------ *
       Vulkan swap chain
     * ------------------------------------------------------------ */

    const uint32_t queueFamilyIndices[] =
    {
        graphicsQueueFamilyIndex,
        presentQueueFamilyIndex
    };

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = SWAP_CHAIN_IMAGE_COUNT;
    createInfo.imageFormat      = SURFACE_FORMAT;
    createInfo.imageColorSpace  = SURFACE_COLOR_SPACE;
    createInfo.imageExtent      = VkExtent2D { WINDOW_WIDTH, WINDOW_HEIGHT };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = PRESENT_MODE;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;
    createInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSwapchainKHR swapChain;
    result = vkCreateSwapchainKHR(device,
                                  &createInfo,
                                  nullptr,
                                  &swapChain);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan swap chain"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Vulkan logical device queues
     * ------------------------------------------------------------ */

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device,
                     graphicsQueueFamilyIndex,
                     0,
                     &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(device,
                     presentQueueFamilyIndex,
                     0,
                     &presentQueue);

    /* ------------------------------------------------------------ *
       Start the "event" loop.
     * ------------------------------------------------------------ */

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    /* ------------------------------------------------------------ *
       Clean up
     * ------------------------------------------------------------ */

    glfwDestroyWindow(window);
    glfwTerminate();
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    return EXIT_SUCCESS;
}
