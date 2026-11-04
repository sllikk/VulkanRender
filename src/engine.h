#pragma once

#include "window.h"
#include "timer.h"
#include "render.h"
#include "settings.h"


class Engine {

  Render& m_vulkan_render  = Render::getInstance();
  Window& m_window = Window::getInstance();
  Timer& m_timer = Timer::getInstance();
  Setting& m_setting = Setting::get_instance();

public:

  Engine(const Engine& other ) = delete;
  Engine& operator=(const Engine& other ) = delete;
  Engine();

  void init() const;
  void update() const;
  void destroy() const;

};


