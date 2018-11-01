#pragma once

#include <vulkan/vulkan.hpp>
namespace ppr
{
    class buffer
    {
    public:
        struct create_info;
        struct copy_data;

        buffer(const vk::Device& a_device);

        void create(const vk::PhysicalDevice& a_physical_device,
                    const vk::BufferCreateInfo a_buffer_info,
                    const vk::MemoryPropertyFlags a_properties);

        void create(const create_info a_create_info);

        uint32_t find_memory_type(const     uint32_t a_typefilter,
                                        vk::MemoryPropertyFlags a_properties,
                                  const vk::PhysicalDevice& a_physical_device) const;

        vk::Buffer& get_mut();
        const vk::Buffer& get() const;
        const vk::DeviceMemory& memory() const;

        void copy(const copy_data& a_buffer_data) const;

        void destroy() const;

    private:
        const vk::Device& m_device;
        
        vk::Buffer m_buffer;
        vk::DeviceMemory m_buffer_memory;

    public:
        struct copy_data
        {
            copy_data(const     buffer& a_dst,
                      const vk::Device* const a_device,
                      const vk::DeviceSize& a_size,
                      const vk::CommandPool& a_pool,
                      const vk::Queue& a_queue)
                : dst_buffer(a_dst.get())
                , device(a_device)
                , size(a_size)
                , cmd_pool(a_pool)
                , queue(a_queue)
            {}

            const vk::Buffer& dst_buffer;
            const vk::Device* const device;
            const vk::DeviceSize& size;
            const vk::CommandPool& cmd_pool;
            const vk::Queue& queue;
        };

        struct create_info
        {
            create_info(const vk::PhysicalDevice& a_physical_device,
                        const vk::MemoryPropertyFlags a_mem_properties,
                        const vk::BufferUsageFlags a_usage = vk::BufferUsageFlags(),
                        const vk::DeviceSize a_size = 0,
                        const vk::BufferCreateFlags a_buffer_create_flags = vk::BufferCreateFlags(),
                        const vk::SharingMode a_sharing_mode = vk::SharingMode::eExclusive,
                        const uint32_t a_queue_index_count = 0,
                        const uint32_t* const a_queue_indices = nullptr)
                : device(a_physical_device)
                , data(a_buffer_create_flags,
                    a_size,
                    a_usage,
                    a_sharing_mode,
                    a_queue_index_count,
                    a_queue_indices)
                , mem_properties(a_mem_properties)
            {}

            create_info(const vk::PhysicalDevice& a_physical_device,
                        const vk::MemoryPropertyFlags a_mem_properties,
                        const vk::BufferCreateInfo a_create_info = vk::BufferCreateInfo())
                : device(a_physical_device)
                , data(a_create_info)
                , mem_properties(a_mem_properties)
            {}

            const vk::PhysicalDevice& device;
            const vk::BufferCreateInfo data;
            const vk::MemoryPropertyFlags mem_properties;
        };
    };
}
