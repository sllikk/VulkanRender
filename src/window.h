#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <assert.h>
#include <iostream>
#include <functional>


void window_size_callback(GLFWwindow* pwindow, int width, int height);

struct WindowExtent {

  uint32_t m_window_width = 0;
  uint32_t m_window_height = 0;

};

// singleton class one instance for everything
class Window {

  GLFWwindow* m_window = nullptr;
  GLFWmonitor* m_monitor = nullptr;
  WindowExtent m_window_extent = {};


public:

  static Window& getInstance() {
      static Window* pWindow = new Window();
          return *pWindow;
  }

  void init() {


    if (!glfwInit() || !glfwVulkanSupported()) {
      throw std::runtime_error("glfw initialization is fail");
    }

    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    for (int i = 0; i < count; i++) {
        std::cout << glfwGetMonitorName(monitors[i]) << std::endl;
        if (glfwGetPrimaryMonitor() == monitors[i]) {
            m_monitor =  monitors[i];
            break;
        }
    }

    const GLFWvidmode* mode = glfwGetVideoMode(m_monitor);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    //glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    m_window = glfwCreateWindow(m_window_extent.m_window_width, m_window_extent.m_window_height, "Engine", nullptr, nullptr);
    assert(m_window && "Window was not created");

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

  void resize() const {

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


inline void window_size_callback(GLFWwindow *pwindow, int width, int height) {

//  Window& window = Window::getInstance();
//  window.set_window_extent(width, height);
    std::cout << width << " :::::::: " << height << std::endl;

}