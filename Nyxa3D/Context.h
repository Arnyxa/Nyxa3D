#pragma once

#include "Globals.h"
#include "VulkanStructs.h"
#include "Structs.h"
#include "Window.h"
#include "Debug.h"
#include "Pipeline.h"
#include "VertexBuffer.h"
#include "Swapchain.h"

#include <vector>
#include <sstream>
#include <string>
#include <iostream>

namespace nx
{
	class Context
	{
	public:
		Context();
		~Context();

		void Run();

	private:
		void Init();

		void CreateInstance();
		void CreateLogicalDevice();

		void SelectPhysicalDevice();
		bool IsDeviceSuitable(vk::PhysicalDevice aDevice);

		bool CheckDeviceExtensionSupport(vk::PhysicalDevice aDevice);

		std::vector<const char*> GetRequiredExtensions();

		void MainLoop();

	private:

		// Vulkan
		vk::Instance mInstance;

		vk::Device mDevice;
		vk::PhysicalDevice mPhysicalDevice;

		// Nyxa3D
		Debugger mDebugger;
		Window mWindow;
		Swapchain mSwapchain;

		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}