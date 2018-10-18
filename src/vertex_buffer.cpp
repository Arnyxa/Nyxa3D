#include "vertex_buffer.hpp"
#include "util.hpp"

namespace ppr
{
	vertex_buffer::vertex_buffer(const vk::Device& a_device) : m_device(a_device)
	{
		m_vertices.emplace_back(vertex({ { 0.0f, -0.5f }, { 0.4f, 0.7f, 0.3f } }));
		m_vertices.emplace_back(vertex({ { 0.5f,  0.5f }, { 0.4f, 0.3f, 0.1f } }));
		m_vertices.emplace_back(vertex({ {-0.5f,  0.5f }, { 0.1f, 0.2f, 0.8f } }));
	}

    void vertex_buffer::create(const vk::PhysicalDevice& a_physical_device)
    {
        const vk::BufferCreateInfo buffer_info({}, 
                                               sizeof(m_vertices[0]) * m_vertices.size(), 
                                               vk::BufferUsageFlagBits::eVertexBuffer, 
                                               vk::SharingMode::eExclusive);
        m_buffer = m_device.createBuffer(buffer_info);

        const auto memory_reqs = m_device.getBufferMemoryRequirements(m_buffer);
        const vk::MemoryAllocateInfo memory_alloc_info(memory_reqs.size, find_memory_type(memory_reqs.memoryTypeBits, 
                                                                                          vk::MemoryPropertyFlagBits::eHostVisible
                                                                                        | vk::MemoryPropertyFlagBits::eHostCoherent, 
                                                                                          a_physical_device));
        m_buffer_memory = m_device.allocateMemory(memory_alloc_info, nullptr);
        m_device.bindBufferMemory(m_buffer, m_buffer_memory, 0);

        void* memory_map = m_device.mapMemory(m_buffer_memory, 0, buffer_info.size);
        memcpy(memory_map, m_vertices.data(), (size_t)buffer_info.size);
        m_device.unmapMemory(m_buffer_memory);
    }

    void vertex_buffer::destroy() const
    {
        m_device.destroyBuffer(m_buffer);
        m_device.freeMemory(m_buffer_memory);
    }

    uint32_t vertex_buffer::find_memory_type(uint32_t a_typefilter, 
                                             vk::MemoryPropertyFlags a_properties,
                                       const vk::PhysicalDevice& a_physical_device) const
    {
        const auto memory_properties = a_physical_device.getMemoryProperties();

        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
        {
            // find index of suitable memory type by checking if the corresponding bit is set to 1
            // also need to check if memory is suitable for writing to through property flags
            if ((a_typefilter & (1 << i) 
             && (memory_properties.memoryTypes[i].propertyFlags & a_properties) 
             == a_properties))
                return i;
        }

        throw Error("Failed to find suitable memory type.", Error::Code::BUFFER_MEMORY_TYPE_UNSUITABLE);
    }

    vk::Buffer& vertex_buffer::get()
    {
        return m_buffer;
    }

    std::vector<vertex>& vertex_buffer::get_vertex_array()
    {
        return m_vertices;
    }
}