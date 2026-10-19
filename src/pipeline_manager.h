#pragma once
#include <map>
#include <stdexcept>

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

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

namespace vk_pipeline {

    enum class EDepthTestMode {
        None,
        EnableTest,
        EnableTestWithWrite,

    };

    enum class EStencilTestMode {
        None,
    };

    inline VkPipelineInputAssemblyStateCreateInfo assembly_state_create_info(const VkPipelineInputAssemblyStateCreateFlags flags,  const VkPrimitiveTopology topology) {

        VkPipelineInputAssemblyStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.flags = flags;
        info.topology = topology;
        info.pNext = nullptr;

        return info;
    }

    inline VkPipelineLayoutCreateInfo pipeline_layout_create_info(const VkPipelineLayoutCreateFlags flags, const VkPushConstantRange* pRange, const VkDescriptorSetLayout* pDescriptorSetLayout, const uint32_t layoutCount, const uint32_t rangeCount) {

        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.flags = flags;
        info.pPushConstantRanges = pRange;
        info.pushConstantRangeCount = rangeCount;
        info.pSetLayouts = pDescriptorSetLayout;
        info.setLayoutCount = layoutCount;
        info.pNext = nullptr;

        return info;
    }

    inline VkPipelineDynamicStateCreateInfo dynamic_state_create_info(const VkPipelineDynamicStateCreateFlags flags,  const VkDynamicState* pDynamicState, const uint32_t stateCount) {

        VkPipelineDynamicStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        info.pDynamicStates = pDynamicState;
        info.dynamicStateCount = stateCount;
        info.flags = flags;
        info.pNext = nullptr;
        return info;
    }

    inline VkShaderModuleCreateInfo shader_module_create_info(const VkShaderModuleCreateFlags flags, const uint32_t* pCode, const size_t codeSize) {

        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.flags = flags;
        info.pCode = pCode;
        info.codeSize = codeSize;
        info.pNext = nullptr;

        return info;
    }

    inline VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info(const VkPipelineDepthStencilStateCreateFlags flags, const EDepthTestMode depthTestMode, const VkCompareOp depthCompareOp, const EStencilTestMode stencilTestMode) {

        VkPipelineDepthStencilStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.flags = flags;

        switch (depthTestMode) {
        case EDepthTestMode::None:

            info.depthTestEnable = false;
            info.depthWriteEnable = false;
            info.depthBoundsTestEnable = false;
            break;

        case EDepthTestMode::EnableTest:

            info.depthTestEnable = true;
            info.depthWriteEnable = false;
            info.depthBoundsTestEnable = false;
            break;

        case EDepthTestMode::EnableTestWithWrite:
            info.depthTestEnable = true;
            info.depthWriteEnable = true;
            info.depthBoundsTestEnable = false;
            break;

        default:
            throw std::runtime_error("depth test mode error");
            break;
        }

        info.depthCompareOp = depthCompareOp;
        info.pNext = nullptr;
        return info;
    }

    inline VkPipelineRasterizationStateCreateInfo rasterization_state_create_info() {

    }

}

class PipelineManager
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    std::string_view m_name = "";



public:

    PipelineManager(const PipelineManager& rhs) = delete;
    PipelineManager& operator=(const PipelineManager& rhs) = delete;
    PipelineManager(const std::string_view& name, VkDevice device);

    std::map<std::string_view, VkPipeline> m_pipelines;

    void init(ShaderManager* shader_manager);
    void destroy();

};


