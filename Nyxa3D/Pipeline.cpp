#include "Pipeline.h"

#include <iostream>
#include <fstream>
#include <string>

namespace nx
{
	Pipeline::Pipeline(const vk::Device& aDevice, const vk::Extent2D& aViewport)
		: mDevice(aDevice)
		, mViewport(aViewport)
	{}

	Pipeline::~Pipeline()
	{
		Destroy();
	}

	void Pipeline::Create()
	{
		std::cout << "Creating graphics pipeline...\n";

		auto myVertShaderCode = ReadShader("shaders/vert.spv");
		auto myFragShaderCode = ReadShader("shaders/frag.spv");

		vk::ShaderModule myVertShaderModule = CreateShaderModule(myVertShaderCode);
		vk::ShaderModule myFragShaderModule = CreateShaderModule(myFragShaderCode);

		std::cout << "Shaders loaded.\n";

		std::cout << "Initializing shader and pipeline info...\n";

		vk::PipelineShaderStageCreateInfo myVertShaderInfo = {};
		myVertShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		myVertShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		myVertShaderInfo.module = myVertShaderModule;
		myVertShaderInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo myFragShaderInfo = {};
		myFragShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		myFragShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		myFragShaderInfo.module = myFragShaderModule;
		myFragShaderInfo.pName = "main";

		vk::PipelineShaderStageCreateInfo myShaderStages[] = { myVertShaderInfo, myFragShaderInfo };

		vk::PipelineVertexInputStateCreateInfo myVertexInputInfo = {};
		myVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		myVertexInputInfo.vertexBindingDescriptionCount = NULL;
		myVertexInputInfo.pVertexBindingDescriptions = nullptr;
		myVertexInputInfo.vertexAttributeDescriptionCount = NULL;
		myVertexInputInfo.pVertexAttributeDescriptions = nullptr;

		vk::PipelineInputAssemblyStateCreateInfo myInputAssembly = {};
		myInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		myInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		myInputAssembly.primitiveRestartEnable = false;

		vk::Viewport myViewport = {};
		myViewport.x = 0.0f;
		myViewport.y = 0.0f;
		myViewport.width = (float)mViewport.width;
		myViewport.height = (float)mViewport.height;
		myViewport.minDepth = 0.0f;
		myViewport.maxDepth = 1.0f;

		vk::Rect2D myScissor = {};
		myScissor.offset = { 0, 0 };
		myScissor.extent = mViewport;

		vk::PipelineViewportStateCreateInfo myViewportState = {};
		myViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		myViewportState.viewportCount = 1;
		myViewportState.pViewports = &myViewport;
		myViewportState.scissorCount = 1;
		myViewportState.pScissors = &myScissor;

		vk::PipelineRasterizationStateCreateInfo myRasterizer = {};
		myRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		myRasterizer.depthClampEnable = false;
		myRasterizer.rasterizerDiscardEnable = false;
		myRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		myRasterizer.lineWidth = 1.0f;
		myRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		myRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		myRasterizer.depthBiasEnable = false;

		vk::PipelineMultisampleStateCreateInfo myMultiSampling = {};
		myMultiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		myMultiSampling.sampleShadingEnable = false;
		myMultiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		myMultiSampling.minSampleShading = 1.0f;
		myMultiSampling.pSampleMask = nullptr;
		myMultiSampling.alphaToCoverageEnable = false;
		myMultiSampling.alphaToOneEnable = false;

		vk::PipelineColorBlendAttachmentState myColorBlendAttachment = {};
		myColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		myColorBlendAttachment.blendEnable = false;
		myColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		myColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		myColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		myColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		myColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		myColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		vk::PipelineColorBlendStateCreateInfo myColorBlendingGlobal = {};
		myColorBlendingGlobal.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		myColorBlendingGlobal.logicOpEnable = false;
		myColorBlendingGlobal.attachmentCount = 1;
		myColorBlendingGlobal.pAttachments = &myColorBlendAttachment;

		std::cout << "Creating pipeline layout...\n";

		vk::PipelineLayoutCreateInfo myLayoutInfo = {};
		myLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		if (PrintResult(vkCreatePipelineLayout(mDevice, &myLayoutInfo, nullptr, &mPipelineLayout)) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan Pipeline Layout");

		vk::GraphicsPipelineCreateInfo myPipelineInfo = {};
		myPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		myPipelineInfo.stageCount = 2;
		myPipelineInfo.pStages = myShaderStages;
		myPipelineInfo.pVertexInputState = &myVertexInputInfo;
		myPipelineInfo.pInputAssemblyState = &myInputAssembly;
		myPipelineInfo.pViewportState = &myViewportState;
		myPipelineInfo.pRasterizationState = &myRasterizer;
		myPipelineInfo.pMultisampleState = &myMultiSampling;
		myPipelineInfo.pDepthStencilState = nullptr;
		myPipelineInfo.pColorBlendState = &myColorBlendingGlobal;
		myPipelineInfo.pDynamicState = nullptr;
		myPipelineInfo.layout = mPipelineLayout;
		myPipelineInfo.renderPass = mRenderPass;
		myPipelineInfo.subpass = 0;
		myPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		myPipelineInfo.basePipelineIndex = -1;

		if (PrintResult(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &myPipelineInfo, nullptr, &mPipeline)) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan Graphics Pipeline.");

		std::cout << "Graphics pipeline successfully created.\n\n";

		vkDestroyShaderModule(mDevice, myVertShaderModule, nullptr);
		vkDestroyShaderModule(mDevice, myFragShaderModule, nullptr);
	}

	vk::ShaderModule Pipeline::CreateShaderModule(const std::vector<char>& aByteCode)
	{
		std::cout << "Creating shader module...\n";

		vk::ShaderModuleCreateInfo myCreateInfo = {};
		myCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		myCreateInfo.codeSize = aByteCode.size();
		myCreateInfo.pCode = reinterpret_cast<const uint32_t*>(aByteCode.data());

		vk::ShaderModule myShaderModule;

		if (PrintResult(vkCreateShaderModule(mDevice, &myCreateInfo, nullptr, &myShaderModule)) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan Shader Module.");

		return myShaderModule;
	}

	std::vector<char> Pipeline::ReadShader(const std::string& aFileName)
	{
		// open file at its end to know the filesize
		std::ifstream myFile(aFileName, std::ios::ate | std::ios::binary);

		if (!myFile.is_open())
			throw std::runtime_error("Failed to open file \"" + aFileName + "\"");

		// figure out file size based on buffer position
		size_t myFileSize = (size_t)myFile.tellg();

		std::cout << aFileName << " size: " << myFileSize << std::endl;

		std::vector<char> myBuffer(myFileSize);

		// reset buffer position and read all bytes at once
		myFile.seekg(0);
		myFile.read(myBuffer.data(), myFileSize);

		myFile.close();

		return myBuffer;
	}

	vk::Pipeline& Pipeline::GetRef()
	{
		return mPipeline;
	}

	vk::PipelineLayout& Pipeline::GetLayout()
	{
		return mPipelineLayout;
	}

	vk::RenderPass& Pipeline::GetRenderPass()
	{
		return mRenderPass;
	}

	void Pipeline::Destroy()
	{
		vkDestroyPipeline(mDevice, mPipeline, nullptr);
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	}
}
