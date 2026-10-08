#pragma once
#include "utils.hpp"


struct ObjectConstant {

    glm::mat4 worldTransform{};
    glm::mat4 worldTextureTransform{};

};

struct MainPassUbo {

    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 projView;

};

class FrameContext {


public:

    FrameContext(const FrameContext& other) = delete;
    FrameContext& operator=(const FrameContext& other) = delete;
    explicit FrameContext(const VmaAllocator allocator, const uint32_t* queue_index);

    std::unique_ptr<UniformBuffer<MainPassUbo>> m_main_pass_uniform_buffer = nullptr;

    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore render_semaphore = VK_NULL_HANDLE;
    VkSemaphore swapchain_semaphore = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandPool command_pool = VK_NULL_HANDLE;

    DeletionQueue deletion_queue;

};


