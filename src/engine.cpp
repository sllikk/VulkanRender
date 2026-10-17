#include "engine.h"

Engine::Engine()  {

   m_vulkan_render = std::make_unique<Render>();
}

void Engine::init() const {

  m_window.set_window_extent(1200, 720);
  m_window.init();
  m_vulkan_render->init();
  std::cout << &m_timer << std::endl;

}

void Engine::update() const {

  while (!m_window.is_closed())
  {
    glfwPollEvents();

    m_timer.update();

    m_vulkan_render->update(m_timer);

  }


}

void Engine::destroy() const {

  m_vulkan_render->cleanup();
  m_window.destroy();


}

int main() {

  const Engine* engine = new Engine();
  engine->init();
  engine->update();
  engine->destroy();
  delete engine;
}


