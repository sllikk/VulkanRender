#pragma once
#include <map>

#include <vulkan/vulkan.h>
#include <string>
#include <vector>


class ShaderManager
{

public:

    ShaderManager(const ShaderManager& rhs)= delete;
    ShaderManager& operator=(const ShaderManager& rhs) = delete;
    ShaderManager(const ShaderManager&& rhs) = delete;
    ShaderManager&& operator=(const ShaderManager&& rhs) = delete;

    ShaderManager() = default;

public:

    void Load(VkDevice device, VkShaderStageFlagBits stage_flag, const std::string& shader_name);

    VkPipelineShaderStageCreateInfo GetShaderStageByName(const std::string& shader_name) const;

    std::map<const std::string,  VkPipelineShaderStageCreateInfo> shaders{};


};


class PipelineManager
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    std::string_view m_name = "";

    VkVertexInputAttributeDescription m_vertex_attribute_description{};
    VkVertexInputBindingDescription m_vertex_binding_description{};
    VkPipelineInputAssemblyStateCreateInfo m_assembly_state_create_info{};
    VkPipelineLayoutCreateInfo m_pipeline_layout_create_info{};
    VkPipelineDynamicStateCreateInfo m_dynamic_state_create_info{};
    VkShaderModuleCreateInfo m_shader_module_create_info{};
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_state_create_info{};
    VkPipelineRasterizationStateCreateInfo m_rasterization_state_create_info{};

public:

    PipelineManager(const PipelineManager& rhs) = delete;
    PipelineManager& operator=(const PipelineManager& rhs) = delete;
    PipelineManager(const std::string_view& name, VkDevice device);

    std::map<std::string_view, VkPipeline> m_pipelines;

    void init(ShaderManager* shader_manager);
    void destroy();

};


