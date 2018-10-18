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

	void pipeline::destroy()
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

		auto myVertShaderCode = read_shader("shaders/vert.spv");
		auto myFragShaderCode = read_shader("shaders/frag.spv");

		vk::ShaderModule myVertShaderModule = create_shader_module(myVertShaderCode);
		vk::ShaderModule myFragShaderModule = create_shader_module(myFragShaderCode);

		printf("Shaders loaded.\n");

		printf("Initializing shader and pipeline info...\n");

		vk::PipelineShaderStageCreateInfo myVertShaderInfo({}, vk::ShaderStageFlagBits::eVertex, myVertShaderModule, "main");
		vk::PipelineShaderStageCreateInfo myFragShaderInfo({}, vk::ShaderStageFlagBits::eFragment, myFragShaderModule, "main");

		vk::PipelineShaderStageCreateInfo myShaderStages[] = { myVertShaderInfo, myFragShaderInfo };

		auto myBinding = vertex::binding_descript();
		auto myAttributes = vertex::attribute_descript();

		vk::PipelineVertexInputStateCreateInfo myVertexInputInfo({}, 1, &myBinding, (uint32_t)myAttributes.size(), myAttributes.data());

		vk::PipelineInputAssemblyStateCreateInfo myInputAssembly({}, vk::PrimitiveTopology::eTriangleList);

		vk::Viewport myViewport = {};
		myViewport.x = 0.0f;
		myViewport.y = 0.0f;
		myViewport.width = (float)m_viewport.width;
		myViewport.height = (float)m_viewport.height;
		myViewport.minDepth = 0.0f;
		myViewport.maxDepth = 1.0f;

		vk::Rect2D myScissor = {};
		myScissor.offset = { 0, 0 };
		myScissor.extent = m_viewport;

		vk::PipelineViewportStateCreateInfo myViewportState({}, 1, &myViewport, 1, &myScissor);

		vk::PipelineRasterizationStateCreateInfo myRasterizer({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);
		myRasterizer.lineWidth = 1.0f;

		vk::PipelineMultisampleStateCreateInfo myMultiSampling({}, vk::SampleCountFlagBits::e1, false, 1.0f);

		vk::PipelineColorBlendAttachmentState myColorBlendAttachment = {};
		myColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |vk::ColorComponentFlagBits::eB |vk::ColorComponentFlagBits::eA;
		myColorBlendAttachment.blendEnable = false;
		myColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
		myColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
		myColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		myColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		myColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		myColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo myColorBlendingGlobal;
		myColorBlendingGlobal.logicOpEnable = false;
		myColorBlendingGlobal.attachmentCount = 1;
		myColorBlendingGlobal.pAttachments = &myColorBlendAttachment;

		printf("Creating pipeline layout...\n");

		vk::PipelineLayoutCreateInfo myLayoutInfo;

		m_pipe_layout = m_device.createPipelineLayout(myLayoutInfo);

		vk::GraphicsPipelineCreateInfo myPipelineInfo({}, 2, myShaderStages, &myVertexInputInfo, &myInputAssembly, nullptr, &myViewportState, 
										&myRasterizer, &myMultiSampling, nullptr, &myColorBlendingGlobal, nullptr, m_pipe_layout, 
										m_renderpass, 0, vk::Pipeline(), -1);

		if (Print(m_device.createGraphicsPipelines(vk::PipelineCache(), 1, &myPipelineInfo, nullptr, &m_pipeline)) != vk::Result::eSuccess)
			throw Error("Failed to create Vulkan graphics pipeline.", Error::Code::PIPELINE_CREATION_FAIL);

		printf("graphics pipeline successfully created.\n");

		m_device.destroyShaderModule(myVertShaderModule);
		m_device.destroyShaderModule(myFragShaderModule);

		if (!m_initialized)
			m_initialized = true;
	}

	vk::ShaderModule pipeline::create_shader_module(const std::vector<char>& a_bytecode)
	{
		printf("Creating shader module...\n");

		vk::ShaderModuleCreateInfo createinfo({}, a_bytecode.size(), reinterpret_cast<const uint32_t*>(a_bytecode.data()));
		vk::ShaderModule myShaderModule = m_device.createShaderModule(createinfo);

		return myShaderModule;
	}

	std::vector<char> pipeline::read_shader(const std::string& a_filename)
	{
		// open file at its end to know the filesize
		std::ifstream myFile(a_filename, std::ios::ate | std::ios::binary);

		if (!myFile.is_open())
			throw Error("Failed to open shader \"" + a_filename + "\"", Error::Code::SHADER_NOT_FOUND);

		// figure out file size based on buffer position
		size_t myFileSize = (size_t)myFile.tellg();

		printf(std::string(std::string(a_filename) + " size: " + std::to_string(myFileSize) + "\n").c_str());

		std::vector<char> myBuffer(myFileSize);

		// reset buffer position and read all bytes at once
		myFile.seekg(0);
		myFile.read(myBuffer.data(), myFileSize);

		myFile.close();

		return myBuffer;
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
