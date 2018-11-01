#include "vertex_buffer.hpp"
#include "util.hpp"
#include "logger.hpp"

namespace ppr
{
	vertex_buffer::vertex_buffer(const vk::Device& a_device) 
        : m_device(a_device)
        , m_buffer(a_device)
	{
		m_vertices.emplace_back(vertex({ { 0.0f, -0.5f }, { 0.4f, 0.7f, 0.3f } }));
		m_vertices.emplace_back(vertex({ { 0.5f,  0.5f }, { 0.4f, 0.3f, 0.1f } }));
		m_vertices.emplace_back(vertex({ {-0.5f,  0.5f }, { 0.1f, 0.2f, 0.8f } }));
	}

    void vertex_buffer::create(const vk::PhysicalDevice& a_physical_device,
                               const vk::CommandPool& a_command_pool,
                               const vk::Queue& a_graphics_queue)
    {
        const vk::DeviceSize buffer_size = sizeof(m_vertices[0]) * m_vertices.size();

        buffer staging_buffer(m_device);
        auto staging_buffer_info = buffer::create_info(a_physical_device, 
                                                       vk::MemoryPropertyFlagBits::eHostVisible
                                                       | vk::MemoryPropertyFlagBits::eHostCoherent,
                                                       vk::BufferUsageFlagBits::eTransferSrc,
                                                       buffer_size);
        staging_buffer.create(staging_buffer_info);

        auto memory_map = m_device.mapMemory(staging_buffer.memory(), 0, staging_buffer_info.data.size);
            memcpy(memory_map, m_vertices.data(), staging_buffer_info.data.size);
        m_device.unmapMemory(staging_buffer.memory());

        auto vertex_buffer_info = buffer::create_info(a_physical_device, 
                                                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                      vk::BufferUsageFlagBits::eTransferDst
                                                      | vk::BufferUsageFlagBits::eVertexBuffer,
                                                      buffer_size);
        m_buffer.create(vertex_buffer_info);

        auto buffer_copy_data = buffer::copy_data(m_buffer, 
                                                  nullptr, 
                                                  buffer_size, 
                                                  a_command_pool, 
                                                  a_graphics_queue);
        staging_buffer.copy(buffer_copy_data);

        staging_buffer.destroy();
    }

    void vertex_buffer::destroy() const
    {
        m_buffer.destroy();
    }

    const vk::Buffer& vertex_buffer::get() const
    {
        return m_buffer.get();
    }

    vk::Buffer& vertex_buffer::get_mut()
    {
        return m_buffer.get_mut();
    }

    std::vector<vertex>& vertex_buffer::get_vertex_array()
    {
        return m_vertices;
    }
}