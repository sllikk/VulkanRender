#pragma once
#include "utils.hpp"

struct ObjectConstant {

    glm::mat4 worldTransform{};

};


struct FrameContext {


    FrameContext(const FrameContext& other) = delete;
    FrameContext& operator=(const FrameContext& other) = delete;

    FrameContext() = default;

    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore render_semaphore = VK_NULL_HANDLE;
    VkSemaphore swapchain_semaphore = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandPool command_pool = VK_NULL_HANDLE;

    DeletionQueue deletion_queue;

};

