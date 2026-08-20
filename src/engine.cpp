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

    // if you need an instance level extension
    if (system_info.is_extension_available("VK_KHR_get_physical_device_properties2")) {
        instance_builder.enable_extension("VK_KHR_get_physical_device_properties2");
    }

    if (system_info.is_extension_available("VK_LAYER_KHRONOS_validation")) {
        instance_builder.enable_extension("VK_LAYER_KHRONOS_validation");
    }

    THROW_IF_ERROR(glfwCreateWindowSurface(instance_ret.value(), m_window, nullptr, &m_surface));

    vkb::PhysicalDeviceSelector physical_device_selector(instance_ret.value());
    physical_device_selector.set_surface(m_surface);
    physical_device_selector.set_minimum_version(1, 3);
    // Engine cannot function without this extensions
    VkPhysicalDeviceFeatures required_features{};
    required_features.multiViewport = true;
    physical_device_selector.add_required_extension("VK_KHR_timeline_semaphore");
    physical_device_selector.add_required_extension("VK_KHR_synchronization2");
    physical_device_selector.set_required_features(required_features);

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

    m_instance = instance_ret.value();
    m_messenger = instance_ret.value().debug_messenger;
    m_gpu = dev_ret.value().physical_device;
    m_device = dev_ret.value();
    m_graphics_queue = graphics_queue_ret.value();
    m_compute_queue = graphics_queue_ret.value();

}


void Engine::init_swap_chain()
{

}


void Engine::init() {

    init_window();
    init_vulkan();


}


void Engine::update() {

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        render();
    }
}


void Engine::render() {


}


void Engine::cleanup() const
{

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_debug_utils_messenger(m_instance, m_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);


}



int main() {

  Engine* engine = new Engine(1280, 1024, "Engine");
  engine->init();
  engine->update();
  engine->cleanup();

  delete engine;
  return 0;
}

