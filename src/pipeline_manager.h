#pragma once
#include <vulkan/vulkan.h>



class PipelineManager
{
    VkPipeline m_pipelineObject;

public:

    PipelineManager(const PipelineManager& other) = delete;
    PipelineManager operator=(const PipelineManager& other) = delete;


};