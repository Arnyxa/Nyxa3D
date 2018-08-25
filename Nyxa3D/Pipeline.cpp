#include "Pipeline.h"
#include "Util.h"
#include "DbgMsgr.h"

#include <iostream>
#include <fstream>
#include <string>

namespace nx
{
	Pipeline::Pipeline(const vk::Device& aDevice, const vk::Extent2D& aViewport)
		: mDevice(aDevice)
		, mViewport(aViewport)
		, mCleaned(false)
	{}

	Pipeline::~Pipeline()
	{}

	void Pipeline::Destroy()
	{
		mDevice.destroyPipeline(mPipeline);
		mDevice.destroyPipelineLayout(mPipelineLayout);
		mDevice.destroyRenderPass(mRenderPass);
	}


	void Pipeline::Create()
	{
		if (!mInitialised)
			DbgPrint("Creating graphics pipeline...\n");
		else
			DeepPrint("Creating graphics pipeline...\n");

		auto myVertShaderCode = ReadShader("shaders/vert.spv");
		auto myFragShaderCode = ReadShader("shaders/frag.spv");

		vk::ShaderModule myVertShaderModule = CreateShaderModule(myVertShaderCode);
		vk::ShaderModule myFragShaderModule = CreateShaderModule(myFragShaderCode);

		DeepPrint("Shaders loaded.\n");

		DeepPrint("Initializing shader and pipeline info...\n");

		vk::PipelineShaderStageCreateInfo myVertShaderInfo({}, vk::ShaderStageFlagBits::eVertex, myVertShaderModule, "main");
		vk::PipelineShaderStageCreateInfo myFragShaderInfo({}, vk::ShaderStageFlagBits::eFragment, myFragShaderModule, "main");

		vk::PipelineShaderStageCreateInfo myShaderStages[] = { myVertShaderInfo, myFragShaderInfo };

		auto myBinding = Vertex::GetBindingDescript();
		auto myAttributes = Vertex::GetAttributeDescript();

		vk::PipelineVertexInputStateCreateInfo myVertexInputInfo({}, 1, &myBinding, (uint32_t)myAttributes.size(), myAttributes.data());

		vk::PipelineInputAssemblyStateCreateInfo myInputAssembly({}, vk::PrimitiveTopology::eTriangleList);

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

		DeepPrint("Creating pipeline layout...\n");

		vk::PipelineLayoutCreateInfo myLayoutInfo;

		mPipelineLayout = mDevice.createPipelineLayout(myLayoutInfo);

		vk::GraphicsPipelineCreateInfo myPipelineInfo({}, 2, myShaderStages, &myVertexInputInfo, &myInputAssembly, nullptr, &myViewportState, 
										&myRasterizer, &myMultiSampling, nullptr, &myColorBlendingGlobal, nullptr, mPipelineLayout, 
										mRenderPass, 0, vk::Pipeline(), -1);

		if (Print(mDevice.createGraphicsPipelines(vk::PipelineCache(), 1, &myPipelineInfo, nullptr, &mPipeline)) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan Graphics Pipeline.");

		DeepPrint("Graphics pipeline successfully created.\n");

		mDevice.destroyShaderModule(myVertShaderModule);
		mDevice.destroyShaderModule(myFragShaderModule);

		if (!mInitialised)
			mInitialised = true;
	}

	vk::ShaderModule Pipeline::CreateShaderModule(const std::vector<char>& aByteCode)
	{
		DeepPrint("Creating shader module...\n");

		vk::ShaderModuleCreateInfo myCreateInfo({}, aByteCode.size(), reinterpret_cast<const uint32_t*>(aByteCode.data()));
		vk::ShaderModule myShaderModule = mDevice.createShaderModule(myCreateInfo);

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

		DeepPrint(std::string(aFileName) + " size: " + std::to_string(myFileSize) + "\n");

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
}
