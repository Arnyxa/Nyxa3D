#pragma once
#include "vertex.hpp"
#include "buffer.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

namespace ppr
{
	class vertex_buffer
	{
	public:
		explicit vertex_buffer(const vk::Device& a_physical_device);

		void create(const vk::PhysicalDevice& a_physical_device,
                    const vk::CommandPool& a_command_pool,
                    const vk::Queue& a_graphics_queue);

		void destroy() const;

        vk::Buffer& get_mut();
        const vk::Buffer& get() const;
        std::vector<vertex>& get_vertex_array();

	private:
		const vk::Device& m_device;

		buffer m_buffer;
        std::vector<vertex> m_vertices;
	};

}
