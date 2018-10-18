#pragma once

#include "globals.hpp"
#include "vulkan_structs.hpp"
#include "structs.hpp"
#include "window.hpp"
#include "debug.hpp"
#include "pipeline.hpp"
#include "vertex_buffer.hpp"
#include "swapchain.hpp"

#include <vector>
#include <sstream>
#include <string>
#include <iostream>

namespace ppr
{
	class Context
	{
	public:
		Context(const std::string& aTitle = "Pepper");
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
        // Pepper
        Window mWindow;
        Debugger mDebugger;
        Swapchain mSwapchain;

		// Vulkan
		vk::Instance mInstance;
		vk::Device mDevice;
		vk::PhysicalDevice mPhysicalDevice;

		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}