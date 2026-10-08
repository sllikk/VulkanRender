#define VMA_IMPLEMENTATION

#include "engine.h"
#include "VkBootstrap.h"


#include <iostream>

Engine::Engine(const uint32_t &width, const uint32_t &height, const std::string_view title)
    : m_window_title(title),  m_window_width(width), m_window_height(height)
{

    m_triangle_item = std::make_unique<RenderItem>();

}

void Engine::init_window()
{
    if (!glfwInit())
    {
        throw std::runtime_error("glfw is not init");
    }

    if (!glfwVulkanSupported())
    {
        throw std::runtime_error("glfw is not supporting VULKAN");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_window_width, m_window_height, m_window_title.data(), nullptr, nullptr);
    if (m_window != nullptr)
    {
        std::cout << "Window initialized\n";
    }

}



void Engine::init_vulkan()
{
    bool blsValidationError = false;
#if _DEBUG
    blsValidationError = true;
#    endif

    vkb::InstanceBuilder instance_builder{};
    auto instance_ret = instance_builder
                            .set_app_name("VkApp")
                            .set_engine_name("VulkanEngine")
                            .require_api_version(1,3, 0)
                            .request_validation_layers(blsValidationError)
                            .use_default_debug_messenger()
                            .build(); // build is always called last

    // simple error checking and helpful error messages
    if (!instance_ret) {
        std::cout << "Failed to create Vulkan instance. Error: {}\n" << instance_ret.error().message();
        return;
    }

    auto system_info_ret = vkb::SystemInfo::get_system_info();
    if (!system_info_ret)
    {
        std::cout << "Failed to create SystemInfo. Error: {}\n" << system_info_ret.error().message();
    }
    auto system_info = system_info_ret.value();

    THROW_IF_ERROR(glfwCreateWindowSurface(instance_ret.value(), m_window, nullptr, &m_surface));

    vkb::PhysicalDeviceSelector physical_device_selector(instance_ret.value());
    physical_device_selector.set_surface(m_surface);
    physical_device_selector.set_minimum_version(1, 3);
    // Engine cannot function without this extensions
    VkPhysicalDeviceFeatures required_features{};
    required_features.multiViewport = true;

    VkPhysicalDeviceVulkan11Features features1{.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    features1.multiview = true;

    VkPhysicalDeviceVulkan12Features features2{.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features2.timelineSemaphore = true;

    VkPhysicalDeviceVulkan13Features features3{.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features3.synchronization2 = true;
    features3.dynamicRendering = true;

    physical_device_selector.set_required_features_11(features1);
    physical_device_selector.set_required_features_12(features2);
    physical_device_selector.set_required_features_13(features3);

    vkb::Result<vkb::PhysicalDevice> physical_device_selector_return = physical_device_selector.select();

    if (!physical_device_selector_return)
    {
        if (physical_device_selector_return.error() == vkb::PhysicalDeviceError::no_suitable_device)
        {
            const auto& reason = physical_device_selector_return.full_error();
            std::cerr << reason.type.message() << "\n";
            return;
        }
    }
    else
    {
        std::string name = physical_device_selector_return.value().name;
        std::cout << "GPU: " << name << "\n";
    }

    vkb::DeviceBuilder device_builder{physical_device_selector_return.value()};
    auto dev_ret = device_builder.build();
    if (!dev_ret)
    {
        throw std::runtime_error(dev_ret.error().message());
    }

    auto graphics_queue_ret = dev_ret.value().get_queue(vkb::QueueType::graphics);
    auto compute_queue_ret = dev_ret.value().get_queue(vkb::QueueType::compute);
    auto transfer_queue_ret = dev_ret.value().get_queue(vkb::QueueType::transfer);

    m_queue_graphics_family_index = dev_ret.value().get_queue_index(vkb::QueueType::graphics).value();
    m_queue_compute_family_index = dev_ret.value().get_queue_index(vkb::QueueType::compute).value();
    auto queue_transfer_index = dev_ret.value().get_queue_index(vkb::QueueType::transfer).value();

    m_instance = instance_ret.value();
    m_messenger = instance_ret.value().debug_messenger;
    m_gpu = dev_ret.value().physical_device;
    m_device = dev_ret.value();

    m_graphics_queue = graphics_queue_ret.value();
    m_compute_queue = compute_queue_ret.value();

    // Init vma
    VmaAllocatorCreateInfo vma_allocator_create_info{};
    vma_allocator_create_info.instance = m_instance;
    vma_allocator_create_info.device = m_device;
    vma_allocator_create_info.physicalDevice = m_gpu;
    vma_allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_3;

    THROW_IF_ERROR(vmaCreateAllocator(&vma_allocator_create_info, &m_vma_allocator));

    m_transfer_queue.init(m_device, transfer_queue_ret.value(), queue_transfer_index);

    m_main_deletion_queue.add_to_queue([&](
    )
    {
        vmaDestroyAllocator(m_vma_allocator);
        m_transfer_queue.destroy();
    });
}


void Engine::init_frame_resources()
{
    for (int i = 0; i < FRAME_IN_FLIGHTS; i++) {

        m_frame_contexts[i] = std::make_unique<FrameContext>(m_vma_allocator, &m_queue_graphics_family_index);
    }

}

void Engine::init_commands()
{
    const auto& cmd_pool_create_info = VkUtils::command_pool_create_info(m_queue_graphics_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_IN_FLIGHTS; i++)
    {
        THROW_IF_ERROR(vkCreateCommandPool(m_device, &cmd_pool_create_info, nullptr, &m_frame_contexts[i]->command_pool));
        const auto& cmd_allocation_create_info = VkUtils::command_buffer_allocate_info(m_frame_contexts[i]->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        THROW_IF_ERROR(vkAllocateCommandBuffers(m_device, &cmd_allocation_create_info, &m_frame_contexts[i]->command_buffer));
    }

}


void Engine::init_sync_objects()
{
    const auto& fence_create_info = VkUtils::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    const auto& semaphore_create_info = VkUtils::semaphore_create_info(0);

    for (int i = 0; i < FRAME_IN_FLIGHTS; i++)
    {
        THROW_IF_ERROR(vkCreateFence(m_device, &fence_create_info, nullptr, &m_frame_contexts[i]->fence));

        THROW_IF_ERROR(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_frame_contexts[i]->swapchain_semaphore));
        THROW_IF_ERROR(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_frame_contexts[i]->render_semaphore));

    }


}


void Engine::create_swapchain(const uint32_t width, const uint32_t height)
{
    vkb::SwapchainBuilder swapchain_builder {m_gpu, m_device, m_surface};
    VkSurfaceFormatKHR const& surface_format_khr{.format = m_swapchain_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    vkb::Swapchain swapchain = swapchain_builder.set_desired_format(surface_format_khr)
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(width, height)
    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    .build().value();

    m_window_width = swapchain.extent.width;
    m_window_height = swapchain.extent.height;

    m_swapchain = swapchain;
    m_swapchain_images = swapchain.get_images().value();
    m_swapchain_images_image_views = swapchain.get_image_views().value();

}


void Engine::resize()
{
    vkDeviceWaitIdle(m_device);

    destroy_swapchain();

    int h, w;
    glfwGetWindowSize(m_window, &w, &h);
    m_window_width = w;
    m_window_height = h;

    m_scissor.extent.width = m_window_width;
    m_scissor.extent.height = m_window_height;
    m_scissor.offset.x = 0;
    m_scissor.offset.y = 0;


    create_swapchain(m_window_width, m_window_height);
    isResized = false;

}


void Engine::destroy_swapchain() const
{
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    for (size_t i = 0; i < m_swapchain_images_image_views.size(); i++)
    {
        vkDestroyImageView(m_device, m_swapchain_images_image_views[i], nullptr);
    }
}

void Engine::init_descriptors()
{
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorCount = 1;
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_set_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
    descriptor_set_layout_create_info.pNext = nullptr;

    THROW_IF_ERROR( vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_create_info, nullptr, &m_descriptor_set_layout));

    constexpr VkDescriptorPoolSize descriptor_pool_size[] {
     {   .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, }
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.pPoolSizes = descriptor_pool_size;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    descriptor_pool_create_info.maxSets = 1;
    descriptor_pool_create_info.pNext = nullptr;

    THROW_IF_ERROR(vkCreateDescriptorPool(m_device, &descriptor_pool_create_info, nullptr, &m_descriptor_pool));

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pSetLayouts = &m_descriptor_set_layout;
    allocate_info.descriptorSetCount = 1;
    allocate_info.descriptorPool = m_descriptor_pool;
    allocate_info.pNext = nullptr;

    THROW_IF_ERROR(vkAllocateDescriptorSets(m_device, &allocate_info, &m_descriptor_set));


    VkDescriptorBufferInfo descriptor_buffer_info{};

    for (int i = 0; i < FRAME_IN_FLIGHTS; i++) {


        descriptor_buffer_info.range = m_frame_contexts[i]->m_main_pass_uniform_buffer->get_size();
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.buffer = m_frame_contexts[i]->m_main_pass_uniform_buffer->get_buffer();


    }
    VkWriteDescriptorSet write_descriptor_set{};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstSet = m_descriptor_set;
    write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
    write_descriptor_set.pNext = nullptr;

    vkUpdateDescriptorSets(m_device, 1, &write_descriptor_set, 0, nullptr);

    m_main_deletion_queue.add_to_queue([this]() {

        vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
        vkFreeDescriptorSets(m_device, m_descriptor_pool, 1, &m_descriptor_set);
        vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
    });

}


void Engine::init_vertex_buffer()
{
    std::vector<Vertex> vertices = {
        {glm::vec3(0.0f, -0.5f, 0.0f)},
        {glm::vec3(0.5f, 0.5f, 0.0f)},
        {glm::vec3(-0.5f, 0.5f, 0.0f)},
    };

    const VkDeviceSize size = static_cast<VkDeviceSize>(vertices.size() * sizeof(Vertex));

    // DEFAULT BUFFER FOR GPU
    VkBufferCreateInfo vertex_buffer_create_info{};
    vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_create_info.pNext = nullptr;
    vertex_buffer_create_info.size = size;
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo gpu_allocation_create_info{};
    gpu_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    gpu_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VmaAllocation gpuBufferAllocation = VK_NULL_HANDLE;

    THROW_IF_ERROR(vmaCreateBuffer(m_vma_allocator, &vertex_buffer_create_info, &gpu_allocation_create_info, &m_triangle_item->vertex_buffer, &gpuBufferAllocation, nullptr));

    const uint32_t arr[] = {m_transfer_queue.get_family_index() , m_queue_graphics_family_index};

    // Staging buffer
    VkBufferCreateInfo staging_buffer_create_info = vertex_buffer_create_info;
    staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_buffer_create_info.queueFamilyIndexCount = std::size(arr);
    staging_buffer_create_info.pQueueFamilyIndices = arr;
    staging_buffer_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;

    VmaAllocationCreateInfo staging_allocation_create_info{};
    staging_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    staging_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    THROW_IF_ERROR(vmaCreateBuffer(m_vma_allocator, &staging_buffer_create_info, &staging_allocation_create_info, &stagingBuffer, &stagingBufferAllocation, nullptr));

    m_main_deletion_queue.add_to_queue([=, this]()
    {
        vmaDestroyBuffer(m_vma_allocator, m_triangle_item->vertex_buffer, gpuBufferAllocation);
        vmaDestroyBuffer(m_vma_allocator, stagingBuffer, stagingBufferAllocation);
    });

    void* data;
    THROW_IF_ERROR(vmaMapMemory(m_vma_allocator, stagingBufferAllocation, &data));
    memcpy(data, vertices.data(), size);
    vmaUnmapMemory(m_vma_allocator, stagingBufferAllocation);

    m_transfer_queue.immediate_submit([&](const VkCommandBuffer cmd)
    {
        VkBufferCopy buffer_copy{};
        buffer_copy.size = size;
        vkCmdCopyBuffer(cmd, stagingBuffer, m_triangle_item->vertex_buffer, 1, &buffer_copy);
    });

}


void Engine::init_pipeline()
{
    auto fragment_shader = VkUtils::load_shaders("../compiled_shaders/fragment_shader.frag.spv");
    auto vertex_shader = VkUtils::load_shaders("../compiled_shaders/vertex_shader.vert.spv");

    VkShaderModuleCreateInfo vertex_shader_module_info{};
    vertex_shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_shader_module_info.pNext = nullptr;
    vertex_shader_module_info.codeSize = vertex_shader.size();
    vertex_shader_module_info.pCode = reinterpret_cast<const uint32_t*>(vertex_shader.data());

    VkShaderModuleCreateInfo fragment_shader_module_info{};
    fragment_shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragment_shader_module_info.pNext = nullptr;
    fragment_shader_module_info.codeSize = fragment_shader.size();
    fragment_shader_module_info.pCode = reinterpret_cast<const uint32_t*>(fragment_shader.data());

    VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
    VkShaderModule fragment_shader_module = VK_NULL_HANDLE;

    THROW_IF_ERROR(vkCreateShaderModule(m_device, &vertex_shader_module_info, nullptr, &vertex_shader_module));
    THROW_IF_ERROR(vkCreateShaderModule(m_device, &fragment_shader_module_info, nullptr, &fragment_shader_module));

    m_main_deletion_queue.add_to_queue([=, this]()
    {
        vkDestroyShaderModule(m_device, vertex_shader_module, nullptr);
        vkDestroyShaderModule(m_device, fragment_shader_module, nullptr);
    });

    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_create_info.pNext = nullptr;
    vertex_shader_stage_create_info.module = vertex_shader_module;
    vertex_shader_stage_create_info.pName = "main";
    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_create_info.pNext = nullptr;
    fragment_shader_stage_create_info.module = fragment_shader_module;
    fragment_shader_stage_create_info.pName = "main";
    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    /// FOR DYNAMIC RENDERING
    VkPipelineRenderingCreateInfo rendering_create_info{};
    rendering_create_info.pNext = nullptr;
    rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rendering_create_info.colorAttachmentCount = 1;
    rendering_create_info.pColorAttachmentFormats = &m_swapchain_format;
    // depth need there!!

    VkDynamicState dynamic_states[] { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.pNext = nullptr;
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = 2;
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.pNext = nullptr;
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.pScissors = &m_scissor;
    viewport_state_create_info.pViewports = &m_viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.viewportCount = 1;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &colorBlendAttachment;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY; // Optional

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
    depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_create_info.pNext = nullptr;
    depth_stencil_state_create_info.stencilTestEnable = false;
    depth_stencil_state_create_info.depthBoundsTestEnable = false;
    depth_stencil_state_create_info.depthTestEnable = false;

    VkPipelineInputAssemblyStateCreateInfo assembly_state_create_info{};
    assembly_state_create_info.pNext = nullptr;
    assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // shit for layout(binding)
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.binding = 0;
    vertex_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attribute_description.offset = 0;
    vertex_input_attribute_description.location = 0;

    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_description.stride = sizeof(Vertex);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.pNext = nullptr;
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_input_attribute_description;
    vertex_input_state_create_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPushConstantRange range;
    range.size = sizeof(ObjectConstant);
    range.offset = 0;
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &m_descriptor_set_layout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
    pipelineLayoutInfo.pPushConstantRanges = &range; // Optional

    THROW_IF_ERROR(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipeline_layout));


    VkPipelineShaderStageCreateInfo stages[ ] = { vertex_shader_stage_create_info, fragment_shader_stage_create_info };
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = &rendering_create_info;
    graphics_pipeline_create_info.pStages = stages;
    graphics_pipeline_create_info.stageCount = 2;
    graphics_pipeline_create_info.layout = m_pipeline_layout;
    graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &assembly_state_create_info;
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisampling;
    graphics_pipeline_create_info.pRasterizationState = &rasterizer;
    //graphics_pipeline_create_info.pTessellationState = nullptr;

    graphics_pipeline_create_info.renderPass = nullptr;
    graphics_pipeline_create_info.subpass = 0;

    THROW_IF_ERROR(vkCreateGraphicsPipelines(m_device, nullptr, 1, &graphics_pipeline_create_info, nullptr, &m_graphics_pipeline));


    m_main_deletion_queue.add_to_queue([this]()
    {
        vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
        vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    });
}

void Engine::init_texture() {



}

void Engine::init() {

    init_window();
    init_vulkan();
    init_frame_resources();
    init_commands();
    init_sync_objects();
    create_swapchain(m_window_width, m_window_height);
    init_vertex_buffer();
    init_descriptors();
    init_pipeline();

}


void Engine::update() {

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        m_timer.update();
        m_object_constant.worldTransform = glm::rotate(glm::mat4(1), m_timer.GetCurrentTime(), glm::vec3(sin(m_timer.GetCurrentTime()), 1, 1));

        // update data ubo
//        m_main_pass_ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//        m_main_pass_ubo.proj =  glm::perspective(glm::radians(90.0f), (float)m_window_width / (float)m_window_height, 0.1f, 1000.0f);
//        m_main_pass_ubo.proj[1][1] *= -1;

//        m_main_pass_ubo.projView = m_main_pass_ubo.proj * m_main_pass_ubo.view;
        MainPassUbo ubo;
        ubo.proj =  glm::rotate(glm::mat4(1), m_timer.GetCurrentTime(), glm::vec3(m_timer.GetCurrentTime(), 0, 1));
        m_get_frame_context_index().m_main_pass_uniform_buffer->update(&ubo, sizeof(MainPassUbo));

        if (isResized == true)
        {
            resize();
        }

        render();


    }
}


void Engine::render()
{
    //wait until the GPU has finished rendering the last frame.
    THROW_IF_ERROR(vkWaitForFences(m_device, 1, &m_get_frame_context_index().fence, true, UINT64_MAX));
    THROW_IF_ERROR(vkResetFences(m_device, 1, &m_get_frame_context_index().fence));

    const VkResult frame_res = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_get_frame_context_index().swapchain_semaphore, nullptr, &m_swapchainIndex);
    if (frame_res ==  VK_ERROR_OUT_OF_DATE_KHR)
    {
        isResized = true;
        return;
    }

    const VkCommandBuffer& cmd = m_get_frame_context_index().command_buffer;

    THROW_IF_ERROR( vkResetCommandBuffer(cmd, 0));

    const auto& cmd_begin_info = VkUtils::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
    THROW_IF_ERROR(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    VkUtils::transition_image(cmd, m_swapchain_images[m_swapchainIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    VkRenderingAttachmentInfo rendering_attachment_info{};
    rendering_attachment_info.pNext= nullptr;
    rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    rendering_attachment_info.clearValue.color = {0.1, 0.2, 0.1, 1};
    rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rendering_attachment_info.imageView = m_swapchain_images_image_views[m_swapchainIndex];
    rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo renderingInfo = {
        .sType =  VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { .offset = { .x = 0, .y = 0 }, .extent = {.width = m_window_width, .height = m_window_height} },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &rendering_attachment_info,
        .pDepthAttachment = nullptr,
    };

    vkCmdBeginRendering(cmd, &renderingInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_window_width);
    viewport.height = static_cast<float>(m_window_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {m_window_width, m_window_height};
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    VkDeviceSize size[] = {0};

    vkCmdPushConstants(cmd, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ObjectConstant), &m_object_constant);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_descriptor_set, 0, nullptr);
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_triangle_item->vertex_buffer, size);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRendering(cmd);

    VkUtils::transition_image(cmd, m_swapchain_images[m_swapchainIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    THROW_IF_ERROR(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmd_submit_info{};
    cmd_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmd_submit_info.pNext = nullptr;
    cmd_submit_info.commandBuffer = cmd;
    cmd_submit_info.deviceMask = 0;

    VkSemaphoreSubmitInfo semaphore_submit_info_wait = {};
    semaphore_submit_info_wait.sType =  VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    semaphore_submit_info_wait.pNext = nullptr;
    semaphore_submit_info_wait.deviceIndex = 0;
    semaphore_submit_info_wait.semaphore = m_get_frame_context_index().swapchain_semaphore;
    semaphore_submit_info_wait.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
    semaphore_submit_info_wait.value = 0;

    VkSemaphoreSubmitInfo semaphore_submit_info_signal = {};
    semaphore_submit_info_signal.sType =  VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    semaphore_submit_info_signal.pNext = nullptr;
    semaphore_submit_info_signal.deviceIndex = 0;
    semaphore_submit_info_signal.semaphore = m_get_frame_context_index().render_semaphore;
    semaphore_submit_info_signal.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    semaphore_submit_info_signal.value = 0;


    VkSubmitInfo2 submit_info2 = {};
    submit_info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info2.pNext = nullptr;
    submit_info2.commandBufferInfoCount = 1;
    submit_info2.pCommandBufferInfos = &cmd_submit_info;
    submit_info2.signalSemaphoreInfoCount = 1;
    submit_info2.pSignalSemaphoreInfos = &semaphore_submit_info_signal;
    submit_info2.waitSemaphoreInfoCount = 1;
    submit_info2.pWaitSemaphoreInfos = &semaphore_submit_info_wait;

    THROW_IF_ERROR(vkQueueSubmit2(m_graphics_queue, 1, &submit_info2, m_get_frame_context_index().fence));

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.pImageIndices = &m_swapchainIndex;
    present_info.pSwapchains = &m_swapchain;
    present_info.swapchainCount = 1;
    present_info.pWaitSemaphores = &m_get_frame_context_index().render_semaphore;
    present_info.waitSemaphoreCount = 1;

    VkResult present_res = vkQueuePresentKHR(m_graphics_queue, &present_info);

    if (present_res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        isResized = true;
    }

    m_frame_number++;
}


void Engine::cleanup()
{
    vkDeviceWaitIdle(m_device);


    for (int i = 0; i < FRAME_IN_FLIGHTS; i++)
    {
        vkDestroyCommandPool(m_device, m_frame_contexts[i]->command_pool, nullptr);

        vkDestroyFence(m_device, m_frame_contexts[i]->fence, nullptr);
        vkDestroySemaphore(m_device, m_frame_contexts[i]->swapchain_semaphore, nullptr);
        vkDestroySemaphore(m_device, m_frame_contexts[i]->render_semaphore, nullptr);

        m_frame_contexts[i]->deletion_queue.flush();
    }

    destroy_swapchain();

    m_main_deletion_queue.flush();

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_debug_utils_messenger(m_instance, m_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    glfwDestroyWindow(m_window);


}



int main() {

  Engine engine = Engine(1280, 1024, "Engine");
  engine.init();
  engine.update();
  engine.cleanup();

  return 0;
}

