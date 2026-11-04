#include "engine.h"

#include "device.h"
#include "swapchain.h"

Engine::Engine()  {

}


void Engine::init() const {

  m_setting.init();
  m_window.set_window_extent(m_setting.get_engine_setting_data().window_extent.m_window_width, m_setting.get_engine_setting_data().window_extent.m_window_height);
  m_window.init();
  m_vulkan_render.init();
  std::cout << &m_timer << std::endl;

}


void Engine::update() const {

  while (!m_window.is_closed())
  {
    glfwPollEvents();

    m_timer.update();
    m_vulkan_render.update(m_timer);

  }


}


void Engine::destroy() const {

  m_vulkan_render.cleanup();
  m_window.destroy();


}





