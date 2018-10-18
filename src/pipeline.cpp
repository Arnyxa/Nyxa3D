#include "pipeline.hpp"
#include "util.hpp"


#include <iostream>
#include <fstream>
#include <string>

namespace ppr
{
	pipeline::pipeline(const vk::Device& a_device, const vk::Extent2D& aViewport)
		: m_device(a_device)
		, m_viewport(aViewport)
		, m_cleaned(false)
	{}

	pipeline::~pipeline()
	{}

	void pipeline::destroy() const
	{
		m_device.destroyPipeline(m_pipeline);
		m_device.destroyPipelineLayout(m_pipe_layout);
		m_device.destroyRenderPass(m_renderpass);
	}

	void pipeline::create()
	{
		if (!m_initialized)
			printf("Creating graphics pipeline...\n");
		else
			printf("Creating graphics pipeline...\n");

		const auto vert_shader_code = read_shader("shaders/vert.spv");
        const auto frag_shader_code = read_shader("shaders/frag.spv");

        const vk::ShaderModule vert_shader_module = create_shader_module(vert_shader_code);
        const vk::ShaderModule frag_shader_module = create_shader_module(frag_shader_code);

		printf("Shaders loaded.\n");

		printf("Initializing shader and pipeline info...\n");

        const vk::PipelineShaderStageCreateInfo vert_shader_info({}, vk::ShaderStageFlagBits::eVertex, vert_shader_module, "main");
        const vk::PipelineShaderStageCreateInfo frag_shader_info({}, vk::ShaderStageFlagBits::eFragment, frag_shader_module, "main");

        const vk::PipelineShaderStageCreateInfo shader_stages[] = { vert_shader_info, frag_shader_info };

        const auto binding = vertex::binding_descript();
        const auto attributes = vertex::attribute_descript();

        const vk::PipelineVertexInputStateCreateInfo vertex_inputinfo({}, 1, &binding, (uint32_t)attributes.size(), attributes.data());

        const vk::PipelineInputAssemblyStateCreateInfo input_assembly({}, vk::PrimitiveTopology::eTriangleList);

		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_viewport.width;
		viewport.height = (float)m_viewport.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_viewport;

        const vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

        const vk::PipelineRasterizationStateCreateInfo rasterizer({}, 
                                                                  false, false, 
                                                                  vk::PolygonMode::eFill, 
                                                                  vk::CullModeFlagBits::eBack, 
                                                                  vk::FrontFace::eClockwise, 
                                                                  0, 0.f, 0.f, 0.f, 
                                                                  1.f);

        const vk::PipelineMultisampleStateCreateInfo multisampling({}, 
                                                                   vk::SampleCountFlagBits::e1, 
                                                                   false, 
                                                                   1.0f);

		vk::PipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR 
                                              | vk::ColorComponentFlagBits::eG 
                                              | vk::ColorComponentFlagBits::eB 
                                              | vk::ColorComponentFlagBits::eA;
		color_blend_attachment.blendEnable = false;
		color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eOne;
		color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eZero;
		color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
		color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo color_blend_global;
		color_blend_global.logicOpEnable = false;
		color_blend_global.attachmentCount = 1;
		color_blend_global.pAttachments = &color_blend_attachment;

		printf("Creating pipeline layout...\n");

        const vk::PipelineLayoutCreateInfo layout_info;

		m_pipe_layout = m_device.createPipelineLayout(layout_info);

        const vk::GraphicsPipelineCreateInfo pipeline_info({}, 2, shader_stages, &vertex_inputinfo, &input_assembly, 
                                                           nullptr, &viewport_state, &rasterizer, &multisampling, 
                                                           nullptr, &color_blend_global, 
                                                           nullptr, m_pipe_layout, m_renderpass, 
                                                           0, vk::Pipeline(), -1);

		if (print(m_device.createGraphicsPipelines(vk::PipelineCache(), 1, &pipeline_info, nullptr, &m_pipeline)) != vk::Result::eSuccess)
			throw Error("Failed to create Vulkan graphics pipeline.", Error::Code::PIPELINE_CREATION_FAIL);

		printf("graphics pipeline successfully created.\n");

		m_device.destroyShaderModule(vert_shader_module);
		m_device.destroyShaderModule(frag_shader_module);

		if (!m_initialized)
			m_initialized = true;
	}

	vk::ShaderModule pipeline::create_shader_module(const std::vector<char>& a_bytecode) const
	{
		printf("Creating shader module...\n");

        const vk::ShaderModuleCreateInfo createinfo({}, 
                                                    a_bytecode.size(), 
                                                    reinterpret_cast<const uint32_t*>(a_bytecode.data()));
        return m_device.createShaderModule(createinfo);
	}

	std::vector<char> pipeline::read_shader(const std::string& a_filename)
	{
		// open file at its end to know the filesize
		std::ifstream shader(a_filename, std::ios::ate 
                                       | std::ios::binary);

		if (!shader.is_open())
			throw Error("Failed to open shader \"" 
                        + a_filename + "\"", 
                        Error::Code::SHADER_NOT_FOUND);

		// figure out file size based on buffer position
        const size_t file_size = (size_t)shader.tellg();

		printf(std::string(std::string(a_filename) 
                                     + " size: " 
                                     + std::to_string(file_size) 
                                     + "\n").c_str());

		std::vector<char> buffer(file_size);

		// reset buffer position and read all bytes at once
		shader.seekg(0);
		shader.read(buffer.data(), file_size);

		shader.close();

		return buffer;
	}

	vk::Pipeline& pipeline::get()
	{
		return m_pipeline;
	}

	vk::PipelineLayout& pipeline::get_layout()
	{
		return m_pipe_layout;
	}

	vk::RenderPass& pipeline::get_renderpass()
	{
		return m_renderpass;
	}
}
