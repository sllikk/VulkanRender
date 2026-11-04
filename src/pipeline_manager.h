#pragma once
#include "utils.hpp"

#include <map>
#include <stdexcept>

#include <string>
#include <vector>
#include <vulkan/vulkan.h>


namespace vk_pipeline_utils {

    enum class EDepthTestMode {
        None,
        EnableTest,
        EnableTestWithWrite,
    };

    enum class EStencilTestMode {
        None,
    };

    inline VkPipelineRenderingCreateInfo pipeline_rendering_create_info(const VkFormat* color_attachment_formats, const uint32_t& color_attachment_count,  const VkFormat& depth_attachment_format, const VkFormat& stencil_attachment_format) {
        VkPipelineRenderingCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        info.pColorAttachmentFormats = color_attachment_formats;
        info.colorAttachmentCount = color_attachment_count;
        info.depthAttachmentFormat = depth_attachment_format;
        info.stencilAttachmentFormat = stencil_attachment_format;
        info.pNext = nullptr;
        return info;
    }

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

    inline VkPipelineShaderStageCreateInfo shader_stage_create_info(VkShaderStageFlagBits stage_flag, VkShaderModule shader_module, const std::string_view& entry_point_name) {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage =  stage_flag;
        info.module = shader_module;
        info.pName = entry_point_name.data();
        info.pNext = nullptr;
        return info;
    }

    inline VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info(const VkPipelineDepthStencilStateCreateFlags flags, const EDepthTestMode depthTestMode, const VkCompareOp depthCompareOp) {

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

    inline VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkFrontFace front_face, VkCullModeFlags cull_mode_flags, VkPolygonMode polygon_mode, bool rasterization_discard_enable) {
        VkPipelineRasterizationStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.cullMode = cull_mode_flags;
        info.polygonMode = polygon_mode;
        info.lineWidth = 1.0f;
        info.rasterizerDiscardEnable = rasterization_discard_enable;
        info.frontFace = front_face;
        info.depthBiasEnable = false;
        info.pNext = nullptr;

        return info;
    }

    inline VkPipelineColorBlendAttachmentState color_blend_attachment_state(VkBlendOp alphaBlendOp, VkBlendOp colorBlendOp , bool blsBlend, VkColorComponentFlags color_flag_bits, VkBlendFactor srcColor, VkBlendFactor dstColor, VkBlendFactor srcAlpha, VkBlendFactor dstAlpha) {
        VkPipelineColorBlendAttachmentState state{};
        state.alphaBlendOp = alphaBlendOp;
        state.blendEnable = blsBlend;
        state.colorWriteMask = color_flag_bits;
        state.colorBlendOp = colorBlendOp;
        state.srcColorBlendFactor = srcColor;
        state.dstColorBlendFactor = dstColor;
        state.srcAlphaBlendFactor = srcAlpha;
        state.dstAlphaBlendFactor = dstAlpha;
        return state;
    }

    inline VkPipelineColorBlendStateCreateInfo color_blend_state_create_info( const VkPipelineColorBlendAttachmentState* attachment_states, const uint32_t& attachment_count, float blendConstants[4], size_t blend_constant_size, VkLogicOp logic_op, bool blsLogicOp) {
        VkPipelineColorBlendStateCreateInfo info{};
        info.pAttachments = attachment_states;
        info.attachmentCount = attachment_count;
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info.logicOpEnable = logic_op;
        memcpy(info.blendConstants, blendConstants, blend_constant_size);
        info.logicOp = logic_op;
        info.pNext = nullptr;
        return info;
    }

    inline VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info(const VkVertexInputAttributeDescription* p_vertex_input_attributes, const uint32_t& vertex_attribute_count, const VkVertexInputBindingDescription* p_vertex_input_bindings, const uint32_t& vertex_binding_count)
    {
        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.flags = 0;
        info.pVertexAttributeDescriptions = p_vertex_input_attributes;
        info.pVertexBindingDescriptions = p_vertex_input_bindings;
        info.vertexAttributeDescriptionCount = vertex_attribute_count;
        info.vertexBindingDescriptionCount = vertex_binding_count;
        info.pNext = nullptr;
        return info;
    }

    inline VkPipelineViewportStateCreateInfo viewport_state_create_info(const VkViewport* viewports, const VkRect2D* scissors, const uint32_t& viewport_count, const uint32_t& scissor_count) {

        VkPipelineViewportStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        info.flags = 0;
        info.pNext = nullptr;
        info.pScissors = scissors;
        info.scissorCount = scissor_count;
        info.pViewports = viewports;
        info.viewportCount = viewport_count;
        return info;
    }

    inline VkPipelineLayoutCreateInfo pipeline_layout_create_info(const VkPushConstantRange* ranges, const uint32_t& range_count, const VkDescriptorSetLayout* set_layouts, const uint32_t& layout_count) {
        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.flags = 0;
        info.pPushConstantRanges = ranges;
        info.pushConstantRangeCount = range_count;
        info.pSetLayouts = set_layouts;
        info.setLayoutCount = layout_count;
        info.pNext = nullptr;

        return info;
    }

    inline VkPipelineMultisampleStateCreateInfo multisample_state_create_info() {
        VkPipelineMultisampleStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        info.sampleShadingEnable = VK_FALSE;
        info.pNext = nullptr;
        return info;
    }

}

