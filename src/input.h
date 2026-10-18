#pragma once
#include "window.h"
#include <functional>
#include <vector>
#include <unordered_map>

enum class EKeyState : uint8_t {

   EPressed,
   EReleased,
   EToggle,
   ERepeat,

};

enum class EKeyContext {

   EGameContext,
   EUIContext,
   EEngineContext,

};


struct KeySatus {

   EKeyState current_state;
   EKeyContext context;

};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
   }
}

class InputManager {

   Window& m_window = Window::getInstance();

public:

   std::unordered_map<KeySatus, int> actions {};

};