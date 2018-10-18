#pragma once
#include "vertex_buffer.hpp"
#include "util.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace ppr
{
	class Pipeline : public CommonChecks
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

		bool mCleaned;
	};
}