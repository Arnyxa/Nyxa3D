#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>
#include <array>

namespace nx
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

	class Pipeline
	{
	public:
		Pipeline(const vk::Device& aDevice,
				const vk::PhysicalDevice& aPhysDevice,
				const vk::Extent2D& aViewportSize);
		~Pipeline();

		void Create();
		void DestroyVertexBuffer();
		void DestroyPipeline();

		void UpdateVertexArray();

		vk::Pipeline& GetRef();
		vk::RenderPass& GetRenderPass();
		vk::PipelineLayout& GetLayout();

		vk::ShaderModule CreateShaderModule(const std::vector<char>& aByteCode);
		static std::vector<char> ReadShader(const std::string& aFileName);

		void CreateVertexBuffer();
		vk::Buffer& GetVertexBuffer();
		std::vector<Vertex>& GetVertexArray();

		uint32_t FindMemType(uint32_t aTypeFilter, vk::MemoryPropertyFlags aProperties);

	private:
		const vk::PhysicalDevice& mPhysDevice;
		const vk::Device& mDevice;
		const vk::Extent2D& mViewport;

		vk::Pipeline mPipeline;
		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;

		vk::Buffer mVertexBuffer;
		vk::DeviceMemory mVrtxBufferMemory;
		std::vector<Vertex> mVertices;

		bool mCleaned;
	};
}