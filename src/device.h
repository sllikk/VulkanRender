#pragma once
#include  "utils.hpp"
#include  "window.h"
#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

class FrameInFlight;

class VulkanContext {

  Window& m_window = Window::getInstance();

  VkDevice m_device = VK_NULL_HANDLE;
  VkInstance m_instance = VK_NULL_HANDLE;
  VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debug_utils_messenger_ext = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface_khr = VK_NULL_HANDLE;
  VkQueue m_graphics_queue = VK_NULL_HANDLE;
  VkQueue m_compute_queue = VK_NULL_HANDLE;
  uint32_t m_graphics_family_index = 0;
  uint32_t m_compute_family_index = 0;
  vkb::Device m_vkb_device{};
  VmaAllocator m_allocator = nullptr;

private:

  // stuff for immediate execute
  VkCommandPool m_command_pool = VK_NULL_HANDLE;
  VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
  VkFence m_fence = VK_NULL_HANDLE;

  void create_immediate_primitives();


public:

  VulkanContext(const VulkanContext& rhs) = delete;
  VulkanContext operator=(const VulkanContext& rhs) = delete;
  VulkanContext();

  void init();
  void destroy() const;
  void immediate_submit(const std::function<void(const VkCommandBuffer cmd)>&& function) const;

private:



public:

  /*getters*/
  inline  VmaAllocator                  get_allocator() const { return m_allocator; }
  inline  vkb::Device                   get_vkb_device() const { return  m_vkb_device;}
  inline  VkPhysicalDevice              get_physical_device() const { return m_physical_device; }
  inline  VkDevice                      get_device() const { return m_device; }
  inline  VkDebugUtilsMessengerEXT      get_debug_utils_messenger_ext() const { return m_debug_utils_messenger_ext; }
  inline  VkInstance                    get_instance() const { return m_instance; }
  inline  VkSurfaceKHR                  get_surface() const { return m_surface_khr; }
  inline  VkQueue                       get_graphics_queue() const { return m_graphics_queue; }
  inline  VkQueue                       get_compute_queue() const { return m_compute_queue; }
  inline  uint32_t                      get_graphics_family_index() const { return m_graphics_family_index; }
  inline  uint32_t                      get_compute_family_index() const { return m_compute_family_index; }


};



class FrameInFlight {
public:
  VkFence m_fence = VK_NULL_HANDLE;
  VkSemaphore m_render_semaphore = VK_NULL_HANDLE;
  VkSemaphore m_swap_chain_semaphore = VK_NULL_HANDLE;
  VkCommandBuffer m_cmd_buffer = VK_NULL_HANDLE;
  VkCommandPool m_cmd_pool = VK_NULL_HANDLE;
  VulkanContext* m_vulkan_context = nullptr;

public:

  uint32_t frame_counter = 0;

public:

  FrameInFlight(const FrameInFlight& other) = delete;
  FrameInFlight& operator=(const FrameInFlight& other) = delete;

  FrameInFlight(const FrameInFlight&& other) = delete;
  FrameInFlight&& operator=(const FrameInFlight&& other) = delete;

  FrameInFlight(VulkanContext* context);

  void init();
  void destroy() const;

  inline VkFence            get_fence() const { return m_fence; };
  inline VkCommandBuffer    get_command_buffer() const { return m_cmd_buffer; }
  inline VkCommandPool      get_command_pool() const { return m_cmd_pool; }
  inline VkSemaphore        get_render_semaphore() const { return m_render_semaphore; }
  inline VkSemaphore        get_swap_chain_semaphore() const { return m_swap_chain_semaphore; }


};

