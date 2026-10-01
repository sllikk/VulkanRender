#pragma once
#include  "utils.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "FrameResources.h"

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

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
    {
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

    uint32_t get_family_index() const
    {
        return m_queue_family_index;
    }

    VkQueue get_queue() const
    {
        return m_queue;
    }
};


class Engine {

    GLFWwindow* m_window = nullptr;
    std::string_view m_window_title  = "";
    uint32_t m_window_width = 0;
    uint32_t m_window_height = 0;
    bool m_window_is_close = false;

    VkDevice m_device = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE; 

    VkDebugUtilsMessengerEXT m_messenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkRect2D m_scissor{};
    VkViewport m_viewport{};

    VkFormat m_swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
    std::vector<VkImage> m_swapchain_images{};
    std::vector<VkImageView> m_swapchain_images_image_views{};

    uint32_t m_current_frame_index = 0;

    uint32_t m_queue_graphics_family_index = 0;
    uint32_t m_queue_compute_family_index = 0;

    uint32_t m_swapchainIndex = 0;
    uint32_t m_frame_number = 0;

    FrameContext m_frame_contexts[FRAME_IN_FLIGHTS]{};
    FrameContext& m_get_frame_context_index() { return m_frame_contexts[m_frame_number % FRAME_IN_FLIGHTS]; }

    DeletionQueue m_main_deletion_queue{};
    TransferQueue m_transfer_queue{};

    VmaAllocator m_vma_allocator = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

    bool isResized = false;

    // Render items for drawing
    std::unique_ptr<RenderItem> m_triangle_item = nullptr;

public:

    Engine(const Engine& other) = delete;
    Engine& operator=(const Engine& other) = delete;

public:

    Engine(const uint32_t& width, const uint32_t& height, const std::string_view title);

    void init_window();
    void init_vulkan();
    void init_commands();
    void init_sync_objects();
    void create_swapchain(const uint32_t width, const uint32_t height);
    void resize();
    void destroy_swapchain() const;
    void init_vertex_buffer();
    void init_pipeline();
    void init_image_for_triangle();


public:

    void init();
    void update();
    void render();
    void cleanup();



};

