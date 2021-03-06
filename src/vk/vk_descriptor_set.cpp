/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::DescriptorSet class
 * -------------------------------------------------------------------------- */

#include "vk_descriptor_set.h"
#include <algorithm>
#include <iostream>
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct DescriptorPool::Impl
{
    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        VkDescriptorPoolCreateInfo poolInfo;
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pNext         = NULL;
        poolInfo.flags         = 0;
        poolInfo.maxSets       = maxCount;
        poolInfo.poolSizeCount = uint32_t(typeSizes.size());
        poolInfo.pPoolSizes    = typeSizes.data();

        const VkResult result =
            vkCreateDescriptorPool(
                logicalDevice,
                &poolInfo,
                NULL,
                &pool);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": descriptor pool creation failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return false;
        }

        return true;
    }

    void destroy()
    {
        vkDestroyDescriptorPool(
            logicalDevice,
            pool,
            NULL);

        pool = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return pool != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkDescriptorPool pool = VK_NULL_HANDLE;

    // User set properties.
    std::vector<VkDescriptorPoolSize> typeSizes;
    uint32_t maxCount = 1;
};

/* -------------------------------------------------------------------------- */

DescriptorPool::DescriptorPool(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

DescriptorPool& DescriptorPool::addTypeSize(const VkDescriptorPoolSize& size)
{
    impl->typeSizes.push_back(size);
    return *this;
}

DescriptorPool& DescriptorPool::addTypeSize(
        VkDescriptorType type,
        uint32_t size)
{ return addTypeSize( { type, std::max(uint32_t(1), size) } ); }

std::vector<VkDescriptorPoolSize> DescriptorPool::typeSizes() const
{ return impl->typeSizes; }

DescriptorPool& DescriptorPool::setMaxCount(uint32_t count)
{
    impl->maxCount = std::max(uint32_t(1), count);
    return *this;
}

uint32_t DescriptorPool::maxCount() const
{ return impl->maxCount; }

bool DescriptorPool::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void DescriptorPool::destroy()
{
    if (isValid())
        impl->destroy();
}

bool DescriptorPool::isValid() const
{ return impl->pool != VK_NULL_HANDLE; }

VkDescriptorPool DescriptorPool::handle() const
{ return impl->pool; }

/* -------------------------------------------------------------------------- */

struct DescriptorSets::Impl
{
    Impl()
    {
        bufferInfo.buffer   = VK_NULL_HANDLE;
        imageInfo.imageView = VK_NULL_HANDLE;
    }

    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        if (layout == VK_NULL_HANDLE)
        {
            VkDescriptorSetLayoutCreateInfo layoutInfo;
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pNext        = NULL;
            layoutInfo.flags        = 0;
            layoutInfo.bindingCount = uint32_t(layoutBindings.size());
            layoutInfo.pBindings    = layoutBindings.data();

            VkResult result =
                vkCreateDescriptorSetLayout(
                    logicalDevice,
                    &layoutInfo,
                    NULL,
                    &layout);

            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": descriptor set layout creation failed as "
                          << vk::stringify::resultDesc(result)
                          << std::endl;
                return false;
            }

            ownLayout = true;
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &layout;

        VkResult result = vkAllocateDescriptorSets(
            logicalDevice,
            &allocInfo,
            &descriptorSets);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": descriptor set allocate failed as "
                      << vk::stringify::resultDesc(result)
                      << std::endl;

            return false;
        }

        return true;
    }

    void destroy()
    {
        if (ownLayout)
            vkDestroyDescriptorSetLayout(
                logicalDevice,
                layout,
                NULL);

        layout         = VK_NULL_HANDLE;
        descriptorSets = VK_NULL_HANDLE;
        logicalDevice  = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return descriptorSets != VK_NULL_HANDLE &&
               layout         != VK_NULL_HANDLE &&
               logicalDevice  != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;

    // Child
    VkDescriptorSet descriptorSets = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout   = VK_NULL_HANDLE;
    bool ownLayout = true;

    // Data from user
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorBufferInfo bufferInfo;
    VkDescriptorImageInfo imageInfo;
    uint32_t bindingPoint = 0;
};

/* -------------------------------------------------------------------------- */

DescriptorSets::DescriptorSets(const VkDevice& logicalDevice,
                             const VkDescriptorPool& pool)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
    impl->pool          = pool;
}

DescriptorSets& DescriptorSets::setBufferInfo(const VkDescriptorBufferInfo& info)
{
    impl->bufferInfo = info;
    return *this;
}

DescriptorSets& DescriptorSets::setImageInfo(const VkDescriptorImageInfo& info)
{
    impl->imageInfo = info;
    return *this;
}

DescriptorSets& DescriptorSets::setBindingPoint(uint32_t point)
{
    impl->bindingPoint = point;
    return *this;
}

uint32_t DescriptorSets::bindingPoint() const
{ return impl->bindingPoint; }

DescriptorSets& DescriptorSets::addLayoutBinding(
    const VkDescriptorSetLayoutBinding& layoutBinding)
{
    impl->layoutBindings.push_back(layoutBinding);
    return *this;
}

DescriptorSets& DescriptorSets::addLayoutBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        uint32_t descriptorCount,
        VkShaderStageFlags stageFlags,
        const VkSampler* immutableSamplers)
{
    return addLayoutBinding( { binding,
                               descriptorType,
                               std::max(uint32_t(1), descriptorCount),
                               stageFlags,
                               immutableSamplers } );
}

std::vector<VkDescriptorSetLayoutBinding> DescriptorSets::layoutBindings() const
{ return impl->layoutBindings; }

DescriptorSets& DescriptorSets::setLayout(VkDescriptorSetLayout layout)
{
    impl->layout = layout;
    impl->ownLayout = false;
    return *this; }

bool DescriptorSets::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void DescriptorSets::destroy()
{
    if (isValid())
        impl->destroy();
}

bool DescriptorSets::isValid() const
{
    return impl->isValid();
}

VkDescriptorSet DescriptorSets::handle() const
{ return impl->descriptorSets; }

VkDescriptorSetLayout DescriptorSets::layoutHandle() const
{ return impl->layout; }

void DescriptorSets::writeUniformBuffer(
        uint32_t binding,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize range)
{
    VkDescriptorBufferInfo info;
    info.buffer = buffer;
    info.offset = offset;
    info.range  = range;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet          = impl->descriptorSets;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo     = &info;
    writeDescriptorSet.dstBinding      = binding;

    vkUpdateDescriptorSets(
        impl->logicalDevice,
        1,
        &writeDescriptorSet,
        0, NULL);
}

void DescriptorSets::writeImage(
    uint32_t binding,
    VkSampler sampler,
    VkImageView imageView,
    VkImageLayout imageLayout)
{
    VkDescriptorImageInfo info;
    info.sampler     = sampler;
    info.imageView   = imageView;
    info.imageLayout = imageLayout;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet          = impl->descriptorSets;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo      = &info;
    writeDescriptorSet.dstBinding      = binding;

    vkUpdateDescriptorSets(
        impl->logicalDevice,
        1,
        &writeDescriptorSet,
        0, NULL);
}
} // namespace vk
} // namespace kuu
