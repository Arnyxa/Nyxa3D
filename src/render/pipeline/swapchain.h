#pragma once

#include "VulkanStructs.h"
#include "Structs.h"
#include "Util.h"
#include "Window.h"
#include "Pipeline.h"
#include "VertexBuffer.h"

#include <vulkan/vulkan.hpp>

#include <vector>
#include <functional>

namespace ppr
{
	class Swapchain : public CommonChecks
	{
	public:
		Swapchain(const vk::Device& aDevice, 
				const Window& aWindow,
				const vk::Instance& anInstance,
				const vk::PhysicalDevice& aPhysicalDevice);
		~Swapchain();

		void Draw();
		void Init();

		void Create();
		void Recreate();

		void Cleanup();
		void Destroy();

		void CreateWindowSurface();
		void OnWindowResize();

		SwapchainDetails QuerySupport(vk::PhysicalDevice aDevice);
		QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice aDevice);

		vk::Queue& GetGraphicsQueue();
		vk::Queue& GetPresentQueue();

	private:
		void DestroyWindowSurface();
		void CreateImageViews();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateRenderPass();
		void CreateSemaphores();

		vk::Extent2D ChooseExtent(const vk::SurfaceCapabilitiesKHR& aCapabilities);
		vk::PresentModeKHR ChoosePresentMode(const std::vector<vk::PresentModeKHR> anAvailablePresentModes);
		vk::SurfaceFormatKHR ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& anAvailableFormats);

	private:
		const Window& mWindow;
		const vk::Device& mDevice;
		const vk::Instance& mInstance;
		const vk::PhysicalDevice& mPhysicalDevice;

		Pipeline mPipeline;
		VertexBuffer mVertexBuffer;

		vk::SurfaceKHR mSurface;
		vk::SwapchainKHR mSwapchain;

		vk::Format mImageFormat;
		vk::Extent2D mExtent2D;

		vk::Queue mPresentQueue;
		vk::Queue mGraphicsQueue;

		vk::CommandPool mCommandPool;
		vk::Semaphore mImageAvailableSema;
		vk::Semaphore mRenderFinishedSema;

		std::vector<vk::Image> mSwapchainImages;
		std::vector<vk::ImageView> mImageViews;
		std::vector<vk::Framebuffer> mFramebuffers;
		std::vector<vk::CommandBuffer> mCommandBuffers;

		std::function<void(GLFWwindow*, int, int)> mResizeCallbackFn;
	};
}
