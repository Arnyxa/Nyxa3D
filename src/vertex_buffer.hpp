#pragma once
#include "vertex.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>

namespace ppr
{
	class vertex_buffer
	{
	public:
		explicit vertex_buffer(const vk::Device& a_physical_device);

		void create(const vk::PhysicalDevice& a_physical_device);
		void destroy() const;

		uint32_t find_memory_type(uint32_t a_typefilter, 
                              vk::MemoryPropertyFlags a_properties, 
                        const vk::PhysicalDevice& a_physical_device) const;

        vk::Buffer& get();
        std::vector<vertex>& get_vertex_array();

	private:
		const vk::Device& m_device;

		vk::Buffer m_buffer;
		vk::DeviceMemory m_buffer_memory;
		std::vector<vertex> m_vertices;
	};

}
