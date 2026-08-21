#pragma once
#include  "utils.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    VkCommandBuffer m_cmd_buffer = VK_NULL_HANDLE;
    VkCommandPool m_cmd_pool = VK_NULL_HANDLE;
    VkRect2D m_scissor{};
    VkViewport m_viewport{};

    VkFormat m_swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_images_image_views;


    VkFence m_fence = VK_NULL_HANDLE;
    VkSemaphore m_semaphore = VK_NULL_HANDLE;

    uint32_t m_current_frame_index = 0;
    uint32_t m_queue_graphics_index = 0;
    uint32_t m_queue_compute_index = 0;


public:

  Engine(const Engine& other) = delete;
  Engine& operator=(const Engine& other) = delete;


public:

  Engine(const uint32_t& width, const uint32_t& height, const std::string_view& title);

    void init_window();
    void init_vulkan();
    void init_commands();
    void init_sync_objects();
    void create_swapchain(const uint32_t width, const uint32_t& height);
    void destroy_swapchain() const;

public:

    void init();
    void update();
    void render();
    void cleanup() const;



};

