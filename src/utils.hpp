#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_enum_string_helper.h>



#include <array>
#include <deque>
#include <queue>
#include <stack>
#include <vector>

#include <memory>
#include <functional>


constexpr uint32_t DOUBLE_BUFFERING = 2;
constexpr uint32_t TRIPLE_BUFFERING = 3;


struct DeletionQueue {

 std::deque<std::function<void()>> queue{};

  void flush()  {

    for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
        (*it)();
    }
    queue.clear();
  }

  void add_to_queue(std::function<void()>&& function) {
      queue.push_back(function);
  }


};

inline void THROW_IF_ERROR(const VkResult& result)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(string_VkResult(result));
    }
}

struct Vertex {

  glm::vec3 position;
  glm::vec4 color;

};

//
// This buffer uses for everything mem allocations
//

struct GpuBuffer {

  VkBuffer buffer = nullptr;
  VmaAllocation allocation = nullptr;

};


struct RenderItem {

  uint32_t vertices_count = 0;
  uint32_t vertices_start = 0;

  std::unique_ptr<GpuBuffer> vertex_buffer;
  std::unique_ptr<GpuBuffer> index_buffer;


};