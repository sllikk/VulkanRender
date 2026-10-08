#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "material.h"
#include  "utils.hpp"
#include "FrameResources.h"


struct RenderItem {

    uint32_t vertices_count = 0;
    uint32_t vertices_start = 0;

    VkBuffer vertex_buffer = VK_NULL_HANDLE;


};


class Timer {

    float m_delta_time;
    float m_current_time;
    float m_prev;

public:

    Timer() {
        m_delta_time = 0.0f;
        m_prev = 0.0f;
        m_current_time = static_cast<float>(glfwGetTime());
    }

    Timer(const Timer& other) = delete;
    Timer operator=(const Timer& other) = delete;


    void update() {
        m_current_time = static_cast<float>(glfwGetTime());
        m_delta_time = m_current_time - m_prev;
        m_prev = m_current_time;
    }

    float GetCurrentTime() const {
        return m_current_time;
    }

    float GetDeltaTime() const {
        return m_delta_time;
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

    std::array<std::unique_ptr<FrameContext>, FRAME_IN_FLIGHTS> m_frame_contexts{};
    FrameContext& m_get_frame_context_index() const { return *m_frame_contexts[m_frame_number % FRAME_IN_FLIGHTS]; }

    DeletionQueue m_main_deletion_queue{};
    TransferQueue m_transfer_queue{};

    VmaAllocator m_vma_allocator = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

    bool isResized = false;

    // Render items for drawing
    std::unique_ptr<RenderItem> m_triangle_item = nullptr;
    Timer m_timer{};
    ObjectConstant m_object_constant{};

    VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;

public:

    Engine(const Engine& other) = delete;
    Engine& operator=(const Engine& other) = delete;

public:

    Engine(const uint32_t& width, const uint32_t& height, const std::string_view title);

    void init_window();
    void init_vulkan();
    void init_frame_resources();
    void init_commands();
    void init_sync_objects();
    void create_swapchain(const uint32_t width, const uint32_t height);
    void resize();
    void destroy_swapchain() const;
    void init_descriptors();
    void init_vertex_buffer();
    void init_pipeline();
    void init_texture();


public:

    void init();
    void update();
    void render();
    void cleanup();



};

