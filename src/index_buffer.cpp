#include "index_buffer.hpp"

namespace ppr
{
    index_buffer::index_buffer(const vk::Device& a_device)
        : m_device(a_device)
        , m_buffer(a_device)
        , m_indices({0, 1, 2, 2, 3, 0})
    {}

    void index_buffer::create(const vk::PhysicalDevice& a_physical_device,
                              const vk::CommandPool& a_command_pool,
                              const vk::Queue& a_graphics_queue)
    {
        const vk::DeviceSize buffer_size = sizeof(m_indices[0]) * m_indices.size();

        buffer staging_buffer(m_device);
        auto staging_buffer_info = buffer::create_info(a_physical_device,
                                                       vk::MemoryPropertyFlagBits::eHostVisible
                                                       | vk::MemoryPropertyFlagBits::eHostCoherent,
                                                       vk::BufferUsageFlagBits::eTransferSrc,
                                                       buffer_size);
        staging_buffer.create(staging_buffer_info);

        auto memory_map = m_device.mapMemory(staging_buffer.memory(), 0, staging_buffer_info.data.size);
        memcpy(memory_map, m_indices.data(), staging_buffer_info.data.size);
        m_device.unmapMemory(staging_buffer.memory());

        auto index_buffer_info = buffer::create_info(a_physical_device,
                                                      vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                      vk::BufferUsageFlagBits::eTransferDst
                                                      | vk::BufferUsageFlagBits::eIndexBuffer,
                                                      buffer_size);
        m_buffer.create(index_buffer_info);

        auto buffer_copy_data = buffer::copy_data(m_buffer,
                                                  nullptr,
                                                  buffer_size,
                                                  a_command_pool,
                                                  a_graphics_queue);
        staging_buffer.copy(buffer_copy_data);

        staging_buffer.destroy();
    }

    void index_buffer::destroy() const
    {
        m_buffer.destroy();
    }

    const vk::Buffer& index_buffer::get() const
    {
        return m_buffer.get();
    }

    vk::Buffer& index_buffer::get_mut()
    {
        return m_buffer.get_mut();
    }

    const std::vector<uint16_t>& index_buffer::indices() const
    {
        return m_indices;
    }
}