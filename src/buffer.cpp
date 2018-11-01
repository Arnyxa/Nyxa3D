#include "buffer.hpp"
#include "logger.hpp"

namespace ppr
{
    buffer::buffer(const vk::Device& a_device)
        : m_device(a_device)
    {}

    void buffer::create(const vk::PhysicalDevice& a_physical_device,
                        const vk::BufferCreateInfo a_buffer_info,
                        const vk::MemoryPropertyFlags a_properties)
    {
        m_buffer = m_device.createBuffer(a_buffer_info);

        const auto memory_reqs = m_device.getBufferMemoryRequirements(m_buffer);
        const vk::MemoryAllocateInfo memory_alloc_info(memory_reqs.size, 
                                                       find_memory_type(memory_reqs.memoryTypeBits,
                                                       a_properties,
                                                       a_physical_device));

        m_buffer_memory = m_device.allocateMemory(memory_alloc_info, nullptr);
        m_device.bindBufferMemory(m_buffer, m_buffer_memory, 0);
    }

    void buffer::create(const create_info a_create_info)
    {
        m_buffer = m_device.createBuffer(a_create_info.data);

        const auto memory_reqs = m_device.getBufferMemoryRequirements(m_buffer);
        const vk::MemoryAllocateInfo memory_alloc_info(memory_reqs.size,
            find_memory_type(memory_reqs.memoryTypeBits,
                a_create_info.mem_properties,
                a_create_info.device));

        m_buffer_memory = m_device.allocateMemory(memory_alloc_info, nullptr);
        m_device.bindBufferMemory(m_buffer, m_buffer_memory, 0);
    }

    uint32_t buffer::find_memory_type(const     uint32_t a_typefilter,
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

        log->critical("Failed to find suitable memory type.");
        return 0;
    }

    vk::Buffer& buffer::get_mut()
    {
        return m_buffer;
    }

    const vk::Buffer& buffer::get() const
    {
        return m_buffer;
    }

    const vk::DeviceMemory& buffer::memory() const
    {
        return m_buffer_memory;
    }

    void buffer::copy(const copy_data& a_data) const
    {
        //we want to use a user-specified device if they specified one
        auto& physical_device = a_data.device == nullptr ? m_device : *a_data.device;

        const auto alloc_info = vk::CommandBufferAllocateInfo()
                                           .setLevel(vk::CommandBufferLevel::ePrimary)
                                           .setCommandPool(a_data.cmd_pool)
                                           .setCommandBufferCount(1);
        const auto cmd_buffer = physical_device.allocateCommandBuffers(alloc_info).at(0);

        const auto begin_info = vk::CommandBufferBeginInfo()
                                .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmd_buffer.begin(begin_info);

        const auto copy_region = vk::BufferCopy()
                                        .setSrcOffset(0)
                                        .setDstOffset(0)
                                        .setSize(a_data.size);
        cmd_buffer.copyBuffer(m_buffer, 
                              a_data.dst_buffer, 
                              copy_region);
        cmd_buffer.end();

        const auto submit_info = vk::SubmitInfo()
                                 .setCommandBufferCount(1)
                                 .setPCommandBuffers(&cmd_buffer);
        a_data.queue.submit(submit_info, nullptr);
        a_data.queue.waitIdle();

        physical_device.freeCommandBuffers(a_data.cmd_pool, cmd_buffer);
    }

    void buffer::destroy() const
    {
        if (m_buffer)
            m_device.destroyBuffer(m_buffer);
        if (m_buffer_memory)
            m_device.freeMemory(m_buffer_memory);
    }
}