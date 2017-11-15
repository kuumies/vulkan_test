/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk:_test::Controller class
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{
    class Widget;
}

namespace vk_test
{

class MainWindow;

/* -------------------------------------------------------------------------- *
   A controller for Vulkan Test application.
 * -------------------------------------------------------------------------- */
class Controller
{
public:
    Controller();
    ~Controller();

    void runVulkanTest();

private:
    bool createInstance();
    void destroyInstance();
    
private:
    MainWindow* mainWindow = nullptr;
    vk::Widget* widget     = nullptr;

    VkInstance instance    = VK_NULL_HANDLE;
};

} // namespace vk_test
} // namespace kuu
