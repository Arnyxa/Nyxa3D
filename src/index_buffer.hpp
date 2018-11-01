#pragma once
#include "buffer.hpp"

#include <vector>

namespace ppr
{
    class index_buffer
    {
    public:
        explicit index_buffer(const vk::Device& a_device);

		void create(const vk::PhysicalDevice& a_physical_device,
                    const vk::CommandPool& a_command_pool,
                    const vk::Queue& a_graphics_queue);
        void destroy() const;

        vk::Buffer& get_mut();
        const vk::Buffer& get() const;

        const std::vector<uint16_t>& indices() const;

    private:
        const vk::Device& m_device;
        const std::vector<uint16_t> m_indices;

        buffer m_buffer;
    };
}