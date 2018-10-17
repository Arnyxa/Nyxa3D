#include "VertexBuffer.h"
#include "Util.h"

// Vertex
namespace ppr
{
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

// VertexBuffer
namespace ppr
{
	VertexBuffer::VertexBuffer(const vk::Device& aDevice) : mDevice(aDevice)
	{
		mVertices.push_back(Vertex({ { 0.0f, -0.5f }, { 0.4f, 0.7f, 0.3f } }));
		mVertices.push_back(Vertex({ { 0.5f, 0.5f }, { 0.4f, 0.3f, 0.1f } }));
		mVertices.push_back(Vertex({ { -0.5f, 0.5f }, { 0.1f, 0.2f, 0.8f } }));
	}

	uint32_t VertexBuffer::FindMemoryType(uint32_t aTypeFilter, vk::MemoryPropertyFlags aProperties, const vk::PhysicalDevice& aPhysicalDevice)
	{
		vk::PhysicalDeviceMemoryProperties myMemProperties = aPhysicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < myMemProperties.memoryTypeCount; ++i)
		{
			// find index of suitable memory type by checking if the corresponding bit is set to 1
			// also need to check if memory is suitable for writing to through property flags
			if ((aTypeFilter & (1 << i) && (myMemProperties.memoryTypes[i].propertyFlags & aProperties) == aProperties))
				return i;
		}

		throw Error("Failed to find suitable memory type.", Error::Code::BUFFER_MEMORY_TYPE_UNSUITABLE);
	}

	vk::Buffer& VertexBuffer::Get()
	{
		return mBuffer;
	}

	std::vector<Vertex>& VertexBuffer::GetVertexArray()
	{
		return mVertices;
	}

	void VertexBuffer::Create(const vk::PhysicalDevice& aPhysicalDevice)
	{
		vk::BufferCreateInfo myBufferInfo({}, sizeof(mVertices[0]) * mVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive);
		mBuffer = mDevice.createBuffer(myBufferInfo);

		vk::MemoryRequirements myMemReqs = mDevice.getBufferMemoryRequirements(mBuffer);
		vk::MemoryAllocateInfo myAllocInfo(myMemReqs.size, FindMemoryType(myMemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, aPhysicalDevice));

		mBufferMemory = mDevice.allocateMemory(myAllocInfo, nullptr);
		mDevice.bindBufferMemory(mBuffer, mBufferMemory, 0);

		void* myData = mDevice.mapMemory(mBufferMemory, 0, myBufferInfo.size);
		memcpy(myData, mVertices.data(), (size_t)myBufferInfo.size);
		mDevice.unmapMemory(mBufferMemory);
	}

	void VertexBuffer::Destroy()
	{
		mDevice.destroyBuffer(mBuffer);
		mDevice.freeMemory(mBufferMemory);
	}
}