#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <array>

namespace ppr
{
	struct Vertex
	{
	public:
		glm::vec2 Pos;
		glm::vec3 Color;

	public:
		static vk::VertexInputBindingDescription GetBindingDescript();
		static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescript();
	};

	class VertexBuffer
	{
	public:
		explicit VertexBuffer(const vk::Device& aPhysDevice);

		void Create(const vk::PhysicalDevice& aPhysicalDevice);
		void Destroy();

		vk::Buffer& Get();
		std::vector<Vertex>& GetVertexArray();

		uint32_t FindMemoryType(uint32_t aTypeFilter, vk::MemoryPropertyFlags aProperties, const vk::PhysicalDevice& aPhysicalDevice);

	private:
		const vk::Device& mDevice;

		vk::Buffer mBuffer;
		vk::DeviceMemory mBufferMemory;
		std::vector<Vertex> mVertices;
	};

}
