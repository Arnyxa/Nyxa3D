#pragma once
#include "vertex_buffer.hpp"
#include "util.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace ppr
{
	class pipeline : public common_checks
	{
	public:
		pipeline(const vk::Device& a_device, const vk::Extent2D& a_viewport_size);
		~pipeline();

		void create();
		void destroy();

		vk::Pipeline& get();
		vk::RenderPass& get_renderpass();
		vk::PipelineLayout& get_layout();

		vk::ShaderModule create_shader_module(const std::vector<char>& a_bytecode);
		static std::vector<char> read_shader(const std::string& a_filename);

	private:
		const vk::Device& m_device;
		const vk::Extent2D& m_viewport;

		vk::Pipeline m_pipeline;
		vk::RenderPass m_renderpass;
		vk::PipelineLayout m_pipe_layout;

		bool m_cleaned;
	};
}