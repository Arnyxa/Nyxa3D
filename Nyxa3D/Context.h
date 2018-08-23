#pragma once

#include "Globals.h"
#include "VulkanStructs.h"
#include "Structs.h"
#include "Window.h"
#include "Debug.h"
#include "Pipeline.h"
#include "VertexBuffer.h"

#include <vulkan/vulkan.hpp>
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
		void CreateSwapchain();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSemaphores();

		void RecreateSwapchain();
		void CleanupSwapchain();

		void SelectPhysicalDevice();
		bool IsDeviceSuitable(vk::PhysicalDevice aDevice);
		bool CheckDeviceExtensionSupport(vk::PhysicalDevice aDevice);
		bool CheckValidationLayerSupport();

		vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& aCapabilities);
		vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> anAvailablePresentModes);
		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& anAvailableFormats);
		SwapchainDetails QuerySwapchainSupport(vk::PhysicalDevice aDevice);
		QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice aDevice);
		std::vector<const char*> GetRequiredExtensions();

		static void OnWindowResize(GLFWwindow* aWindow, int aWidth, int aHeight);

		void MainLoop();

		void Draw();

	private:

		// Vulkan
		vk::Instance mInstance;
		vk::SurfaceKHR mSurface;

		vk::Device mDevice;
		vk::PhysicalDevice mPhysicalDevice;

		vk::Queue mPresentQueue;
		vk::Queue mGraphicsQueue;

		vk::SwapchainKHR mSwapchain;
		vk::Format mSwapchainImageFormat;
		vk::Extent2D mSwapchainExtent2D;

		vk::CommandPool mCommandPool;
		vk::Semaphore mImageAvailableSema;
		vk::Semaphore mRenderFinishedSema;

		// STL
		std::vector<vk::Image> mSwapchainImages;
		std::vector<vk::ImageView> mSwapchainImageViews;
		std::vector<vk::Framebuffer> mSwapchainFramebuffers;
		std::vector<vk::CommandBuffer> mCommandBuffers;

		// Nyxa3D
		Debug mDebugger;
		Window mWindow;
		Pipeline mPipeline;
		VertexBuffer mVertexBuffer;

		const std::vector<const char*> mValidationLayers = { LNG_STANDARD_VALIDATION_NAME };
		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}