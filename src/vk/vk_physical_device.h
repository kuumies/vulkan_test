/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::PhysicalDevice class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   Physical device information.

   To retrieve some of the information user must use an extension 
   'VK_KHR_get_physical_device_properties2' when creating the Vulkan 
   instance. When the extension is used then hasExtensionsFeatures and
   hasExtensionsProperties is set to true and the values are valid.
 * -------------------------------------------------------------------------- */
struct PhysicalDeviceInfo
{
    // Constructs the information of the physical device. Instance is needed
    // to retrieve info from extension functions.
    PhysicalDeviceInfo(const VkPhysicalDevice& physicalDevice,
                       const VkInstance& instance);

    // Main properties (name, type etc.)
    VkPhysicalDeviceProperties properties;

    // Features that user can enable before creating the logical device.
    // Remember that the each feature needs to be MANUALLY enabled.
    VkPhysicalDeviceFeatures features;

    // Memory properties (memory heaps, memory types)
    VkPhysicalDeviceMemoryProperties memoryProperties;

    // Queue families. Vector index is the queue family index.
    std::vector<VkQueueFamilyProperties> queueFamilies;
    // Queue presentation support status.
    std::vector<bool> queuePresentation;

    // A vector of all of the formats that Vulkan support and the properties
    // associated with the format.
    std::vector<std::pair<VkFormat, VkFormatProperties>> formats;

    // All the available extensions that the device supports.
    std::vector<VkExtensionProperties> extensions;

    // Features that requires an extension use
    bool hasExtensionsFeatures;
    VkPhysicalDeviceVariablePointerFeaturesKHR featuresVariablePointer;
    VkPhysicalDeviceMultiviewFeaturesKHX multiviewFeatures;
    VkPhysicalDevice16BitStorageFeaturesKHR features16BitStorage;
    VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR yuvSamplerFeatures;
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blendFeatures;

    // Properties (and limits) that requires an extension use
    bool hasExtensionsProperties;
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blendProperties;
    VkPhysicalDeviceDiscardRectanglePropertiesEXT  discardRectangleProperties;
    VkPhysicalDeviceIDPropertiesKHR  idProperties;
    VkPhysicalDeviceMultiviewPropertiesKHX  multiviewProperties;
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX multiviewPerView;
    VkPhysicalDevicePointClippingPropertiesKHR clippingProperties;
    VkPhysicalDevicePushDescriptorPropertiesKHR  pushDescriptorProperties;
    VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProperties;
    VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT samplerMinMaxProperties;
};

/* -------------------------------------------------------------------------- *
   A Vulkan physical device wrapper struct. 

   This will create a logical device of the physical device to access
   the hardware.

   Physical device handle is valid until Vulkan instance is destroyed.
 * -------------------------------------------------------------------------- */
class PhysicalDevice
{
public:
    // Constructs the physical device from physical device and instance 
    // handles.
    PhysicalDevice(const VkPhysicalDevice& physicalDevice,
                   const VkInstance& instance);

    // Returns true if the device is valid. Device is valid if the
    // logical device handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the info of physical device.
    PhysicalDeviceInfo info() const;

    // Returns the physical device handle.
    VkPhysicalDevice handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu
