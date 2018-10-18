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
		glm::vec2 pos;
		glm::vec3 color;

	public:
		static vk::VertexInputBindingDescription binding_descript();
		static std::array<vk::VertexInputAttributeDescription, 2> attribute_descript();
	};

	class vertex_buffer
	{
	public:
		explicit vertex_buffer(const vk::Device& a_physical_device);

		void create(const vk::PhysicalDevice& a_physical_device);
		void destroy();

		vk::Buffer& get();
		std::vector<vertex>& get_vertex_array();

		uint32_t find_memory_type(uint32_t a_typefilter, vk::MemoryPropertyFlags a_properties, const vk::PhysicalDevice& a_physical_device);

	private:
		const vk::Device& m_device;

		vk::Buffer m_buffer;
		vk::DeviceMemory m_buffer_memory;
		std::vector<vertex> m_vertices;
	};

}
