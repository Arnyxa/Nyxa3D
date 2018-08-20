#include "Context.h"

#include <set>
#include <algorithm>
#include <fstream>

namespace nx
{
#define VERTEX_COUNT 3

	Context::Context()
		: mVkPhysicalDevice(VK_NULL_HANDLE)
		, mDebugger(mVkInstance)
		, mPipeline(mVkLogicalDevice, mVkSwapChainExtent2D)
	{}

	void Context::Run()
	{
		Init();
		MainLoop();
	}

	void Context::MainLoop()
	{
		while (!mWindow.ShouldClose())
		{
			mWindow.PollEvents();
			Draw();
		}

		vkDeviceWaitIdle(mVkLogicalDevice);
	}

	void Context::Draw()
	{
		uint32_t myImageIndex;

		VkResult myResult = vkAcquireNextImageKHR(mVkLogicalDevice, mVkSwapChain, std::numeric_limits<uint64_t>::max(), mVkImageAvailableSema, NULL, &myImageIndex);

		if (PrintVkResult(myResult) == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (myResult != VK_SUCCESS && myResult != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to acquire swapchain image.");

		VkSubmitInfo mySubmitInfo = {};
		mySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore myWaitSemaphores[] = { mVkImageAvailableSema };
		VkPipelineStageFlags myWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		mySubmitInfo.waitSemaphoreCount = 1;
		mySubmitInfo.pWaitSemaphores = myWaitSemaphores;
		mySubmitInfo.pWaitDstStageMask = myWaitStages;
		mySubmitInfo.commandBufferCount = 1;
		mySubmitInfo.pCommandBuffers = &mVkCommandBuffers[myImageIndex];

		VkSemaphore mySignalSemaphores[] = { mVkRenderFinishedSema };
		mySubmitInfo.signalSemaphoreCount = 1;
		mySubmitInfo.pSignalSemaphores = mySignalSemaphores;

		if (PrintVkResult(vkQueueSubmit(mVkGraphicsQueue, 1, &mySubmitInfo, VK_NULL_HANDLE)) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit draw command buffer.");

		VkPresentInfoKHR myPresentInfo = {};
		myPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		myPresentInfo.waitSemaphoreCount = 1;
		myPresentInfo.pWaitSemaphores = mySignalSemaphores;

		VkSwapchainKHR mySwapChains[] = { mVkSwapChain };
		myPresentInfo.swapchainCount = 1;
		myPresentInfo.pSwapchains = mySwapChains;
		myPresentInfo.pImageIndices = &myImageIndex;
		myPresentInfo.pResults = nullptr;

		myResult = vkQueuePresentKHR(mVkPresentQueue, &myPresentInfo);

		if (PrintVkResult(myResult) == VK_ERROR_OUT_OF_DATE_KHR || myResult == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (myResult != VK_SUCCESS)
			throw std::runtime_error("Failed to present swapchain image.");

		vkQueueWaitIdle(mVkPresentQueue);
	}

	void Context::Init()
	{
		mWindow.Init();
		mWindow.SetResizeCallback(OnWindowResized, this);
		InitVulkan();
	}

	void Context::OnWindowResize(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		Context* myApp = reinterpret_cast<Context*>(glfwGetWindowUserPointer(aWindow));
		myApp->RecreateSwapChain();
		myApp->Draw();
	}

	void Context::InitVulkan()
	{
		std::cout << "Initializing Vulkan..." << std::endl;

		CreateVKInstance();
		mDebugger.Init();
		CreateSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		mPipeline.Create();
		CreateFrameBuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSemaphores();

		std::cout << "Vulkan initialized.\n" << std::endl;
	}

	void Context::RecreateSwapChain()
	{
		auto mySize = mWindow.GetSize();

		if (mySize.Width == 0 || mySize.Height == 0)
			return;

		vkDeviceWaitIdle(mVkLogicalDevice);

		CleanupSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		mPipeline.Create();
		CreateFrameBuffers();
		CreateCommandBuffers();
	}

	void Context::CreateSemaphores()
	{
		VkSemaphoreCreateInfo mySemaInfo = {};
		mySemaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (PrintVkResult(vkCreateSemaphore(mVkLogicalDevice, &mySemaInfo, nullptr, &mVkImageAvailableSema)) != VK_SUCCESS
			|| PrintVkResult(vkCreateSemaphore(mVkLogicalDevice, &mySemaInfo, nullptr, &mVkRenderFinishedSema)) != VK_SUCCESS)

			throw std::runtime_error("Failed to create Vulkan Semaphores.");
	}

	void Context::CreateCommandBuffers()
	{
		std::cout << "Creating Command Buffers..." << std::endl;

		mVkCommandBuffers.resize(mVkSwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo myAllocInfo = {};
		myAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		myAllocInfo.commandPool = mVkCommandPool;
		myAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		myAllocInfo.commandBufferCount = (uint32_t)mVkCommandBuffers.size();

		if (PrintVkResult(vkAllocateCommandBuffers(mVkLogicalDevice, &myAllocInfo, mVkCommandBuffers.data())) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate Vulkan Command Buffers.");

		for (size_t i = 0; i < mVkCommandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo myBeginInfo = {};
			myBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			myBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			myBeginInfo.pInheritanceInfo = nullptr;

			vkBeginCommandBuffer(mVkCommandBuffers[i], &myBeginInfo);

			VkRenderPassBeginInfo myRenderPassInfo = {};
			myRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			myRenderPassInfo.renderPass = mPipeline.GetRenderPass();
			myRenderPassInfo.framebuffer = mVkSwapChainFramebuffers[i];
			myRenderPassInfo.renderArea.offset = { 0, 0 };
			myRenderPassInfo.renderArea.extent = mVkSwapChainExtent2D;

			VkClearValue myClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			myRenderPassInfo.clearValueCount = 1;
			myRenderPassInfo.pClearValues = &myClearColor;

			vkCmdBeginRenderPass(mVkCommandBuffers[i], &myRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(mVkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.Get());

			// OWO WHAT'S THIS
			vkCmdDraw(mVkCommandBuffers[i], VERTEX_COUNT, 1, 0, 0);

			vkCmdEndRenderPass(mVkCommandBuffers[i]);

			if (PrintVkResult(vkEndCommandBuffer(mVkCommandBuffers[i])) != VK_SUCCESS)
				throw std::runtime_error("Failed to record Vulkan Command Buffer.");
		}

		std::cout << "Command Buffers created.\n" << std::endl;
	}

	void Context::CreateCommandPool()
	{
		std::cout << "Creating Command Pool..." << std::endl;

		nvk::QueueFamilyIndices myQueueFamilyIndices = FindQueueFamilies(mVkPhysicalDevice);

		VkCommandPoolCreateInfo myPoolInfo = {};
		myPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		myPoolInfo.queueFamilyIndex = myQueueFamilyIndices.mGraphicsFamily;
		myPoolInfo.flags = NULL;

		if (PrintVkResult(vkCreateCommandPool(mVkLogicalDevice, &myPoolInfo, nullptr, &mVkCommandPool)) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan Command Pool.");

		std::cout << "Command Pool created.\n" << std::endl;
	}

	void Context::CreateFrameBuffers()
	{
		std::cout << "Creating framebuffers..." << std::endl;

		mVkSwapChainFramebuffers.resize(mVkSwapChainImageViews.size());


		for (size_t i = 0; i < mVkSwapChainImageViews.size(); ++i)
		{
			VkImageView myAttachments[] = { mVkSwapChainImageViews[i] };

			VkFramebufferCreateInfo myFramebufferInfo = {};
			myFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			myFramebufferInfo.renderPass = mPipeline.GetRenderPass();
			myFramebufferInfo.attachmentCount = 1;
			myFramebufferInfo.pAttachments = myAttachments;
			myFramebufferInfo.width = mVkSwapChainExtent2D.width;
			myFramebufferInfo.height = mVkSwapChainExtent2D.height;
			myFramebufferInfo.layers = 1;

			if (PrintVkResult(vkCreateFramebuffer(mVkLogicalDevice, &myFramebufferInfo, nullptr, &mVkSwapChainFramebuffers[i])) != VK_SUCCESS)
				throw std::runtime_error("Failed to create Vulkan Framebuffer.");
		}

		std::cout << "Finished creating framebuffers." << std::endl;
	}

	void Context::CreateRenderPass()
	{
		VkAttachmentDescription myColorAttachment = {};
		myColorAttachment.format = mVkSwapChainImageFormat;
		myColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		myColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		myColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		myColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		myColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		myColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		myColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference myColorAttachRef = {};
		myColorAttachRef.attachment = 0;
		myColorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription mySubpass = {};
		mySubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		mySubpass.colorAttachmentCount = 1;
		mySubpass.pColorAttachments = &myColorAttachRef;

		VkSubpassDependency myDependency = {};
		myDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		myDependency.dstSubpass = NULL;
		myDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		myDependency.srcAccessMask = NULL;
		myDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		myDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo myRenderPassInfo = {};
		myRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		myRenderPassInfo.attachmentCount = 1;
		myRenderPassInfo.pAttachments = &myColorAttachment;
		myRenderPassInfo.subpassCount = 1;
		myRenderPassInfo.pSubpasses = &mySubpass;
		myRenderPassInfo.dependencyCount = 1;
		myRenderPassInfo.pDependencies = &myDependency;

		if (PrintVkResult(vkCreateRenderPass(mVkLogicalDevice, &myRenderPassInfo, nullptr, &mPipeline.GetRenderPass())) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan Render Pass");
	}

	void Context::CreateImageViews()
	{
		mVkSwapChainImageViews.resize(mVkSwapChainImages.size());

		for (size_t i = 0; i < mVkSwapChainImages.size(); ++i)
		{
			std::cout << "Creating Swapchain image view..." << std::endl;

			VkImageViewCreateInfo myCreateInfo = {};
			myCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			myCreateInfo.image = mVkSwapChainImages[i];
			myCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			myCreateInfo.format = mVkSwapChainImageFormat;

			myCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			myCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			myCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			myCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			myCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			myCreateInfo.subresourceRange.baseMipLevel = NULL;
			myCreateInfo.subresourceRange.levelCount = 1;
			myCreateInfo.subresourceRange.baseArrayLayer = NULL;
			myCreateInfo.subresourceRange.layerCount = 1;

			if (PrintVkResult(vkCreateImageView(mVkLogicalDevice, &myCreateInfo, nullptr, &mVkSwapChainImageViews[i])) != VK_SUCCESS)
				throw std::runtime_error("Failed to create Vulkan ImageView.");
		}

		std::cout << "Image views created.\n" << std::endl;
	}

	void Context::CreateSwapChain()
	{
		std::cout << "Initializing Vulkan Swapchain..." << std::endl;

		nvk::SwapChainSupportDetails mySwapChainSupport = QuerySwapChainSupport(mVkPhysicalDevice);

		VkSurfaceFormatKHR mySurfaceFormat = ChooseSwapSurfaceFormat(mySwapChainSupport.mFormats);
		VkPresentModeKHR myPresentMode = ChooseSwapPresentMode(mySwapChainSupport.mPresentModes);
		VkExtent2D myExtent2D = ChooseSwapExtent(mySwapChainSupport.mCapabilities);

		uint32_t myImageCount = mySwapChainSupport.mCapabilities.minImageCount + 1;
		if (mySwapChainSupport.mCapabilities.maxImageCount > 0 && myImageCount > mySwapChainSupport.mCapabilities.maxImageCount)
			myImageCount = mySwapChainSupport.mCapabilities.maxImageCount;

		VkSwapchainCreateInfoKHR mySwapCreateInfo = {};

		mySwapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		mySwapCreateInfo.surface = mVkSurface;
		mySwapCreateInfo.minImageCount = myImageCount;
		mySwapCreateInfo.imageFormat = mySurfaceFormat.format;
		mySwapCreateInfo.imageColorSpace = mySurfaceFormat.colorSpace;
		mySwapCreateInfo.imageExtent = myExtent2D;
		mySwapCreateInfo.imageArrayLayers = 1;
		mySwapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		nvk::QueueFamilyIndices myIndices = FindQueueFamilies(mVkPhysicalDevice);
		uint32_t myQueueFamilies[] = { (uint32_t)myIndices.mGraphicsFamily, (uint32_t)myIndices.mPresentFamily };

		std::cout << "Evaluating image sharing mode..." << std::endl;
		std::cout << "Using ";
		if (myIndices.mGraphicsFamily != myIndices.mPresentFamily)
		{
			std::cout << "Concurrent ";
			mySwapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			mySwapCreateInfo.queueFamilyIndexCount = 2;
			mySwapCreateInfo.pQueueFamilyIndices = myQueueFamilies;
		}
		else
		{
			std::cout << "Exclusive ";
			mySwapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			mySwapCreateInfo.queueFamilyIndexCount = NULL; // optional
			mySwapCreateInfo.pQueueFamilyIndices = nullptr; // optional
		}
		std::cout << "sharing mode." << std::endl;

		mySwapCreateInfo.preTransform = mySwapChainSupport.mCapabilities.currentTransform;
		mySwapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		mySwapCreateInfo.presentMode = myPresentMode;
		mySwapCreateInfo.clipped = true;
		mySwapCreateInfo.oldSwapchain = VK_NULL_HANDLE;

		if (PrintVkResult(vkCreateSwapchainKHR(mVkLogicalDevice, &mySwapCreateInfo, nullptr, &mVkSwapChain)) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan Swapchain.");

		vkGetSwapchainImagesKHR(mVkLogicalDevice, mVkSwapChain, &myImageCount, nullptr);
		mVkSwapChainImages.resize(myImageCount);
		vkGetSwapchainImagesKHR(mVkLogicalDevice, mVkSwapChain, &myImageCount, mVkSwapChainImages.data());

		mVkSwapChainImageFormat = mySurfaceFormat.format;
		mVkSwapChainExtent2D = myExtent2D;

		std::cout << "Successfully created Vulkan Swapchain.\n" << std::endl;
	}

	void Context::CreateSurface()
	{
		std::cout << "Creating Vulkan surface..." << std::endl;

		if (PrintVkResult(glfwCreateWindowSurface(mVkInstance, mWindow.GetPtr(), nullptr, &mVkSurface)) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan surface for GLFW window.");
	}

	void Context::CreateLogicalDevice()
	{
		std::cout << "Creating logical Vulkan Device..." << std::endl;

		nvk::QueueFamilyIndices myIndices = FindQueueFamilies(mVkPhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> myQueueCreateInfos;
		std::set<int> myUniqueQueueFamilies = { myIndices.mGraphicsFamily, myIndices.mPresentFamily };

		float myQueuePriority = 1.f;
		for (auto& iQueueFamily : myUniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo myQueueCreateInfo = {};
			myQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			myQueueCreateInfo.queueFamilyIndex = iQueueFamily;
			myQueueCreateInfo.queueCount = 1;
			myQueueCreateInfo.pQueuePriorities = &myQueuePriority;

			myQueueCreateInfos.push_back(myQueueCreateInfo);
		}

		VkPhysicalDeviceFeatures myDeviceFeatures = {};

		VkDeviceCreateInfo myDeviceCreateInfo = {};
		myDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		myDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(myQueueCreateInfos.size());
		myDeviceCreateInfo.pQueueCreateInfos = myQueueCreateInfos.data();
		myDeviceCreateInfo.pEnabledFeatures = &myDeviceFeatures;

		myDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
		myDeviceCreateInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

		if (Debug::VALIDATION_LAYERS_ENABLED)
		{
			myDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			myDeviceCreateInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
			myDeviceCreateInfo.enabledLayerCount = 0;

		if (PrintVkResult(vkCreateDevice(mVkPhysicalDevice, &myDeviceCreateInfo, nullptr, &mVkLogicalDevice)) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan Logical Device.");

		vkGetDeviceQueue(mVkLogicalDevice, myIndices.mGraphicsFamily, NULL, &mVkGraphicsQueue);
		vkGetDeviceQueue(mVkLogicalDevice, myIndices.mPresentFamily, NULL, &mVkPresentQueue);

		std::cout << "Successfully created logical Vulkan Device.\ngg" << std::endl;
	}

	nvk::QueueFamilyIndices Context::FindQueueFamilies(VkPhysicalDevice aDevice)
	{
		std::cout << "Searching for available queue families..." << std::endl;

		nvk::QueueFamilyIndices myIndices;

		uint32_t myQFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(aDevice, &myQFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> myQFamilies(myQFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(aDevice, &myQFamilyCount, myQFamilies.data());

		for (uint16_t i = 0; i < myQFamilies.size(); ++i)
		{
			if (myQFamilies[i].queueCount > 0 && myQFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				myIndices.mGraphicsFamily = i;

			VkBool32 myPresentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(aDevice, i, mVkSurface, &myPresentSupport);
			if (myQFamilies[i].queueCount > QUEUE_FAMILY_MIN_INDEX && myPresentSupport)
				myIndices.mPresentFamily = i;

			if (myIndices.IsComplete())
				break;
		}
		return myIndices;
	}

	void Context::SelectPhysicalDevice()
	{
		std::cout << "Searching for viable physical device..." << std::endl;

		uint32_t myDeviceCount = 0;
		vkEnumeratePhysicalDevices(mVkInstance, &myDeviceCount, nullptr);

		if (myDeviceCount == 0)
			throw std::runtime_error("Failed to find any GPU with Vulkan support.");

		std::vector<VkPhysicalDevice> myDevices(myDeviceCount);
		vkEnumeratePhysicalDevices(mVkInstance, &myDeviceCount, myDevices.data());

		for (const auto& iDevice : myDevices)
		{
			if (IsDeviceSuitable(iDevice))
			{
				mVkPhysicalDevice = iDevice;
				break;
			}
		}

		if (mVkPhysicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU for Vulkan.");

		std::cout << "Matching Vulkan-compatible GPU(s) successfully found.\n" << std::endl;
	}

	bool Context::IsDeviceSuitable(VkPhysicalDevice aDevice)
	{
		//VkPhysicalDeviceProperties myProperties;
		//VkPhysicalDeviceFeatures myFeatures;
		//vkGetPhysicalDeviceProperties(aDevice, &myProperties);
		//vkGetPhysicalDeviceFeatures(aDevice, &myFeatures);

		// return myProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		// myFeatures.geometryShader;

		std::cout << "Evaluating device suitability..." << std::endl;

		nvk::QueueFamilyIndices myIndices = FindQueueFamilies(aDevice);

		bool isExtensionsSupported = CheckDeviceExtensionSupport(aDevice);
		bool isSwapChainAdequate = false;

		if (isExtensionsSupported)
		{
			nvk::SwapChainSupportDetails mySwapChainSupport = QuerySwapChainSupport(aDevice);
			isSwapChainAdequate = !mySwapChainSupport.mFormats.empty() && !mySwapChainSupport.mPresentModes.empty();
		}

		return myIndices.IsComplete() && isExtensionsSupported && isSwapChainAdequate;
	}

	VkSurfaceFormatKHR Context::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& anAvailableFormats)
	{
		std::cout << "Checking available Swapchain Surface Format..." << std::endl;
		if (anAvailableFormats.size() == 1 && anAvailableFormats[0].format == VK_FORMAT_UNDEFINED)
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

		for (const auto& iAvailableFormat : anAvailableFormats)
		{
			if (iAvailableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && iAvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return iAvailableFormat;
		}

		return anAvailableFormats[0];
	}

	VkPresentModeKHR Context::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> anAvailablePresentModes)
	{
		std::cout << "Determining optimal Swapchain Present mode..." << std::endl;
		VkPresentModeKHR myBestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& iAvailableMode : anAvailablePresentModes)
		{
			if (iAvailableMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return iAvailableMode;
			else if (iAvailableMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				myBestMode = iAvailableMode;
		}

		return myBestMode;
	}

	VkExtent2D Context::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& aCapabilities)
	{
		std::cout << "Choosing Swapchain Extent..." << std::endl;

		if (aCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return aCapabilities.currentExtent;
		else
		{
			auto mySize = (Size<uint32_t>)mWindow.GetSize();

			VkExtent2D myActualExtent = { mySize.Width, mySize.Height };

			myActualExtent.width = std::max(aCapabilities.minImageExtent.width, std::min(aCapabilities.maxImageExtent.width, myActualExtent.width));
			myActualExtent.height = std::max(aCapabilities.minImageExtent.height, std::min(aCapabilities.maxImageExtent.height, myActualExtent.height));

			return myActualExtent;
		}
	}

	std::vector<const char*> Context::GetRequiredExtensions()
	{
		std::cout << "Fetching required extensions..." << std::endl;
		uint32_t myGlfwExtensionCount = 0;
		const char** myGlfwExtensions;
		myGlfwExtensions = glfwGetRequiredInstanceExtensions(&myGlfwExtensionCount);

		std::vector<const char*> myExtensions(myGlfwExtensions, myGlfwExtensions + myGlfwExtensionCount);

		if (Debug::VALIDATION_LAYERS_ENABLED)
			myExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		return myExtensions;
	}

	bool Context::CheckDeviceExtensionSupport(VkPhysicalDevice aDevice)
	{
		std::cout << "Checking device extension support..." << std::endl;
		uint32_t myExtensionCount;
		vkEnumerateDeviceExtensionProperties(aDevice, nullptr, &myExtensionCount, nullptr);

		std::vector<VkExtensionProperties> myAvailableExtensions(myExtensionCount);
		vkEnumerateDeviceExtensionProperties(aDevice, nullptr, &myExtensionCount, myAvailableExtensions.data());

		std::set<std::string> myRequiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

		for (const auto& iExtension : myAvailableExtensions)
			myRequiredExtensions.erase(iExtension.extensionName);

		return myRequiredExtensions.empty();
	}

	void Context::CreateVKInstance()
	{
		if (Debug::VALIDATION_LAYERS_ENABLED && !CheckValidationLayerSupport())
			throw std::runtime_error("Validation layers requested, but not available.");

		// initialize the vulkan application info struct
		VkApplicationInfo myAppInfo = {};

		myAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		myAppInfo.pApplicationName = "owo Triangle";
		myAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		myAppInfo.pEngineName = "No engine";
		myAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		myAppInfo.apiVersion = VK_API_VERSION_1_0;

		// do the same for the creation info struct
		VkInstanceCreateInfo myCreateInfo = {};
		myCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		myCreateInfo.pApplicationInfo = &myAppInfo;

		// fetch extensions needed of vulkan by glfw
		auto myExtensions = GetRequiredExtensions();

		myCreateInfo.enabledExtensionCount = static_cast<uint32_t>(myExtensions.size());
		myCreateInfo.ppEnabledExtensionNames = myExtensions.data();

		// set up LunarG validation layers
		if (Debug::VALIDATION_LAYERS_ENABLED)
		{
			myCreateInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			myCreateInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
			myCreateInfo.enabledLayerCount = 0;

		// fetch extension count
		uint32_t myExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &myExtensionCount, nullptr);

		// allocate vector with number of extensions and insert them into the vector
		std::vector<VkExtensionProperties> myVkExtensions(myExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &myExtensionCount, myVkExtensions.data());

		// print out the names of the extensions we have available to us
		std::cout << "Available extensions:" << std::endl;
		for (const auto& iExtension : myVkExtensions)
		{
			std::cout << "\t" << iExtension.extensionName << std::endl;
		}
		std::cout << myExtensionCount << " extensions found in total.\n" << std::endl;

		std::cout << "Checking for GLFW extension compatibility..." << std::endl;

		// check whether extensions required by GLFW are in available extension list
		for (size_t i = 0; i < myExtensions.size(); ++i)
		{
			bool isFound = false;
			for (const auto& iVkExtension : myVkExtensions)
			{
				if (strcmp(myExtensions[i], iVkExtension.extensionName) == 0)
				{
					isFound = true;
					break;
				}
			}

			if (!isFound)
				throw std::runtime_error("Could not find required GLFW extension for Vulkan on this system.");

			std::cout << "        " << myExtensions[i] << " found." << std::endl;
		}

		std::cout << "Extension check successful.\n" << std::endl;

		// finally create our instance of vulkan
		if (PrintVkResult(vkCreateInstance(&myCreateInfo, nullptr, &mVkInstance)) != VK_SUCCESS)
			throw std::runtime_error("Could not create Vulkan instance.");
	}

	nvk::SwapChainSupportDetails Context::QuerySwapChainSupport(VkPhysicalDevice aDevice)
	{
		std::cout << "Querying Swapchain support..." << std::endl;

		nvk::SwapChainSupportDetails myDetails;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(aDevice, mVkSurface, &myDetails.mCapabilities);

		uint32_t myFormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(aDevice, mVkSurface, &myFormatCount, nullptr);

		if (myFormatCount != NULL)
		{
			myDetails.mFormats.resize(myFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(aDevice, mVkSurface, &myFormatCount, myDetails.mFormats.data());
		}

		uint32_t myPresentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(aDevice, mVkSurface, &myPresentModeCount, nullptr);

		if (myPresentModeCount != NULL)
		{
			myDetails.mPresentModes.resize(myPresentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(aDevice, mVkSurface, &myPresentModeCount, myDetails.mPresentModes.data());
		}

		return myDetails;
	}

	bool Context::CheckValidationLayerSupport()
	{
		std::cout << "Checking for validation layer compatibility..." << std::endl;

		uint32_t myLayerCount;
		vkEnumerateInstanceLayerProperties(&myLayerCount, nullptr);

		std::vector<VkLayerProperties> myAvailableLayers(myLayerCount);
		vkEnumerateInstanceLayerProperties(&myLayerCount, myAvailableLayers.data());

		for (const char* iLayerName : mValidationLayers)
		{
			bool isFound = false;

			for (const auto& iLayerProperties : myAvailableLayers)
			{
				if (strcmp(iLayerName, iLayerProperties.layerName) == 0)
				{
					isFound = true;
					break;
				}
			}

			if (!isFound)
			{
				std::cout << iLayerName << " is not supported." << std::endl;
				return false;
			}

			std::cout << iLayerName << " is supported." << std::endl;
		}

		std::cout << "All validation layers are supported.\n" << std::endl;

		return true;
	}

	void Context::CleanupSwapChain()
	{
		for (auto iBuffer : mVkSwapChainFramebuffers)
			vkDestroyFramebuffer(mVkLogicalDevice, iBuffer, nullptr);

		vkFreeCommandBuffers(mVkLogicalDevice, mVkCommandPool, static_cast<uint32_t>(mVkCommandBuffers.size()), mVkCommandBuffers.data());
		mPipeline.Destroy();

		for (auto iView : mVkSwapChainImageViews)
			vkDestroyImageView(mVkLogicalDevice, iView, nullptr);

		vkDestroySwapchainKHR(mVkLogicalDevice, mVkSwapChain, nullptr);
	}

	Context::~Context()
	{
		std::cout << "Terminating program..." << std::endl;

		CleanupSwapChain();

		vkDestroySemaphore(mVkLogicalDevice, mVkRenderFinishedSema, nullptr);
		vkDestroySemaphore(mVkLogicalDevice, mVkImageAvailableSema, nullptr);
		vkDestroyCommandPool(mVkLogicalDevice, mVkCommandPool, nullptr);
		vkDestroyDevice(mVkLogicalDevice, nullptr);
		vkDestroySurfaceKHR(mVkInstance, mVkSurface, nullptr);

		mDebugger.Destroy();
		vkDestroyInstance(mVkInstance, nullptr);

		std::cout << "Application terminated. Exiting." << std::endl;
	}

}