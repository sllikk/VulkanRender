#include "engine.h"
#include "VkBootstrap.h"


Engine::Engine(const uint32_t &width, const uint32_t &height, const std::string_view& title)
{



}


void Engine::init_vulkan() {


}


void Engine::init() {

    init_vulkan();


}


void Engine::update() {


}


void Engine::render() {

}


void Engine::cleanup() {


}



int main() {

  Engine* engine = new Engine(1280, 960, "Engine");
  engine->init();
  engine->update();
  engine->render();
  engine->cleanup();

  delete engine;
  return 0;
}

