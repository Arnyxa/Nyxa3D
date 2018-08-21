#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

namespace nx
{
	class Pipeline
	{
	public:
		Pipeline(const vk::Device& aDevice, const vk::Extent2D& aViewportSize);
		~Pipeline();

		void Create();
		void Destroy();

		vk::Pipeline& GetRef();
		vk::RenderPass& GetRenderPass();
		vk::PipelineLayout& GetLayout();

		vk::ShaderModule CreateShaderModule(const std::vector<char>& aByteCode);
		static std::vector<char> ReadShader(const std::string& aFileName);

	private:
		const vk::Device& mDevice;
		const vk::Extent2D& mViewport;

		vk::Pipeline mPipeline;
		vk::RenderPass mRenderPass;
		vk::PipelineLayout mPipelineLayout;
	};
}