#include "window.h"

class Timer {

  float m_delta_time;
  float m_current_time;
  float m_prev;

public:

  Timer() {
    m_delta_time = 0.0f;
    m_prev = 0.0f;
    m_current_time = static_cast<float>(glfwGetTime());
  }

  Timer(const Timer& other) = delete;
  Timer operator=(const Timer& other) = delete;

  static Timer& getInstance() {
    static Timer* timer = new Timer();
    return *timer;
  };

  void update() {
    m_current_time = static_cast<float>(glfwGetTime());
    m_delta_time = m_current_time - m_prev;
    m_prev = m_current_time;
  }

  float GetCurrentTime() const {
    return m_current_time;
  }

  float GetDeltaTime() const {
    return m_delta_time;
  }

};
