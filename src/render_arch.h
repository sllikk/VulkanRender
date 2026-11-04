#pragma once


#include "device.h"
#include "swapchain.h"
#include "resources.h"

class RenderEngine {

  Window& m_window = Window::getInstance();
  std::unique_ptr<VulkanContext> m_vulkan_context = nullptr;
  std::unique_ptr<Swap_chain> m_swap_chain = nullptr;
  std::array<std::unique_ptr<FrameInFlight>, FRAME_IN_FLIGHTS> m_frame_in_flights{};
  uint32_t frame_count = 0;
  uint32_t swap_chain_image_index = 0;
  FrameInFlight* m_current_frame_in_flight = nullptr;

  VkPipeline m_pipeline = nullptr;
  VkPipelineLayout m_pipeline_layout = nullptr;
  VkPipelineCache m_pipeline_cache = nullptr;

  std::unique_ptr<vk_resource::GPUBuffer<Vertex>> m_vertex_buffer = nullptr;
  std::unique_ptr<vk_resource::GPUBuffer<uint32_t>> m_index_buffer = nullptr;
  std::vector<uint32_t> m_indices = {};

  VkViewport m_viewport = {};
  VkRect2D  m_scissor = {};


public:

  RenderEngine(const RenderEngine& other) = delete;
  RenderEngine& operator=(const RenderEngine& other) = delete;
  RenderEngine(const RenderEngine&& other) = delete;
  RenderEngine&& operator=(const RenderEngine&& other) = delete;

  RenderEngine();

  void init();
  void init_pipeline();
  void update();
  void render();
  void destroy() const;


};
