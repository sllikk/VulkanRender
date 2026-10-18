#include "pipeline_manager.h"

#include <fstream>
#include <ios>
#include <ranges>
#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>


void ShaderManager::Load(VkDevice device, VkShaderStageFlagBits stage_flag, const std::string& shader_name)
{
    std::string shader_path = "compiled_shaders/" + shader_name;

    std::ifstream file(shader_path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    VkShaderModuleCreateInfo module_create_info{};
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize = buffer.size();
    module_create_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    module_create_info.pNext = nullptr;

    for (const auto& name : shaders | std::views::keys)
    {
        if (name == shader_name)
        {
            throw std::runtime_error("Create another name this shader name exist: " + shader_name);
        }
    }

    VkShaderModule shader_module = VK_NULL_HANDLE;

    const VkResult result =  vkCreateShaderModule(device, &module_create_info, nullptr, &shader_module);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(string_VkResult(result));
    }

    VkPipelineShaderStageCreateInfo stage_create_info{};
    stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_create_info.pNext = nullptr;
    stage_create_info.pName = "main";
    stage_create_info.module = shader_module;
    stage_create_info.stage = stage_flag;

    shaders[shader_name] = stage_create_info;

}

VkPipelineShaderStageCreateInfo ShaderManager::GetShaderStageByName(const std::string& shader_name) const
{

    for (auto shader : shaders)
    {
        if (shader.first == shader_name)
        {
            return shader.second;
        }
        else
        {
            throw std::runtime_error("This shader does not exist");
        }
    }

    return {};
}


PipelineManager::PipelineManager(const std::string_view& name, VkDevice device)
{
    m_name = name;
    m_device = device;
}
