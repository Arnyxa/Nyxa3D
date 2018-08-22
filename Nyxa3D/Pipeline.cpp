#include "Pipeline.h"
#include "Util.h"

#include <iostream>
#include <fstream>
#include <string>

namespace nx
{
	Pipeline::Pipeline(const vk::Device& aDevice, const vk::PhysicalDevice& aPhysDevice, const vk::Extent2D& aViewport)
		: mDevice(aDevice)
		, mViewport(aViewport)
		, mPhysDevice(aPhysDevice)
		, mCleaned(false)
	{
		mVertices.push_back(Vertex({ { 0.0f, -0.5f }, { 0.4f, 0.7f, 0.3f } }));
		mVertices.push_back(Vertex({ { 0.5f, 0.5f }, { 0.4f, 0.3f, 0.1f } }));
		mVertices.push_back(Vertex({ { -0.5f, 0.5f }, { 0.1f, 0.2f, 0.8f } }));
	}

	Pipeline::~Pipeline()
	{}

	void Pipeline::DestroyPipeline()
	{
		mDevice.destroyPipeline(mPipeline);
		mDevice.destroyPipelineLayout(mPipelineLayout);
		mDevice.destroyRenderPass(mRenderPass);
	}

	void Pipeline::DestroyVertexBuffer()
	{
		mDevice.destroyBuffer(mVertexBuffer);
		mDevice.freeMemory(mVrtxBufferMemory);
	}

	void Pipeline::UpdateVertexArray()
	{
		static float val = 0.0015f;
		const float upper = 1.f;
		const float lower = 0.1f;

		static float rm = val;
		static float gm = val;
		static float bm = val;
		static float xm = val;
		static float ym = val;

		for (int i = 0; i < mVertices.size(); ++i)
		{
			// Red
			if (mVertices[i].Color.r >= upper)
				rm = -val;
			else if (mVertices[i].Color.r <= lower)
				rm = val;
			mVertices[i].Color.r += rm;

			// Green
			if (mVertices[i].Color.g >= upper)
				gm = -val;
			else if (mVertices[i].Color.g <= lower)
				gm = val;
			mVertices[i].Color.g += gm;

			// Blue
			if (mVertices[i].Color.b >= upper)
				bm = -val;
			else if (mVertices[i].Color.b <= lower)
				bm = val;
			mVertices[i].Color.b += rm;
		}
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

		std::cout << "Creating pipeline layout...\n";

		vk::PipelineLayoutCreateInfo myLayoutInfo;

		mPipelineLayout = mDevice.createPipelineLayout(myLayoutInfo);

		std::cout << "Creating graphics pipeline...\n";

		vk::GraphicsPipelineCreateInfo myPipelineInfo({}, 2, myShaderStages, &myVertexInputInfo, &myInputAssembly, nullptr, &myViewportState, 
										&myRasterizer, &myMultiSampling, nullptr, &myColorBlendingGlobal, nullptr, mPipelineLayout, 
										mRenderPass, 0, vk::Pipeline(), -1);

		if (Print(mDevice.createGraphicsPipelines(vk::PipelineCache(), 1, &myPipelineInfo, nullptr, &mPipeline)) != vk::Result::eSuccess)
			throw std::runtime_error("Failed to create Vulkan Graphics Pipeline.");

		std::cout << "Graphics pipeline successfully created.\n\n";

		mDevice.destroyShaderModule(myVertShaderModule);
		mDevice.destroyShaderModule(myFragShaderModule);
	}

	vk::Buffer& Pipeline::GetVertexBuffer()
	{
		return mVertexBuffer;
	}

	std::vector<Vertex>& Pipeline::GetVertexArray()
	{
		return mVertices;
	}

	void Pipeline::CreateVertexBuffer()
	{
		vk::BufferCreateInfo myBufferInfo({}, sizeof(mVertices[0]) * mVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		mVertexBuffer = mDevice.createBuffer(myBufferInfo);

		vk::MemoryRequirements myMemReqs = mDevice.getBufferMemoryRequirements(mVertexBuffer);
		vk::MemoryAllocateInfo myAllocInfo(myMemReqs.size, FindMemType(myMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

		mVrtxBufferMemory = mDevice.allocateMemory(myAllocInfo, nullptr);
		mDevice.bindBufferMemory(mVertexBuffer, mVrtxBufferMemory, 0);

		void* myData = mDevice.mapMemory(mVrtxBufferMemory, 0, myBufferInfo.size);
		memcpy(myData, mVertices.data(), (size_t)myBufferInfo.size);
		mDevice.unmapMemory(mVrtxBufferMemory);
	}

	uint32_t Pipeline::FindMemType(uint32_t aTypeFilter, vk::MemoryPropertyFlags aProperties)
	{
		vk::PhysicalDeviceMemoryProperties myMemProperties = mPhysDevice.getMemoryProperties();

		for (uint32_t i = 0; i < myMemProperties.memoryTypeCount; ++i)
		{
			// find index ofsuitable memory type by checking if the corresponding bit is set to 1
			// also need to check if memory is suitable for writing to through property flags
			if ((aTypeFilter & (1 << i) && (myMemProperties.memoryTypes[i].propertyFlags & aProperties) == aProperties))
				return i;
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

	vk::ShaderModule Pipeline::CreateShaderModule(const std::vector<char>& aByteCode)
	{
		std::cout << "Creating shader module...\n";

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

	vk::VertexInputBindingDescription Vertex::GetBindingDescript()
	{
		return vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
	}

	std::array<vk::VertexInputAttributeDescription, 2> Vertex::GetAttributeDescript()
	{
		std::array<vk::VertexInputAttributeDescription, 2> myAttributes;

		myAttributes[0].binding = 0;
		myAttributes[0].location = 0;
		myAttributes[0].format = vk::Format::eR32G32Sfloat;
		myAttributes[0].offset = offsetof(Vertex, Pos);

		myAttributes[1].binding = 0;
		myAttributes[1].location = 1;
		myAttributes[1].format = vk::Format::eR32G32B32Sfloat;
		myAttributes[1].offset = offsetof(Vertex, Color);

		return myAttributes;
	}
}
