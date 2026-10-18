#pragma once

#include "window.h"
#include "timer.h"
#include "render.h"

class Engine {

  std::unique_ptr<Render> m_vulkan_render;
  Window& m_window = Window::getInstance();
  Timer& m_timer = Timer::getInstance();

public:

  Engine(const Engine& other ) = delete;
  Engine& operator=(const Engine& other ) = delete;
  Engine();

  void init() const;
  void update() const;
  void destroy() const;

};


