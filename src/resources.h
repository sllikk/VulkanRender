#pragma once

#include <vma/vk_mem_alloc.h>

constexpr bool SHOW_INDEX_SETTING = false;


class VulkanContext;

namespace vk_resource {

enum class EBufferType : uint8_t {

  None = 0,
  VertexBuffer,
  IndexBuffer_16U,
  IndexBuffer_32U,
  UniformBuffer,

};


enum class ETextureType : uint8_t {

  None = 0,
  Texture2D,
  Texture3D,
  Sampler,

};


template<typename data_type>
class GPUBuffer {

  VulkanContext* m_vk_context = nullptr;
  VmaAllocation m_gpu_allocation = nullptr;
  VmaAllocation m_staging_allocation = nullptr;
  VkBuffer      m_gpu_buffer = nullptr;
  VkBuffer      m_staging_buffer = nullptr;
  data_type     m_data_type{};
  EBufferType   m_type = EBufferType::None;
  uint32_t      m_buffer_size = 0;

public:

  explicit GPUBuffer(VulkanContext* context, const EBufferType resourceType);

  ~GPUBuffer() = default;

  inline VkBuffer get_gpu_buffer() const {
    return m_gpu_buffer;
  }

  inline VkBuffer get_staging_buffer() const {
    return m_staging_buffer;
  }

  inline data_type get_data_type() const {
    return m_data_type;
  };

  inline EBufferType get_buffer_type() const {
      return m_type;
  }

  inline uint32_t get_buffer_size() const {
      return m_buffer_size;
  }

  inline VkIndexType index_type() const {

      VkIndexType type = VK_INDEX_TYPE_NONE_KHR;

      if (std::is_same_v<data_type, uint32_t>) {
        type = VK_INDEX_TYPE_UINT32;
      }

      if (std::is_same_v<data_type, uint16_t>) {
        type = VK_INDEX_TYPE_UINT16;
      }

      if (!std::is_same_v<data_type, uint32_t> && !std::is_same_v<data_type, uint16_t>) {
          throw std::runtime_error("You have not index data type");
      }
      return type;
  }


  void create(data_type* data, uint32_t size);
  void destroy() const;

};


template <typename data_type>
vk_resource::GPUBuffer<data_type>::GPUBuffer(VulkanContext *context, const EBufferType resourceType)
{
    m_vk_context = context;
    m_type = resourceType;

  if (m_type == EBufferType::IndexBuffer_16U && m_type == EBufferType::IndexBuffer_16U) {

  }

}


template <typename data_type>
void vk_resource::GPUBuffer<data_type>::create(data_type* data, uint32_t size)
{
    if (!std::is_same_v<data_type, uint32_t> && m_type == EBufferType::IndexBuffer_32U) {
        throw std::runtime_error("incorrect data type: should be UINT32_T");
        return;
    }

    if (!std::is_same_v<data_type, uint16_t> && m_type == EBufferType::IndexBuffer_16U) {
        throw std::runtime_error("incorrect data type: should be UINT16_T");
        return;
    }

      const uint32_t queue_indices[] { m_vk_context->get_graphics_family_index() };
      m_buffer_size = size * sizeof(data_type);

      VkBufferCreateInfo gpu_buffer_create_info{};
      gpu_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      gpu_buffer_create_info.pNext = nullptr;
      gpu_buffer_create_info.size = m_buffer_size;
      gpu_buffer_create_info.pQueueFamilyIndices = queue_indices;
      gpu_buffer_create_info.queueFamilyIndexCount = ARRAY_SIZE(queue_indices);
      gpu_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


      VmaAllocationCreateInfo gpu_allocation_create_info{};
      gpu_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

      VkBufferCreateInfo staging_buffer_create_info{};
      staging_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      staging_buffer_create_info.pNext = nullptr;
      staging_buffer_create_info.size = m_buffer_size;
      staging_buffer_create_info.pQueueFamilyIndices = queue_indices;
      staging_buffer_create_info.queueFamilyIndexCount = ARRAY_SIZE(queue_indices);
      staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

      VmaAllocationCreateInfo staging_allocation_create_info{};
      staging_allocation_create_info.usage =  VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
      staging_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

      switch (m_type) {
      case EBufferType::None:
        throw std::runtime_error("buffer wasn't init");
        break;
      case EBufferType::VertexBuffer:
        gpu_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
      case EBufferType::IndexBuffer_16U:
        gpu_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      case EBufferType::IndexBuffer_32U:
        gpu_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
      case EBufferType::UniformBuffer:
        gpu_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;

      default:
        throw std::runtime_error("no comparable type buffer");
        break;
      }


      if (m_type == EBufferType::UniformBuffer) {
        THROW_IF_ERROR(vmaCreateBuffer(m_vk_context->get_allocator(), &staging_buffer_create_info, &staging_allocation_create_info, &m_staging_buffer, &m_staging_allocation, nullptr));
      }
      else {

        THROW_IF_ERROR(vmaCreateBuffer(m_vk_context->get_allocator(), &staging_buffer_create_info, &staging_allocation_create_info, &m_staging_buffer, &m_staging_allocation, nullptr));
        THROW_IF_ERROR(vmaCreateBuffer(m_vk_context->get_allocator(), &gpu_buffer_create_info, &gpu_allocation_create_info, &m_gpu_buffer, &m_gpu_allocation, nullptr));

      }


      void* mappedData = nullptr;
      THROW_IF_ERROR(vmaMapMemory(m_vk_context->get_allocator(), m_staging_allocation, &mappedData));
      memcpy(mappedData, data, m_buffer_size);

      // cuz we wanna update buffer in real time
      if (m_type != EBufferType::UniformBuffer) {
        vmaUnmapMemory(m_vk_context->get_allocator(), m_staging_allocation);

        m_vk_context->immediate_submit([this](const VkCommandBuffer cmd) {

            VkBufferCopy region{};
            region.size = m_buffer_size;
            region.dstOffset = 0;
            region.srcOffset = 0;

            vkCmdCopyBuffer(cmd, m_staging_buffer, m_gpu_buffer, 1, &region);
        });
      }

}


template <typename data_type>
void vk_resource::GPUBuffer<data_type>::destroy() const {

    if (m_type == EBufferType::UniformBuffer) {
        vmaUnmapMemory(m_vk_context->get_allocator(), m_staging_allocation);
        vmaDestroyBuffer(m_vk_context->get_allocator(), m_staging_buffer, m_staging_allocation);
        return;
    }

    else {
        vmaDestroyBuffer(m_vk_context->get_allocator(), m_staging_buffer, m_staging_allocation);
        vmaDestroyBuffer(m_vk_context->get_allocator(), m_gpu_buffer, m_gpu_allocation);

    }


}


}



