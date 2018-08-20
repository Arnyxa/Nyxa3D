#pragma once

#include "Globals.h"
#include "Structs.h"
#include "Structs.h"
#include "Window.h"
#include "Debug.h"

#include <vulkan/vulkan.hpp>
#include <vector>
#include <sstream>
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

		void RecreateSwapChain();
		void CleanupSwapChain();

		void SelectPhysicalDevice();
		bool IsDeviceSuitable(PhysicalDevice aDevice);
		bool CheckDeviceExtensionSupport(PhysicalDevice aDevice);
		bool CheckValidationLayerSupport();

		Extent2D ChooseSwapExtent(const SurfaceCapabilitiesKHR& aCapabilities);
		PresentModeKHR ChooseSwapPresentMode(const std::vector<PresentModeKHR> anAvailablePresentModes);
		SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<SurfaceFormatKHR>& anAvailableFormats);
		SwapchainDetails QuerySwapchainSupport(PhysicalDevice aDevice);
		QueueFamilyIndices FindQueueFamilies(PhysicalDevice aDevice);
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

		vk::SwapchainKHR mSwapChain;
		vk::Format mSwapChainImageFormat;
		vk::Extent2D mSwapChainExtent2D;

		vk::CommandPool mCommandPool;
		vk::Semaphore mImageAvailableSema;
		vk::Semaphore mRenderFinishedSema;

		// STL
		std::vector<vk::Image> mSwapChainImages;
		std::vector<vk::ImageView> mSwapChainImageViews;
		std::vector<vk::Framebuffer> mSwapChainFramebuffers;
		std::vector<vk::CommandBuffer> mCommandBuffers;

		// NyxaVK
		Debug mDebugger;
		Window mWindow;
		Pipeline mPipeline;

		// Const
		const std::vector<const char*> mValidationLayers = { LNG_STANDARD_VALIDATION_NAME };
		const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}