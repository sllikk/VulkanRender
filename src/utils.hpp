#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_enum_string_helper.h>



#include <array>
#include <deque>
#include <queue>
#include <stack>
#include <vector>

#include <memory>
#include <functional>


constexpr uint32_t DOUBLE_BUFFERING = 2;
constexpr uint32_t TRIPLE_BUFFERING = 3;


struct DeletionQueue {

 std::deque<std::function<void()>> queue{};

  void flush()  {

    for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
        (*it)();
    }
    queue.clear();
  }

  void add_to_queue(std::function<void()>&& function) {
      queue.push_back(function);
  }


};

inline void THROW_IF_ERROR(const VkResult& result)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(string_VkResult(result));
    }
}

struct Vertex {

  glm::vec3 position;
  glm::vec4 color;

};

//
// This buffer uses for everything mem allocations
//

struct GpuBuffer {

  VkBuffer buffer = nullptr;
  VmaAllocation allocation = nullptr;

};


struct RenderItem {

  uint32_t vertices_count = 0;
  uint32_t vertices_start = 0;

  std::unique_ptr<GpuBuffer> vertex_buffer;
  std::unique_ptr<GpuBuffer> index_buffer;


};


namespace VkUtils
{
    VkCommandPoolCreateInfo command_pool_create_info (uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo command_pool_create_info = {};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = flags;
        command_pool_create_info.queueFamilyIndex = queueFamilyIndex;
        command_pool_create_info.pNext = nullptr;
        return command_pool_create_info;
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool cmd_pool, VkCommandBufferLevel level, const uint32_t count)
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = cmd_pool;
        command_buffer_allocate_info.commandBufferCount = count;
        command_buffer_allocate_info.level = level;
        command_buffer_allocate_info.pNext = nullptr;

        return command_buffer_allocate_info;
    }

    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        return info;

    }

    VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        return info;
    }


    VkEventCreateInfo event_create_info(VkEventCreateFlags flags)
    {
        VkEventCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        return info;
    }

    VkAttachmentDescription attachment_description(const VkFormat format, const VkImageLayout initial, const VkImageLayout final, const
        VkAttachmentStoreOp storeOpp , const VkAttachmentLoadOp loadOpp , const VkAttachmentStoreOp stencilStoreOpp, const VkAttachmentLoadOp stencilLoadOpp)
    {
        VkAttachmentDescription description = {};
        description.format = format;
        description.flags = 0;
        description.initialLayout = initial;
        description.finalLayout = final;
        description.storeOp = storeOpp;
        description.loadOp = loadOpp;
        description.stencilLoadOp = stencilLoadOpp;
        description.stencilStoreOp = stencilStoreOpp;
        description.samples = VK_SAMPLE_COUNT_1_BIT; // without msaa

        return description;
    }

    VkAttachmentReference attachment_reference(const uint32_t& attachment, const VkImageLayout layout)
    {
        VkAttachmentReference attachment_reference = {};
        attachment_reference.attachment = attachment;
        attachment_reference.layout = layout;
        return attachment_reference;
    }

    VkSubpassDescription subpass_description(VkSubpassDescriptionFlags flags, VkPipelineBindPoint bindPoint, const VkAttachmentReference* pInputRef ,const VkAttachmentReference* pColorRef, const VkAttachmentReference* pDepthStencilRef,
        const uint32_t& colorRefCount, const uint32_t& inputRefCount)
    {
        VkSubpassDescription description = {};
        description.flags = flags;
        description.colorAttachmentCount = colorRefCount;
        description.pColorAttachments = pColorRef;
        description.pDepthStencilAttachment = pDepthStencilRef;
        description.inputAttachmentCount = inputRefCount;
        description.pInputAttachments = pInputRef;
        return description;
    }

    VkSubpassDependency subpass_dependency()
    {
        VkSubpassDependency dependency = {};

        return dependency;
    }

    VkRenderPassCreateInfo render_pass_create_info(VkRenderPassCreateFlags flags, const uint32_t& attachmentCount, const uint32_t& subpassCount, const uint32_t& dependencyCount, const VkSubpassDescription* pSubpasses,
        const VkAttachmentDescription* pAttachments, const VkSubpassDependency* pSubpassDependencies)
    {
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.flags = flags;
        info.attachmentCount = attachmentCount;
        info.pAttachments = pAttachments;
        info.pSubpasses = pSubpasses;
        info.subpassCount = subpassCount;
        info.dependencyCount = dependencyCount;
        info.pDependencies = pSubpassDependencies;
        info.pNext = nullptr;
        return info;
    }

    VkFramebufferCreateInfo framebuffer_create_info(VkFramebufferCreateFlags flags, const VkImageView* pAttachments, const VkRenderPass renderPass, const uint32_t& attachmentCount, const uint32_t& layoutCount,
        const uint32_t& height, const uint32_t& width)
    {
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.flags = flags;
        info.pAttachments = pAttachments;
        info.attachmentCount = attachmentCount;
        info.width = width;
        info.height = height;
        info.renderPass = renderPass;
        info.pNext = nullptr;
        return info;
    }

    VkRenderPassBeginInfo render_pass_begin_info( const VkRenderPass renderPass, const VkFramebuffer framebuffer,  const uint32_t& clearValueCount, const VkClearValue* pClearValues, const VkRect2D area)
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = renderPass;
        info.clearValueCount = clearValueCount;
        info.pClearValues = pClearValues;
        info.framebuffer = framebuffer;
        info.renderArea = area;
        info.pNext = nullptr;
        return info;
    }

    VkSubmitInfo submit_info(const VkCommandBuffer* pCommandBuffers, const VkSemaphore* pSignalSemaphores, const VkSemaphore* pWaitSemaphores, const VkPipelineStageFlags* pWaitDstStageMask,
        const uint32_t& cmdCount, const uint32_t& signalSemaphoreCount, const uint32_t& waitSemaphoreCount)
    {
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = cmdCount;
        info.pCommandBuffers = pCommandBuffers;
        info.pSignalSemaphores = pSignalSemaphores;
        info.pWaitSemaphores = pWaitSemaphores;
        info.signalSemaphoreCount = signalSemaphoreCount;
        info.waitSemaphoreCount = waitSemaphoreCount;
        info.pWaitDstStageMask = pWaitDstStageMask;
        info.pNext = nullptr;
        return info;
    }

}

