#include "FrameResources.h"



FrameContext::FrameContext(const VmaAllocator allocator, const uint32_t* queue_index)
{
    m_main_pass_uniform_buffer = std::make_unique<UniformBuffer<MainPassUbo>>(allocator, queue_index);

    deletion_queue.add_to_queue([this]() {
        m_main_pass_uniform_buffer->destroy();
    });

}