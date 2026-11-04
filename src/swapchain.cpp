#include "swapchain.h"
#include <VkBootstrap.h>

Swap_chain::Swap_chain(VulkanContext *vulkan_context) : m_vulkan_context(vulkan_context) {

}


void Swap_chain::init() {

  VkSurfaceFormatKHR format;
  format.format = m_format;
  format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  vkb::SwapchainBuilder builder{ m_vulkan_context->get_vkb_device(), m_vulkan_context->get_surface()};
  auto swapchain_ret = builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
  .set_desired_min_image_count(2)
  .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
  .set_desired_extent(m_window.get_window_extent().m_window_width, m_window.get_window_extent().m_window_height)
  .set_desired_format(format).build();

  if (!swapchain_ret) {
      throw std::runtime_error(swapchain_ret.error().message());
  }

  m_images = swapchain_ret.value().get_images().value();
  m_image_views = swapchain_ret.value().get_image_views().value();
  m_swap_chain = swapchain_ret.value();

}


void Swap_chain::update() {

if (isResized == true) {

  destroy();
  m_window.get_size();
  init();

  isResized = false;
}

}
void Swap_chain::destroy() const {

  vkDestroySwapchainKHR(m_vulkan_context->get_device(), m_swap_chain, nullptr);

  for (int i = 0; i < m_images.size(); ++i) {

    vkDestroyImageView(m_vulkan_context->get_device(), m_image_views[i], nullptr);

  }



}