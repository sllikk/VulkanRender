#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "vma/vk_mem_alloc.h"

#include <array>
#include <deque>
#include <filesystem>
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
    //glm::vec4 color;


};

//
// This buffer uses for everything mem allocations
//


template<typename T>
class UniformBuffer
{
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VmaAllocator m_allocator;
    void* m_data;

public:

    UniformBuffer(const UniformBuffer& other) = delete;
    UniformBuffer operator=(const UniformBuffer& other) = delete;

    UniformBuffer(VmaAllocator allocator, const uint32_t* queue_index) {

        m_allocator = allocator;
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.size = sizeof(T);
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buffer_create_info.pQueueFamilyIndices = queue_index;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pNext = nullptr;

        VmaAllocationCreateInfo allocation_create_info{};
        allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        THROW_IF_ERROR(vmaCreateBuffer(m_allocator, &buffer_create_info, &allocation_create_info, &m_buffer, &m_allocation, nullptr));
        THROW_IF_ERROR(vmaMapMemory(m_allocator, m_allocation, &m_data));

    }

    void update(const T* newData, const uint32_t& size) {
        memcpy(m_data, newData, size);
    }

    void destroy() const {
        vmaUnmapMemory(m_allocator, m_allocation);
        vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
    }

    VkBuffer get_buffer() const {
        return m_buffer;
    }

    VkDeviceSize get_size() const {
        return static_cast<VkDeviceSize>(sizeof(T));
    }
};



namespace VkUtils
{
  inline  VkCommandPoolCreateInfo command_pool_create_info (uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo command_pool_create_info = {};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = flags;
        command_pool_create_info.queueFamilyIndex = queueFamilyIndex;
        command_pool_create_info.pNext = nullptr;
        return command_pool_create_info;
    }

    inline VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool cmd_pool, VkCommandBufferLevel level, const uint32_t count)
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = cmd_pool;
        command_buffer_allocate_info.commandBufferCount = count;
        command_buffer_allocate_info.level = level;
        command_buffer_allocate_info.pNext = nullptr;

        return command_buffer_allocate_info;
    }

    inline VkCommandBufferBeginInfo command_buffer_begin_info(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info)
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags = flags;
        info.pInheritanceInfo = inheritance_info;
        info.pNext = nullptr;

        return info;
    }


   inline VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags)
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        return info;

    }

    inline VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        return info;
    }


   inline VkEventCreateInfo event_create_info(VkEventCreateFlags flags)
    {
        VkEventCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        return info;
    }

   inline  VkAttachmentDescription attachment_description(const VkFormat format, const VkImageLayout initial, const VkImageLayout final, const
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

   inline VkAttachmentReference attachment_reference(const uint32_t& attachment, const VkImageLayout layout)
    {
        VkAttachmentReference attachment_reference = {};
        attachment_reference.attachment = attachment;
        attachment_reference.layout = layout;
        return attachment_reference;
    }

    inline VkSubpassDescription subpass_description(VkSubpassDescriptionFlags flags, VkPipelineBindPoint bindPoint, const VkAttachmentReference* pInputRef ,const VkAttachmentReference* pColorRef, const VkAttachmentReference* pDepthStencilRef,
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

   inline VkSubpassDependency subpass_dependency()
    {
        VkSubpassDependency dependency = {};

        return dependency;
    }

   inline VkRenderPassCreateInfo render_pass_create_info(VkRenderPassCreateFlags flags, const uint32_t& attachmentCount, const uint32_t& subpassCount, const uint32_t& dependencyCount, const VkSubpassDescription* pSubpasses,
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

  inline  VkFramebufferCreateInfo framebuffer_create_info(const VkFramebufferCreateFlags flags, const VkImageView* pAttachments, const VkRenderPass renderPass, const uint32_t& attachmentCount, const uint32_t& layoutCount,
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

   inline VkRenderPassBeginInfo render_pass_begin_info( const VkRenderPass renderPass, const VkFramebuffer framebuffer,  const uint32_t& clearValueCount, const VkClearValue* pClearValues, const VkRect2D area)
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

   inline VkCommandBufferSubmitInfo command_buffer_submit(const VkCommandBuffer cmd)
    {
        VkCommandBufferSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        info.pNext = nullptr;
        info.commandBuffer = cmd;
        info.deviceMask = 0;

        return info;
    }

  inline  VkSemaphoreSubmitInfo semaphore_submit_info(const VkSemaphore semaphore, const VkPipelineStageFlags2 flags)
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

  inline  VkSubmitInfo2 submit_info2(const VkCommandBufferSubmitInfo* pCmdSubmit, const uint32_t cmdSubmitCount, const VkSubmitFlags flags, const VkSemaphoreSubmitInfo* pWaitSemaphores, const VkSemaphoreSubmitInfo* pSignalSemaphores)
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

  inline  VkPresentInfoKHR present_info_khr(const uint32_t* pImageIndices, const VkSwapchainKHR* pSwapchains, const VkSemaphore* pWaitSemaphores, const uint32_t& waitSemaphoreCount, const uint32_t& swapchainCount)
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

  inline  VkImageSubresourceRange subresource_range(VkImageAspectFlags aspect_flags)
    {
        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask = aspect_flags;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
        subresource_range.baseArrayLayer = 0;
        subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

        return subresource_range;
    }

  inline  void transition_image(const VkCommandBuffer cmd, const VkImage image, const VkImageLayout currentLayout, const VkImageLayout newLayout)
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

   inline void copy_buffer_transfer_queue(VkDevice device, VkQueue transferQueue, VkQueue dstQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, const uint32_t queue_family_index)
    {
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        VkCommandPool cmd_pool = VK_NULL_HANDLE;

        const auto pool_create_info = VkUtils::command_pool_create_info(queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        THROW_IF_ERROR(vkCreateCommandPool(device, &pool_create_info, nullptr, &cmd_pool));

        const auto cmd_buffer_alloc_info = VkUtils::command_buffer_allocate_info(cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        THROW_IF_ERROR(vkAllocateCommandBuffers(device, &cmd_buffer_alloc_info, &cmd));

        // START RECORDING
        THROW_IF_ERROR(vkResetCommandBuffer(cmd, 0));
        const auto cmd_begin_info = VkUtils::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
        THROW_IF_ERROR(vkBeginCommandBuffer(cmd, &cmd_begin_info));

        VkBufferCopy region{};
        region.dstOffset = 0;
        region.srcOffset = 0;
        region.size = size;

        vkCmdCopyBuffer(cmd, srcBuffer, dstBuffer, 1, &region);
        THROW_IF_ERROR(vkEndCommandBuffer(cmd));

        VkCommandBufferSubmitInfo cmd_submit_info = VkUtils::command_buffer_submit(cmd);
        VkSubmitInfo2 submit_info2{};
        submit_info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info2.commandBufferInfoCount = 1;
        submit_info2.pCommandBufferInfos = &cmd_submit_info;
        submit_info2.pNext = nullptr;

        THROW_IF_ERROR(vkQueueSubmit2(transferQueue, 1, &submit_info2, nullptr));
        THROW_IF_ERROR(vkQueueWaitIdle(dstQueue));

        vkDeviceWaitIdle(device);
        vkFreeCommandBuffers(device, cmd_pool, 1, &cmd);
        vkDestroyCommandPool(device, cmd_pool, nullptr);

    }

    static std::vector<char> load_shaders(const std::string& shader_path)
    {
        std::ifstream file(shader_path, std::ios::ate | std::ios::binary );
        std::cout << shader_path << std::endl;

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;


    }

}


class TransferQueue
{
    VkCommandBuffer m_cmd_buffer = VK_NULL_HANDLE;
    VkCommandPool m_cmd_pool = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    uint32_t m_queue_family_index = 0;


public:

    TransferQueue(const TransferQueue& q) = delete;
    TransferQueue operator=(const TransferQueue& q) = delete;
    TransferQueue() = default;

    void init(VkDevice device, VkQueue queue, uint32_t queue_family_index)
    {
        VkFenceCreateInfo fence_create_info = VkUtils::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
        THROW_IF_ERROR(vkCreateFence(device, &fence_create_info, nullptr, &m_fence));

        VkCommandPoolCreateInfo cmd_pool_create_info = VkUtils::command_pool_create_info(queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        THROW_IF_ERROR(vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &m_cmd_pool));

        VkCommandBufferAllocateInfo cmd_allocate_info = VkUtils::command_buffer_allocate_info(m_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        THROW_IF_ERROR(vkAllocateCommandBuffers(device, &cmd_allocate_info, &m_cmd_buffer));

        m_queue = queue;
        m_queue_family_index  = queue_family_index;
        m_device = device;
    }

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) const {
        THROW_IF_ERROR(vkResetFences(m_device, 1, &m_fence));
        THROW_IF_ERROR(vkResetCommandBuffer(m_cmd_buffer, 0));
        VkCommandBufferBeginInfo begin_info = VkUtils::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);

        THROW_IF_ERROR(vkBeginCommandBuffer(m_cmd_buffer, &begin_info));

        function(m_cmd_buffer); // some work what we wanna

        THROW_IF_ERROR(vkEndCommandBuffer(m_cmd_buffer));

        const VkCommandBufferSubmitInfo command_buffer_submit_info = VkUtils::command_buffer_submit(m_cmd_buffer);
        VkSubmitInfo2 submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;

        THROW_IF_ERROR(vkQueueSubmit2(m_queue, 1, &submit_info, m_fence));
        THROW_IF_ERROR(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX));
        THROW_IF_ERROR(vkResetCommandPool(m_device, m_cmd_pool, 0));

    }

    void destroy()
    {
        vkDestroyFence(m_device, m_fence, nullptr);
        vkFreeCommandBuffers(m_device, m_cmd_pool, 1, &m_cmd_buffer);
        vkDestroyCommandPool(m_device, m_cmd_pool, nullptr);
    }

  inline  uint32_t get_family_index() const
    {
        return m_queue_family_index;
    }

  inline  VkQueue get_queue() const
    {
        return m_queue;
    }
};


inline std::filesystem::path GetCurrentWorkingDirectoryPath() {

    std::filesystem::path path = std::filesystem::current_path().parent_path().parent_path();
    return path;
}