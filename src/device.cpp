#include "device.h"

#include "GLFW/glfw3.h"

#include <VkBootstrap.h>

VulkanContext::VulkanContext() {

}


void VulkanContext::init() {
  bool blsValidationError = false;
#if _DEBUG
  blsValidationError = true;
#    endif

  vkb::InstanceBuilder instance_builder{};
  auto instance_ret = instance_builder
                          .set_app_name("VkApp")
                          .set_engine_name("Engine")
                          .require_api_version(1,3, 0)
                          .request_validation_layers(blsValidationError)
                          .use_default_debug_messenger()
                          .build(); // build is always called last

  if (!instance_ret) {
    std::cout << "Failed to create Vulkan instance. Error: {}\n" << instance_ret.error().message();
    return;
  }

  auto system_info_ret = vkb::SystemInfo::get_system_info();
  if (!system_info_ret)
  {
    std::cout << "Failed to create SystemInfo. Error: {}\n" << system_info_ret.error().message();
    return;
  }

  THROW_IF_ERROR(glfwCreateWindowSurface(instance_ret.value(), m_window.get_window_ptr(), VK_NULL_HANDLE, &m_surface_khr));

  auto system_info = system_info_ret.value();

  VkPhysicalDeviceFeatures physical_device_features{};
  physical_device_features.samplerAnisotropy = VK_TRUE;

  VkPhysicalDeviceVulkan13Features features3{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  features3.dynamicRendering = VK_TRUE;
  features3.synchronization2 = VK_TRUE;


  vkb::PhysicalDeviceSelector physical_device_selector{instance_ret.value()};
  auto physical_dev_ret = physical_device_selector
  .set_surface(m_surface_khr)
  .set_required_features(physical_device_features)
  .set_required_features_13(features3)
  .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
  .select();

  if (!physical_dev_ret) {
      if (physical_dev_ret.error() == vkb::PhysicalDeviceError::no_suitable_device) {
          throw std::runtime_error(physical_dev_ret.error().message());
          return;
      }

      if (physical_dev_ret.error() == vkb::PhysicalDeviceError::no_physical_devices_found) {
        throw std::runtime_error(physical_dev_ret.error().message());
        return;
      }
  }

  vkb::DeviceBuilder device_builder{ physical_dev_ret.value() };
  auto device_ret = device_builder.build();
  if (!device_ret) {
      throw std::runtime_error(device_ret.error().message());
      return;
  }

  m_instance = instance_ret.value();
  m_debug_utils_messenger_ext = instance_ret.value().debug_messenger;
  m_physical_device = physical_dev_ret.value();
  m_device = device_ret.value();
  m_vkb_device = device_ret.value();

  m_graphics_queue = device_ret.value().get_queue(vkb::QueueType::graphics).value();
  m_graphics_family_index =  device_ret.value().get_queue_index(vkb::QueueType::graphics).value();

  m_compute_queue = device_ret.value().get_queue(vkb::QueueType::compute).value();
  m_compute_family_index =  device_ret.value().get_queue_index(vkb::QueueType::compute).value();

  VmaAllocatorCreateInfo info{};
  info.device = m_device;
  info.instance = m_instance;
  info.physicalDevice = m_physical_device;
  info.vulkanApiVersion = VK_API_VERSION_1_3;

  THROW_IF_ERROR(vmaCreateAllocator(&info, &m_allocator));

  create_immediate_primitives();


}


void VulkanContext::destroy() const {

  vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
  vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  vkDestroyFence(m_device, m_fence, nullptr);

  vmaDestroyAllocator(m_allocator);
  vkDestroyDevice(m_device, nullptr);

  vkDestroySurfaceKHR(m_instance, m_surface_khr, nullptr);

  vkb::destroy_debug_utils_messenger(m_instance,  m_debug_utils_messenger_ext, nullptr);
  vkDestroyInstance(m_instance, nullptr);

}


void VulkanContext::create_immediate_primitives() {

  const VkCommandPoolCreateInfo& command_pool_create_info = VkUtils::command_pool_create_info(get_graphics_family_index(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  THROW_IF_ERROR(vkCreateCommandPool(get_device(), &command_pool_create_info, nullptr, &m_command_pool));

  const VkCommandBufferAllocateInfo& command_buffer_allocate_info = VkUtils::command_buffer_allocate_info(m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
  THROW_IF_ERROR(vkAllocateCommandBuffers(get_device(), &command_buffer_allocate_info, &m_command_buffer));

  const VkFenceCreateInfo& fence_create_info = VkUtils::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  THROW_IF_ERROR(vkCreateFence(get_device(), &fence_create_info, nullptr, &m_fence));

}


void VulkanContext::immediate_submit(const std::function<void(const VkCommandBuffer cmd)> &&function) const {

  THROW_IF_ERROR(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX));

  vkResetFences(m_device, 1, &m_fence);
  vkResetCommandBuffer(m_command_buffer, 0);
  vkResetCommandPool(m_device, m_command_pool, 0);

  const VkCommandBufferBeginInfo& begin_info = VkUtils::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
  THROW_IF_ERROR(vkBeginCommandBuffer(m_command_buffer, &begin_info));

  function(m_command_buffer);

  THROW_IF_ERROR(vkEndCommandBuffer(m_command_buffer));

  const VkCommandBufferSubmitInfo& cmd_submit_info = VkUtils::command_buffer_submit(m_command_buffer);
  VkSubmitInfo2 submit_info = VkUtils::submit_info2(&cmd_submit_info, 1, 0, nullptr, nullptr);

  THROW_IF_ERROR(vkQueueSubmit2(m_graphics_queue, 1, &submit_info, m_fence));

  THROW_IF_ERROR(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX));

}

FrameInFlight::FrameInFlight(VulkanContext *context) : m_vulkan_context(context) {
}


void FrameInFlight::init() {

  const VkCommandPoolCreateInfo& command_pool_create_info = VkUtils::command_pool_create_info(m_vulkan_context->get_graphics_family_index(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  THROW_IF_ERROR(vkCreateCommandPool(m_vulkan_context->get_device(), &command_pool_create_info, nullptr, &m_cmd_pool));

  const VkCommandBufferAllocateInfo& command_buffer_allocate_info = VkUtils::command_buffer_allocate_info(m_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
  THROW_IF_ERROR(vkAllocateCommandBuffers(m_vulkan_context->get_device(), &command_buffer_allocate_info, &m_cmd_buffer));

  const VkFenceCreateInfo& fence_create_info = VkUtils::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  THROW_IF_ERROR(vkCreateFence(m_vulkan_context->get_device(), &fence_create_info, nullptr, &m_fence));

  const VkSemaphoreCreateInfo& semaphore_create_info = VkUtils::semaphore_create_info(0);
  THROW_IF_ERROR(vkCreateSemaphore(m_vulkan_context->get_device(), &semaphore_create_info, nullptr, &m_swap_chain_semaphore));
  THROW_IF_ERROR(vkCreateSemaphore(m_vulkan_context->get_device(), &semaphore_create_info, nullptr, &m_render_semaphore));


}


void FrameInFlight::destroy() const {

  vkDestroySemaphore(m_vulkan_context->get_device(), m_render_semaphore, nullptr);
  vkDestroySemaphore(m_vulkan_context->get_device(), m_swap_chain_semaphore, nullptr);
  vkDestroyFence(m_vulkan_context->get_device(), m_fence, nullptr);

  vkFreeCommandBuffers(m_vulkan_context->get_device(), m_cmd_pool,  1, &m_cmd_buffer);
  vkDestroyCommandPool(m_vulkan_context->get_device(), m_cmd_pool, nullptr);



}
