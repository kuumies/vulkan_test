/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Pipeline class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "vk_mesh.h"
#include "vk_shader_stage.h"
#include "vk_swap_chain.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

class Device;
class RenderPass;
class Surface;

/* ---------------------------------------------------------------- *
   A pipeline
 * ---------------------------------------------------------------- */
class Pipeline
{
public:
    // Defines pipeline params
    struct Parameters
    {
        Parameters(Mesh mesh, ShaderStage shaderStage, SwapChain swapChain)
            : mesh(mesh)
            , shaderStage(shaderStage)
            , swapChain(swapChain)
        {}

        uint32_t viewportWidth;
        uint32_t viewportHeight;

        VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

        Mesh mesh;
        ShaderStage shaderStage;

        SwapChain swapChain;
    };

    // Constructs the pipeline
    Pipeline(const Device& device,
             const Surface& surface,
             const Parameters& params);

    RenderPass renderPass() const;
    VkPipeline handle() const;

    void bind(VkCommandBuffer buf);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu
