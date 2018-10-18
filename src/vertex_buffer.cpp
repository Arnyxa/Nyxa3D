#include "vertex_buffer.hpp"
#include "util.hpp"

// vertex
namespace ppr
{
	vk::VertexInputBindingDescription vertex::binding_descript()
	{
		return vk::VertexInputBindingDescription(0, sizeof(vertex), vk::VertexInputRate::eVertex);
	}

	std::array<vk::VertexInputAttributeDescription, 2> vertex::attribute_descript()
	{
		std::array<vk::VertexInputAttributeDescription, 2> myAttributes;

		myAttributes[0].binding = 0;
		myAttributes[0].location = 0;
		myAttributes[0].format = vk::Format::eR32G32Sfloat;
		myAttributes[0].offset = offsetof(vertex, pos);

		myAttributes[1].binding = 0;
		myAttributes[1].location = 1;
		myAttributes[1].format = vk::Format::eR32G32B32Sfloat;
		myAttributes[1].offset = offsetof(vertex, color);

		return myAttributes;
	}
}

// vertex_buffer
namespace ppr
{
	vertex_buffer::vertex_buffer(const vk::Device& a_device) : m_device(a_device)
	{
		m_vertices.push_back(vertex({ { 0.0f, -0.5f }, { 0.4f, 0.7f, 0.3f } }));
		m_vertices.push_back(vertex({ { 0.5f, 0.5f }, { 0.4f, 0.3f, 0.1f } }));
		m_vertices.push_back(vertex({ { -0.5f, 0.5f }, { 0.1f, 0.2f, 0.8f } }));
	}

	uint32_t vertex_buffer::find_memory_type(uint32_t a_typefilter, vk::MemoryPropertyFlags a_properties, const vk::PhysicalDevice& a_physical_device)
	{
		vk::PhysicalDeviceMemoryProperties myMemProperties = a_physical_device.getMemoryProperties();

		for (uint32_t i = 0; i < myMemProperties.memoryTypeCount; ++i)
		{
			// find index of suitable memory type by checking if the corresponding bit is set to 1
			// also need to check if memory is suitable for writing to through property flags
			if ((a_typefilter & (1 << i) && (myMemProperties.memoryTypes[i].propertyFlags & a_properties) == a_properties))
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

	void vertex_buffer::create(const vk::PhysicalDevice& a_physical_device)
	{
		vk::BufferCreateInfo myBufferInfo({}, sizeof(m_vertices[0]) * m_vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		m_buffer = m_device.createBuffer(myBufferInfo);

		vk::MemoryRequirements myMemReqs = m_device.getBufferMemoryRequirements(m_buffer);
		vk::MemoryAllocateInfo myAllocInfo(myMemReqs.size, find_memory_type(myMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, a_physical_device));

		m_buffer_memory = m_device.allocateMemory(myAllocInfo, nullptr);
		m_device.bindBufferMemory(m_buffer, m_buffer_memory, 0);

		void* myData = m_device.mapMemory(m_buffer_memory, 0, myBufferInfo.size);
		memcpy(myData, m_vertices.data(), (size_t)myBufferInfo.size);
		m_device.unmapMemory(m_buffer_memory);
	}

	void vertex_buffer::destroy()
	{
		m_device.destroyBuffer(m_buffer);
		m_device.freeMemory(m_buffer_memory);
	}
}