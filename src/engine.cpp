#include "engine.h"
#include "VkBootstrap.h"
#include <iostream>


Engine::Engine(const uint32_t &width, const uint32_t &height, const std::string_view& title)
    : m_window_title(title),  m_window_width(width), m_window_height(height)
{



}

void Engine::init_window()
{
    if (!glfwInit())
    {
        throw std::runtime_error("glfw is not init");
    }

    if (!glfwVulkanSupported())
    {
        throw std::runtime_error("glfw is not init");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_window_width, m_window_height, m_window_title.data(), nullptr, nullptr);
    if (m_window != nullptr)
    {
        std::cout << "Window initialized\n";
    }

}


void Engine::init_vulkan()
{
    bool blsValidationError = false;
#if _DEBUG
    blsValidationError = true;
#    endif

    vkb::InstanceBuilder instance_builder{};
    auto instance_ret = instance_builder
                            .set_app_name("VkApp")
                            .set_engine_name("VulkanEngine")
                            .require_api_version(1,3, 0)
                            .request_validation_layers(blsValidationError)
                            .use_default_debug_messenger()
                            .build(); // build is always called last

    // simple error checking and helpful error messages
    if (!instance_ret) {
        std::cout << "Failed to create Vulkan instance. Error: {}\n" << instance_ret.error().message();
        return;
    }

    auto system_info_ret = vkb::SystemInfo::get_system_info();
    if (!system_info_ret)
    {
        std::cout << "Failed to create SystemInfo. Error: {}\n" << system_info_ret.error().message();
    }
    auto system_info = system_info_ret.value();

    THROW_IF_ERROR(glfwCreateWindowSurface(instance_ret.value(), m_window, nullptr, &m_surface));

    vkb::PhysicalDeviceSelector physical_device_selector(instance_ret.value());
    physical_device_selector.set_surface(m_surface);
    physical_device_selector.set_minimum_version(1, 3);
    // Engine cannot function without this extensions
    VkPhysicalDeviceFeatures required_features{};
    required_features.multiViewport = true;

    VkPhysicalDeviceVulkan11Features features1{.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features1.multiview = true;


    VkPhysicalDeviceVulkan12Features features2{.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features2.timelineSemaphore = true;

    VkPhysicalDeviceVulkan13Features features3{.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features3.synchronization2 = true;

    physical_device_selector.set_required_features_11(features1);
    physical_device_selector.set_required_features_12(features2);
    physical_device_selector.set_required_features_13(features3);

    vkb::Result<vkb::PhysicalDevice> physical_device_selector_return = physical_device_selector.select();

    if (!physical_device_selector_return)
    {
        if (physical_device_selector_return.error() == vkb::PhysicalDeviceError::no_suitable_device)
        {
            const auto& reason = physical_device_selector_return.full_error();
            std::cerr << reason.type.message() << "\n";
            return;
        }
    }
    else
    {
        std::string name = physical_device_selector_return.value().name;
        std::cout << "GPU: " << name << "\n";
    }

    vkb::DeviceBuilder device_builder{physical_device_selector_return.value()};
    auto dev_ret = device_builder.build();
    if (!dev_ret)
    {
        throw std::runtime_error(dev_ret.error().message());
    }

    auto graphics_queue_ret = dev_ret.value().get_queue(vkb::QueueType::graphics);
    auto compute_queue_ret = dev_ret.value().get_queue(vkb::QueueType::compute);

    m_queue_graphics_family = dev_ret.value().get_queue_index(vkb::QueueType::graphics).value();
    m_queue_compute_family = dev_ret.value().get_queue_index(vkb::QueueType::compute).value();

    m_instance = instance_ret.value();
    m_messenger = instance_ret.value().debug_messenger;
    m_gpu = dev_ret.value().physical_device;
    m_device = dev_ret.value();
    m_graphics_queue = graphics_queue_ret.value();
    m_compute_queue = graphics_queue_ret.value();


}


void Engine::init_commands()
{
    const auto& cmd_pool_create_info = VkUtils::command_pool_create_info(m_queue_graphics_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    THROW_IF_ERROR(vkCreateCommandPool(m_device, &cmd_pool_create_info, nullptr, &m_cmd_pool));

    const auto& cmd_allocation_create_info = VkUtils::command_buffer_allocate_info(m_cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    THROW_IF_ERROR(vkAllocateCommandBuffers(m_device, &cmd_allocation_create_info, &m_cmd_buffer));


}


void Engine::init_sync_objects()
{
    const auto& fence_create_info = VkUtils::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    THROW_IF_ERROR(vkCreateFence(m_device, &fence_create_info, nullptr, &m_fence));

    const auto& semaphore_create_info = VkUtils::semaphore_create_info(0);
    THROW_IF_ERROR(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_swapchain_semaphore));
    THROW_IF_ERROR(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_render_semaphore));

}


void Engine::init_render_passes()
{
    const auto& color_attachment = VkUtils::attachment_description(m_swapchain_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE);

    const auto& color_attachment_ref = VkUtils::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    const auto& subpass = VkUtils::subpass_description(0, VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &color_attachment_ref, nullptr, 1, 0);
    const auto& render_pass_info = VkUtils::render_pass_create_info(0, 1, 1, 0, &subpass, &color_attachment, nullptr);

    THROW_IF_ERROR(vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass));
}


void Engine::init_framebuffers()
{
    const uint32_t count = static_cast<uint32_t>(m_swapchain_images_image_views.size());
    auto frame_buffer_create_info = VkUtils::framebuffer_create_info(0, nullptr, m_render_pass, 1, 1, m_window_height, m_window_width);

    m_framebuffers = std::vector<VkFramebuffer>(count);
    for (int i = 0; i < count; ++i)
    {
        frame_buffer_create_info.pAttachments = &m_swapchain_images_image_views[i];
        THROW_IF_ERROR(vkCreateFramebuffer(m_device, &frame_buffer_create_info, nullptr, &m_framebuffers[i]));

    }

}


void Engine::create_swapchain(const uint32_t width, const uint32_t& height)
{
    vkb::SwapchainBuilder swapchain_builder {m_gpu, m_device, m_surface};
    VkSurfaceFormatKHR const& surface_format_khr{.format = m_swapchain_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    vkb::Swapchain swapchain = swapchain_builder.set_desired_format(surface_format_khr)
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(width, height)
    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    .build().value();

    m_window_width = swapchain.extent.width;
    m_window_height = swapchain.extent.height;

    m_swapchain = swapchain;
    m_swapchain_images = swapchain.get_images().value();
    m_swapchain_images_image_views = swapchain.get_image_views().value();

}


void Engine::destroy_swapchain() const
{
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vkDestroyRenderPass(m_device, m_render_pass, nullptr);

    for (size_t i = 0; i < m_swapchain_images_image_views.size(); i++)
    {
        vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
        vkDestroyImageView(m_device, m_swapchain_images_image_views[i], nullptr);
    }
}


void Engine::init() {

    init_window();
    init_vulkan();
    init_commands();
    init_sync_objects();
    create_swapchain(m_window_width, m_window_height);
    init_render_passes();
    init_framebuffers();

}


void Engine::update() {

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        render();
    }
}


void Engine::render()
{
    //wait until the GPU has finished rendering the last frame. Timeout of 1 second
    THROW_IF_ERROR(vkWaitForFences(m_device, 1, &m_fence, true, UINT_FAST64_MAX));
    THROW_IF_ERROR(vkResetFences(m_device, 1, &m_fence));
}


void Engine::cleanup() const
{

    vkDeviceWaitIdle(m_device);

    destroy_swapchain();

    vkDestroySemaphore(m_device, m_render_semaphore, nullptr);
    vkDestroySemaphore(m_device, m_swapchain_semaphore, nullptr);
    vkDestroyFence(m_device, m_fence, nullptr);

    vkDestroyCommandPool(m_device, m_cmd_pool, nullptr);

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_debug_utils_messenger(m_instance, m_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);


}



int main() {

  Engine engine = Engine(1280, 1024, "Engine");
  engine.init();
  engine.update();
  engine.cleanup();

  return 0;
}

