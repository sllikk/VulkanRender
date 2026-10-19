#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <iostream>
#include <stdexcept>

struct WindowExtent {

  uint32_t m_window_width = 0;
  uint32_t m_window_height = 0;

};

// singleton class one instance for everything
class Window {

  GLFWwindow* m_window = nullptr;
  WindowExtent m_window_extent = {};

public:

  static Window& getInstance() {
      static Window* pWindow = new Window();
          return *pWindow;
  }

  void init() {

    if (glfwInit() && glfwVulkanSupported()) {
        std::cout << "GLFW initialized" << std::endl;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_window_extent.m_window_width, m_window_extent.m_window_height, "Engine", nullptr, nullptr);

    if (m_window == nullptr) {
        throw std::runtime_error("Window was not created");
    }
    else {
        std::cout << "Window created" << std::endl;
    }

  }

  bool is_closed() const {
      if (glfwWindowShouldClose(m_window)) {
          return true;
      }

    return false;

  }

  void get_size() {

      int w, h;
      glfwGetWindowSize(m_window, &w, &h);
      m_window_extent.m_window_width = static_cast<uint32_t>(w);
      m_window_extent.m_window_height = static_cast<uint32_t>(h);
  }

  void destroy() const {
      glfwTerminate();
      glfwDestroyWindow(m_window);
  }

public:

  inline  GLFWwindow* get_window_ptr() const { return m_window; }
  inline  WindowExtent get_window_extent() const { return m_window_extent; }
  inline  void set_window_extent(const uint32_t& width, const uint32_t& height) { m_window_extent.m_window_width = width; m_window_extent.m_window_height = height; }

};