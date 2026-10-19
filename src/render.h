#pragma once

#include "FrameResources.h"
#include "material.h"
#include "utils.hpp"
#include "window.h"


class Timer;
struct Material {

    glm::vec4 color;

};

struct RenderItem {

    uint32_t vertices_count = 0;
    uint32_t vertices_start = 0;
    glm::mat4 transformation = glm::mat4(1.0f);

    VkBuffer vertex_buffer = VK_NULL_HANDLE;
    VkBuffer index_buffer = VK_NULL_HANDLE;
    Material material{};


};

struct GPUImage {

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_image_view = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    uint32_t width;
    uint32_t height;
};


class Render {

    Window& m_window = Window::getInstance();
    std::string_view m_window_title  = "";

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

    glm::vec3 position = {0, 0, 0, };
    float speed_cam = 3.0f;

    // Render items for drawing
    std::unique_ptr<RenderItem> m_triangle_item = nullptr;
    ObjectConstant m_object_constant{};

    std::vector<std::unique_ptr<RenderItem>> m_draw_items{};

    VkDescriptorSet m_descriptor_set = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
    GPUImage m_texture;
    GPUImage m_depth_buffer;

public:

     Render(const  Render& other) = delete;
     Render& operator=(const  Render& other) = delete;
     Render();

protected:


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
    void init_draw_items();
    void create_depth_buffer(const uint32_t width, const uint32_t height);
    void destroy_depth_buffer() const;
    void draw_items(Timer& timer, const VkCommandBuffer& cmd);
public:

    void init();
    void update(Timer& timer);
    void render(Timer& timer);
    void cleanup();



};

