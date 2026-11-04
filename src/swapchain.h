#pragma once

#include "device.h"

class VulkanContext;
class FrameInFlight;

class Swap_chain {

  VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
  VkFormat m_format = VK_FORMAT_B8G8R8A8_SRGB;
  VulkanContext* m_vulkan_context = nullptr;
  Window& m_window = Window::getInstance();
  bool isResized = false;
  uint32_t m_swap_chain_image_index = 0;

public:

  Swap_chain(const Swap_chain& rhs) = delete;
  Swap_chain& operator=(const Swap_chain& rhs) = delete;
  Swap_chain(const Swap_chain&& rhs) = delete;
  Swap_chain&& operator=(const Swap_chain&& rhs) = delete;
  Swap_chain(VulkanContext* vulkan_context);

  void init();
  void update();

  void destroy() const;


  std::vector<VkImage> m_images{};
  std::vector<VkImageView> m_image_views{};


  inline VkSwapchainKHR              get_swap_chain() const { return m_swap_chain; }
  inline VkFormat                    get_format() const { return m_format; }
  inline bool                        is_resized() const { return isResized; };
  inline uint32_t                    get_swap_chain_image_index() const { return m_swap_chain_image_index; }
  void                               set_resized(bool state) { isResized = state; };


};

