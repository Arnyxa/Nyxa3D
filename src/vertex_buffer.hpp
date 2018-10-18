#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <array>

namespace ppr
{
	struct vertex
	{
	public:
        vertex(glm::vec2 a_position, 
               glm::vec3 a_color)
            : position(a_position)
            , color(a_color)
        {}
		const glm::vec2 position;
        const glm::vec3 color;

	public:
		static const vk::VertexInputBindingDescription binding_descript();
		static const std::array<vk::VertexInputAttributeDescription, 2> attribute_descript();
	};

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
