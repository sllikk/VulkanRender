#include "render_arch.h"
#include "pipeline_manager.h"
#include "resources.h"



RenderEngine::RenderEngine() {

  m_window.set_window_extent(1980, 1024);
  m_window.init();
  m_vulkan_context = std::make_unique<VulkanContext>();
  m_swap_chain = std::make_unique<Swap_chain>(m_vulkan_context.get());

  for (int i = 0; i < FRAME_IN_FLIGHTS; i++) {
    m_frame_in_flights[i] = std::make_unique<FrameInFlight>(m_vulkan_context.get());
  }

  m_vertex_buffer = std::make_unique<vk_resource::GPUBuffer<Vertex>>(m_vulkan_context.get(), vk_resource::EBufferType::VertexBuffer);
  m_index_buffer = std::make_unique<vk_resource::GPUBuffer<uint32_t>>(m_vulkan_context.get(), vk_resource::EBufferType::IndexBuffer_32U);

}

void RenderEngine::init() {

  m_vulkan_context->init();
  m_swap_chain->init();

  for (int i = 0; i < FRAME_IN_FLIGHTS; i++) {
      m_frame_in_flights[i]->init();
  }

  std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}
  };

  std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
  };

  m_indices  = std::move(indices);
  m_vertex_buffer->create(vertices.data(), vertices.size());
  m_index_buffer->create(m_indices.data(), m_indices.size());

  init_pipeline();

}


void RenderEngine::init_pipeline() {

  auto fragment_shader = VkUtils::load_shaders("compiled_shaders/fragment_shader.frag.spv");
  auto vertex_shader = VkUtils::load_shaders("compiled_shaders/vertex_shader.vert.spv");

  auto fragment_module_info = vk_pipeline_utils::shader_module_create_info(0, reinterpret_cast<const uint32_t*>(fragment_shader.data()),  fragment_shader.size());
  auto vertex_module_info = vk_pipeline_utils::shader_module_create_info(0, reinterpret_cast<const uint32_t*>(vertex_shader.data()),  vertex_shader.size());

  VkShaderModule vertex_module = nullptr;
  VkShaderModule fragment_module = nullptr;

  THROW_IF_ERROR(vkCreateShaderModule(m_vulkan_context->get_device(), &vertex_module_info, nullptr, &vertex_module));
  THROW_IF_ERROR(vkCreateShaderModule(m_vulkan_context->get_device(), &fragment_module_info, nullptr, &fragment_module));

  auto vertex_stage_info = vk_pipeline_utils::shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_module, "main");
  auto fragment_stage_info = vk_pipeline_utils::shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_module, "main");

  const VkPipelineShaderStageCreateInfo shaders_stages[] = { vertex_stage_info, fragment_stage_info };

  auto pipeline_layout_info = vk_pipeline_utils::pipeline_layout_create_info(0, nullptr, nullptr, 0, 0);

  THROW_IF_ERROR(vkCreatePipelineLayout(m_vulkan_context->get_device(), &pipeline_layout_info, nullptr, &m_pipeline_layout));

  VkDynamicState states[] = { VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT, VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT };
  auto attribute_description = Vertex::attribute_description();
  auto binding_description = Vertex::binding_description();
  auto format_color = m_swap_chain->get_format();

  auto rendering_info = vk_pipeline_utils::pipeline_rendering_create_info(&format_color,  1, {}, {});
  auto assembly_info = vk_pipeline_utils::assembly_state_create_info(0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  auto dynamic_info = vk_pipeline_utils::dynamic_state_create_info(0, states, std::size(states));
  auto color_blend_attachment = vk_pipeline_utils::color_blend_attachment_state({}, {}, false, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, {}, {}, {}, {});
  auto color_blend_info = vk_pipeline_utils::color_blend_state_create_info(&color_blend_attachment, 1, nullptr, 0, VK_LOGIC_OP_NO_OP, false);
  auto depth_stencil_info = vk_pipeline_utils::depth_stencil_state_create_info(0, vk_pipeline_utils::EDepthTestMode::None, VK_COMPARE_OP_NEVER);
  auto rasterization_info = vk_pipeline_utils::rasterization_state_create_info(VK_FRONT_FACE_CLOCKWISE,  VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, false);

  auto vertex_input_info = vk_pipeline_utils::vertex_input_state_create_info(attribute_description.data(), attribute_description.size(), &binding_description, 1);
  auto viewport_info = vk_pipeline_utils::viewport_state_create_info(nullptr, nullptr, 0, 0);
  auto multisampling_info = vk_pipeline_utils::multisample_state_create_info();

  VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
  graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  graphics_pipeline_create_info.pNext = &rendering_info;
  graphics_pipeline_create_info.pStages = shaders_stages;
  graphics_pipeline_create_info.stageCount = 2;
  graphics_pipeline_create_info.layout = m_pipeline_layout;
  graphics_pipeline_create_info.pVertexInputState = &vertex_input_info;
  graphics_pipeline_create_info.pInputAssemblyState = &assembly_info;
  graphics_pipeline_create_info.pDynamicState = &dynamic_info;
  graphics_pipeline_create_info.pColorBlendState = &color_blend_info;
  graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_info;
  graphics_pipeline_create_info.pRasterizationState = &rasterization_info;
  graphics_pipeline_create_info.pViewportState = &viewport_info;
  graphics_pipeline_create_info.pMultisampleState = &multisampling_info;
  graphics_pipeline_create_info.pTessellationState = nullptr;
  graphics_pipeline_create_info.renderPass = nullptr;
  graphics_pipeline_create_info.subpass = 0;

  THROW_IF_ERROR(vkCreateGraphicsPipelines(m_vulkan_context->get_device(), nullptr, 1, &graphics_pipeline_create_info, nullptr, &m_pipeline));


}


void RenderEngine::update() {

  while (!m_window.is_closed())
  {
      glfwPollEvents();
      m_swap_chain->update();
      render();

  }

}


void RenderEngine::render() {

  m_current_frame_in_flight = m_frame_in_flights[frame_count % FRAME_IN_FLIGHTS].get();

  VkFence fence = m_current_frame_in_flight->get_fence();
  VkCommandBuffer cmd = m_current_frame_in_flight->get_command_buffer();
  VkSemaphore render_semaphore = m_current_frame_in_flight->get_render_semaphore();
  VkSemaphore swap_chain_semaphore = m_current_frame_in_flight->get_swap_chain_semaphore();

  THROW_IF_ERROR(vkWaitForFences(m_vulkan_context->get_device(), 1, &fence, VK_TRUE, UINT64_MAX));
  THROW_IF_ERROR(vkResetFences(m_vulkan_context->get_device(), 1, &fence));


  VkResult image_res = vkAcquireNextImageKHR(m_vulkan_context->get_device(), m_swap_chain->get_swap_chain(), UINT64_MAX, swap_chain_semaphore, nullptr, &swap_chain_image_index);
  if (image_res == VK_ERROR_OUT_OF_DATE_KHR) {
      m_swap_chain->set_resized(true);
      return;
  }

  auto begin_buffer_info = VkUtils::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);

  THROW_IF_ERROR(vkResetCommandBuffer(cmd, 0));

  THROW_IF_ERROR(vkBeginCommandBuffer(m_current_frame_in_flight->get_command_buffer(), &begin_buffer_info));

  VkUtils::transition_image(cmd, m_swap_chain->m_images[swap_chain_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  VkRenderingAttachmentInfo rendering_attachment_info{};
  rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  rendering_attachment_info.clearValue.color = {0.2, 0.3, 0.5, 0.5};
  rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  rendering_attachment_info.imageView = m_swap_chain->m_image_views[swap_chain_image_index];
  rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  rendering_attachment_info.pNext= nullptr;

  VkRenderingInfo rendering_info{};
  rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering_info.pColorAttachments = &rendering_attachment_info;
  rendering_info.pDepthAttachment = nullptr;
  rendering_info.pStencilAttachment = nullptr;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.layerCount = 1;
  rendering_info.renderArea.extent.width = m_window.get_window_extent().m_window_width;
  rendering_info.renderArea.extent.height = m_window.get_window_extent().m_window_height;
  rendering_info.renderArea.offset.x = 0;
  rendering_info.renderArea.offset.y = 0;
  rendering_info.pNext = nullptr;

  vkCmdBeginRendering(cmd, &rendering_info);

  m_viewport.width = static_cast<float>(m_window.get_window_extent().m_window_width);
  m_viewport.height = static_cast<float>(m_window.get_window_extent().m_window_height);
  m_viewport.maxDepth = 1.0f;
  m_viewport.minDepth = 0;
  m_viewport.x = 0;
  m_viewport.y = 0;

  vkCmdSetViewportWithCount(cmd, 1, &m_viewport);

  m_scissor.extent.height = m_window.get_window_extent().m_window_height;
  m_scissor.extent.width = m_window.get_window_extent().m_window_width;
  m_scissor.offset.x = 0;
  m_scissor.offset.y = 0;

  vkCmdSetScissorWithCount(cmd, 1, &m_scissor);

  auto const& vertex = m_vertex_buffer->get_gpu_buffer();
  auto const& index = m_index_buffer->get_gpu_buffer();

  VkDeviceSize offset[]{0};
  vkCmdBindVertexBuffers(cmd, 0, 1,  &vertex, offset);
  vkCmdBindIndexBuffer(cmd, index, 0, m_index_buffer->index_type());

  vkCmdBindPipeline(cmd,  VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

  vkCmdDrawIndexed(cmd, static_cast<uint32_t>(m_indices.size()), 1,  0, 0, 0);

  vkCmdEndRendering(cmd);

  VkUtils::transition_image(cmd, m_swap_chain->m_images[swap_chain_image_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  THROW_IF_ERROR(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmd_submit_info = VkUtils::command_buffer_submit(cmd);
  VkSemaphoreSubmitInfo render_semaphore_submit_info = VkUtils::semaphore_submit_info(render_semaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
  VkSemaphoreSubmitInfo swap_chain_semaphore_submit_info = VkUtils::semaphore_submit_info(swap_chain_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);

  VkSubmitInfo2 submit_info2 = VkUtils::submit_info2(&cmd_submit_info, 1, 0,  &swap_chain_semaphore_submit_info, &render_semaphore_submit_info);

  THROW_IF_ERROR(vkQueueSubmit2(m_vulkan_context->get_graphics_queue(), 1, &submit_info2, fence));

  auto swapchain = m_swap_chain->get_swap_chain();
  VkPresentInfoKHR present_info_khr = VkUtils::present_info_khr(&swap_chain_image_index, &swapchain, &render_semaphore, 1, 1);

  VkResult present_result = vkQueuePresentKHR(m_vulkan_context->get_graphics_queue(), &present_info_khr);

  if (present_result == VK_ERROR_OUT_OF_DATE_KHR) {
      m_swap_chain->set_resized(true);
  }

  frame_count = frame_count + 1;


}


void RenderEngine::destroy() const {

  vkDeviceWaitIdle(m_vulkan_context->get_device());

  for (int i = 0; i < FRAME_IN_FLIGHTS; i++) {
      m_frame_in_flights[i]->destroy();
  }

  m_swap_chain->destroy();

  m_vulkan_context->destroy();
  m_window.destroy();

}



int main() {

  RenderEngine* render_engine = new RenderEngine();
  render_engine->init();
  render_engine->update();
  render_engine->destroy();
  delete render_engine;
}
