#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"


#include <array>
#include <deque>
#include <fstream>
#include <queue>
#include <stack>
#include <vector>

#include <functional>
#include <iostream>
#include <memory>


constexpr uint32_t FRAME_IN_FLIGHTS = 3;


struct DeletionQueue {

 std::vector<std::function<void()>> queue{};

    void flush()  {

        for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
            (*it)();
        }

        queue.clear();
  }

    void add_to_queue(const std::function<void()>&& function) {

        queue.push_back(function);
    }


};

inline void THROW_IF_ERROR(const VkResult& result)
{
    if (result != VK_SUCCESS)
    {
        std::cerr << string_VkResult(result) << std::endl;
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

    VkCommandBufferBeginInfo command_buffer_begin_info(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info)
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = flags;
        info.pInheritanceInfo = inheritance_info;
        info.pNext = nullptr;

        return info;
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

    VkFramebufferCreateInfo framebuffer_create_info(const VkFramebufferCreateFlags flags, const VkImageView* pAttachments, const VkRenderPass renderPass, const uint32_t& attachmentCount, const uint32_t& layoutCount,
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
        info.layers = layoutCount;
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

    VkCommandBufferSubmitInfo command_buffer_submit(const VkCommandBuffer cmd)
    {
        VkCommandBufferSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        info.pNext = nullptr;
        info.commandBuffer = cmd;
        info.deviceMask = 0;

        return info;
    }

    VkSemaphoreSubmitInfo semaphore_submit_info(const VkSemaphore semaphore, const VkPipelineStageFlags2 flags)
    {
        VkSemaphoreSubmitInfo info{};
        info.pNext = nullptr;
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        info.semaphore = semaphore;
        info.stageMask = flags;
        info.deviceIndex = 0;
        info.value = 1;
        return info;
    }

    VkSubmitInfo2 submit_info2(const VkCommandBufferSubmitInfo* pCmdSubmit, const uint32_t cmdSubmitCount, const VkSubmitFlags flags, const VkSemaphoreSubmitInfo* pWaitSemaphores, const VkSemaphoreSubmitInfo* pSignalSemaphores)
    {
        VkSubmitInfo2 info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        info.pNext = nullptr;
        info.pCommandBufferInfos = pCmdSubmit;
        info.pSignalSemaphoreInfos = pSignalSemaphores;
        info.pWaitSemaphoreInfos = pWaitSemaphores;
        info.signalSemaphoreInfoCount = 1;
        info.waitSemaphoreInfoCount = 1;
        info.commandBufferInfoCount = 1;
        info.flags = flags;
        return info;
    }

    VkPresentInfoKHR present_info_khr(const uint32_t* pImageIndices, const VkSwapchainKHR* pSwapchains, const VkSemaphore* pWaitSemaphores, const uint32_t& waitSemaphoreCount, const uint32_t& swapchainCount)
    {
        VkPresentInfoKHR present_info_khr = {};
        present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info_khr.pImageIndices = pImageIndices;
        present_info_khr.pSwapchains = pSwapchains;
        present_info_khr.swapchainCount = swapchainCount;
        present_info_khr.pWaitSemaphores = pWaitSemaphores;
        present_info_khr.waitSemaphoreCount = waitSemaphoreCount;
        present_info_khr.pNext = nullptr;
        return  present_info_khr;
    }

    VkImageSubresourceRange subresource_range(VkImageAspectFlags aspect_flags)
    {
        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask = aspect_flags;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
        subresource_range.baseArrayLayer = 0;
        subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        return subresource_range;
    }

    void transition_image(const VkCommandBuffer cmd, const VkImage image, const VkImageLayout currentLayout, const VkImageLayout newLayout)
    {
        VkImageMemoryBarrier2 image_memory_barrier = {};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

        image_memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_memory_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        image_memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

        image_memory_barrier.newLayout = newLayout;
        image_memory_barrier.oldLayout = currentLayout;

        VkImageAspectFlags aspect_flags = {};

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
        {
            aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else
        {
            aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        image_memory_barrier.subresourceRange = subresource_range(aspect_flags);
        image_memory_barrier.image = image;
        image_memory_barrier.pNext = nullptr;

        
        VkDependencyInfo dependency_info = {};
        dependency_info.pNext = nullptr;
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

        dependency_info.imageMemoryBarrierCount = 1;
        dependency_info.pImageMemoryBarriers = &image_memory_barrier;

        vkCmdPipelineBarrier2(cmd, &dependency_info);
    }


    static std::vector<char> load_shaders(const std::string& shader_path)
    {
        std::ifstream file(shader_path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;


    }

}

