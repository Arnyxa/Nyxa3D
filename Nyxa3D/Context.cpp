#include "Context.h"
#include "Util.h"

#include <glfw/glfw3.h>

#include <set>
#include <algorithm>
#include <fstream>

namespace nx
{
#define VERTEX_COUNT 3

	Context::Context()
		: mPhysicalDevice()
		, mDebugger(mInstance)
		, mPipeline(mDevice, mSwapchainExtent2D)
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

		vkDeviceWaitIdle(mDevice);
	}

	void Context::Draw()
	{
		vk::ResultValue<uint32_t> myResultPair = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvailableSema, nullptr);

		uint32_t myImageIndex = myResultPair.value;
		vk::Result myResult = myResultPair.result;

		if (PrintResult(myResult) == vk::Result::eErrorOutOfDateKHR)
		{
			RecreateSwapchain();
			return;
		}

		else if (myResult != vk::Result::eSuccess && myResult != vk::Result::eSuboptimalKHR)
			throw std::runtime_error("Failed to acquire swapchain image.");

		vk::Semaphore myWaitSemaphores[] = { mImageAvailableSema };
		vk::Semaphore mySignalSemaphores[] = { mRenderFinishedSema };
		vk::PipelineStageFlags myWaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo mySubmitInfo(1, myWaitSemaphores, myWaitStages, 1, &mCommandBuffers[myImageIndex], 1, mySignalSemaphores);

		mGraphicsQueue.submit(mySubmitInfo, nullptr);

		vk::SwapchainKHR mySwapchains[] = { mSwapchain };
		vk::PresentInfoKHR myPresentInfo(1, mySignalSemaphores, 1, mySwapchains, &myImageIndex);

		myResult = mPresentQueue.presentKHR(myPresentInfo);

		if (PrintResult(myResult) == vk::Result::eErrorOutOfDateKHR || myResult == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}
		else if (myResult != vk::Result::eSuccess)
			throw std::runtime_error("Failed to present swapchain image.");

		vkQueueWaitIdle(mPresentQueue);
	}

	void Context::Init()
	{
		std::cout << "Initializing Window...\n";

		mWindow.Init();
		mWindow.SetResizeCallback(OnWindowResize, this);

		std::cout << "Window initialized.\n\n";
		
		std::cout << "Initializing Vulkan...\n";

		CreateInstance();
		mDebugger.Init();
		CreateSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		mPipeline.Create();
		CreateFrameBuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSemaphores();

		std::cout << "Vulkan initialized.\n\n";
	}

	void Context::OnWindowResize(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		Context* myApp = reinterpret_cast<Context*>(glfwGetWindowUserPointer(aWindow));
		myApp->RecreateSwapchain();
		myApp->Draw();
	}

	void Context::RecreateSwapchain()
	{
		auto mySize = mWindow.GetSize();

		if (mySize.Width == 0 || mySize.Height == 0)
			return;

		vkDeviceWaitIdle(mDevice);

		CleanupSwapchain();

		CreateSwapchain();
		CreateImageViews();
		CreateRenderPass();
		mPipeline.Create();
		CreateFrameBuffers();
		CreateCommandBuffers();
	}

	void Context::CreateSemaphores()
	{
		vk::SemaphoreCreateInfo mySemaInfo = {};
		
		mImageAvailableSema = mDevice.createSemaphore(mySemaInfo);
		mRenderFinishedSema = mDevice.createSemaphore(mySemaInfo);
		
		// No error checking?
	}

	void Context::CreateCommandBuffers()
	{
		std::cout << "Creating Command Buffers...\n";

		mCommandBuffers.resize(mSwapchainFramebuffers.size());

		vk::CommandBufferAllocateInfo myAllocInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, (uint32_t)mCommandBuffers.size());
		mCommandBuffers = mDevice.allocateCommandBuffers(myAllocInfo);

		for (size_t i = 0; i < mCommandBuffers.size(); ++i)
		{
			vk::CommandBufferBeginInfo myBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			mCommandBuffers[i].begin(myBeginInfo);

			vk::ClearValue myClearColor(vk::ClearColorValue(std::array<float, 4>({ 0.f, 0.f, 0.f, 1.f })));
			vk::Rect2D myRect({ 0, 0 }, mSwapchainExtent2D);
			vk::RenderPassBeginInfo myRenderPassInfo(mPipeline.GetRenderPass(), mSwapchainFramebuffers[i], myRect, 1, &myClearColor);

			mCommandBuffers[i].beginRenderPass(myRenderPassInfo, vk::SubpassContents::eInline);
			mCommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.GetRef());

			// OWO WHAT'S THIS
			mCommandBuffers[i].draw(VERTEX_COUNT, 1, 0, 0);
			mCommandBuffers[i].endRenderPass();
			mCommandBuffers[i].end();
		}

		std::cout << "Command Buffers created.\n\n";
	}

	void Context::CreateCommandPool()
	{
		std::cout << "Creating Command Pool...\n";

		QueueFamilyIndices myQueueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

		vk::CommandPoolCreateInfo myPoolInfo({}, myQueueFamilyIndices.Graphics);
		mCommandPool = mDevice.createCommandPool(myPoolInfo);

		std::cout << "Command Pool created.\n\n";
	}

	void Context::CreateFrameBuffers()
	{
		std::cout << "Creating framebuffers...\n";

		mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

		for (size_t i = 0; i < mSwapchainImageViews.size(); ++i)
		{
			vk::ImageView myAttachments[] = { mSwapchainImageViews[i] };

			vk::FramebufferCreateInfo myFramebufferInfo({}, mPipeline.GetRenderPass(), 1, myAttachments, mSwapchainExtent2D.width, mSwapchainExtent2D.height, 1);

			mSwapchainFramebuffers[i] = mDevice.createFramebuffer(myFramebufferInfo);
		}

		std::cout << "Finished creating framebuffers.\n";
	}

	void Context::CreateRenderPass()
	{
		vk::AttachmentDescription myColorAttachment({}, mSwapchainImageFormat, vk::SampleCountFlagBits::e1,
													vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
													vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
													vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference myColorAttachRef(0, vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription mySubpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &myColorAttachRef);

		vk::SubpassDependency myDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
											vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlags(),
											vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo myRenderPassInfo({}, 1, &myColorAttachment, 1, &mySubpass, 1, &myDependency);

		mPipeline.GetRenderPass() = mDevice.createRenderPass(myRenderPassInfo);
	}

	void Context::CreateImageViews()
	{
		mSwapchainImageViews.resize(mSwapchainImages.size());

		for (size_t i = 0; i < mSwapchainImages.size(); ++i)
		{
			std::cout << "Creating Swapchain image view...\n";

			vk::ImageSubresourceRange mySubRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			vk::ImageViewCreateInfo myCreateInfo({}, mSwapchainImages[i], vk::ImageViewType::e2D, mSwapchainImageFormat, vk::ComponentMapping(), mySubRange);

			mSwapchainImageViews[i] = mDevice.createImageView(myCreateInfo);
		}

		std::cout << "Image views created.\n\n";
	}

	void Context::CreateSwapchain()
	{
		std::cout << "Initializing Vulkan Swapchain...\n";

		SwapchainDetails mySwapchainSupport = QuerySwapchainSupport(mPhysicalDevice);

		vk::SurfaceFormatKHR mySurfaceFormat = ChooseSwapSurfaceFormat(mySwapchainSupport.Formats);
		vk::PresentModeKHR myPresentMode = ChooseSwapPresentMode(mySwapchainSupport.PresentModes);
		vk::Extent2D myExtent2D = ChooseSwapExtent(mySwapchainSupport.Capabilities);

		uint32_t myImageCount = mySwapchainSupport.Capabilities.minImageCount + 1;
		if (mySwapchainSupport.Capabilities.maxImageCount > 0 && myImageCount > mySwapchainSupport.Capabilities.maxImageCount)
			myImageCount = mySwapchainSupport.Capabilities.maxImageCount;

		vk::SwapchainCreateInfoKHR mySwapCreateInfo({}, mSurface, myImageCount, mySurfaceFormat.format, mySurfaceFormat.colorSpace, myExtent2D, 1, vk::ImageUsageFlagBits::eColorAttachment);

		QueueFamilyIndices myIndices = FindQueueFamilies(mPhysicalDevice);
		uint32_t myQueueFamilies[] = { (uint32_t)myIndices.Graphics, (uint32_t)myIndices.Present };

		std::cout << "Evaluating image sharing mode...\n";
		std::cout << "Using ";
		if (myIndices.Graphics != myIndices.Present)
		{
			std::cout << "Concurrent ";
			mySwapCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			mySwapCreateInfo.queueFamilyIndexCount = 2;
			mySwapCreateInfo.pQueueFamilyIndices = myQueueFamilies;
		}
		else
		{
			std::cout << "Exclusive ";
			mySwapCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			mySwapCreateInfo.queueFamilyIndexCount = NULL; // optional
			mySwapCreateInfo.pQueueFamilyIndices = nullptr; // optional
		}
		std::cout << "sharing mode.\n";

		mySwapCreateInfo.preTransform = mySwapchainSupport.Capabilities.currentTransform;
		mySwapCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		mySwapCreateInfo.presentMode = myPresentMode;
		mySwapCreateInfo.clipped = true;
		mySwapCreateInfo.oldSwapchain = nullptr;


		mSwapchain = mDevice.createSwapchainKHR(mySwapCreateInfo);
		mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);

		mSwapchainImageFormat = mySurfaceFormat.format;
		mSwapchainExtent2D = myExtent2D;

		std::cout << "Successfully created Vulkan Swapchain.\n\n";
	}

	void Context::CreateSurface()
	{
		std::cout << "Creating Vulkan surface...\n";

		/// NUCLEAR DANGER ///
		VkSurfaceKHR myTempSurface;

		glfwCreateWindowSurface(mInstance, mWindow.GetPtr(), nullptr, &myTempSurface);
		mSurface = myTempSurface;
		/// NUCLEAR DANGER ///
	}

	void Context::CreateLogicalDevice()
	{
		std::cout << "Creating logical Vulkan Device...\n";

		QueueFamilyIndices myIndices = FindQueueFamilies(mPhysicalDevice);

		std::vector<vk::DeviceQueueCreateInfo> myQueueCreateInfos;
		std::set<int> myUniqueQueueFamilies = { myIndices.Graphics, myIndices.Present };

		float myQueuePriority = 1.f;
		for (auto& iQueueFamily : myUniqueQueueFamilies)
		{
			vk::DeviceQueueCreateInfo myQueueCreateInfo({}, iQueueFamily, 1, &myQueuePriority);

			myQueueCreateInfos.push_back(myQueueCreateInfo);
		}

		vk::PhysicalDeviceFeatures myDeviceFeatures = {};

		vk::DeviceCreateInfo myDeviceCreateInfo;
		myDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(myQueueCreateInfos.size());
		myDeviceCreateInfo.pQueueCreateInfos = myQueueCreateInfos.data();
		myDeviceCreateInfo.pEnabledFeatures = &myDeviceFeatures;

		myDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
		myDeviceCreateInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

		if (VALIDATION_LAYERS_ENABLED)
		{
			myDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			myDeviceCreateInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
			myDeviceCreateInfo.enabledLayerCount = 0;

		mDevice = mPhysicalDevice.createDevice(myDeviceCreateInfo);

		mGraphicsQueue = mDevice.getQueue(myIndices.Graphics, 0);
		mPresentQueue = mDevice.getQueue(myIndices.Present, 0);

		std::cout << "Successfully created logical Vulkan Device.\ngg\n";
	}

	QueueFamilyIndices Context::FindQueueFamilies(vk::PhysicalDevice aDevice)
	{
		std::cout << "Searching for available queue families...\n";

		QueueFamilyIndices myIndices;

		std::vector<vk::QueueFamilyProperties> myQFamilies = aDevice.getQueueFamilyProperties();;

		for (uint16_t i = 0; i < myQFamilies.size(); ++i)
		{
			if (myQFamilies[i].queueCount > 0 && myQFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
				myIndices.Graphics = i;

			vk::Bool32 myPresentSupport = aDevice.getSurfaceSupportKHR(i, mSurface);

			if (myQFamilies[i].queueCount > QueueFamilyIndices::MIN_INDEX && myPresentSupport)
				myIndices.Present = i;

			if (myIndices.IsComplete())
				break;
		}
		return myIndices;
	}

	void Context::SelectPhysicalDevice()
	{
		std::cout << "Searching for viable physical device...\n";

		std::vector<vk::PhysicalDevice> myDevices = mInstance.enumeratePhysicalDevices();

		if (myDevices.empty())
			throw std::runtime_error("Failed to find any GPU with Vulkan support.");

		for (const auto& iDevice : myDevices)
		{
			if (IsDeviceSuitable(iDevice))
			{
				mPhysicalDevice = iDevice;
				break;
			}
		}

		if (mPhysicalDevice == vk::PhysicalDevice())
			throw std::runtime_error("Failed to find a suitable GPU for Vulkan.");

		std::cout << "Matching Vulkan-compatible GPU(s) successfully found.\n\n";
	}

	bool Context::IsDeviceSuitable(vk::PhysicalDevice aDevice)
	{
		//vk::PhysicalDeviceProperties myProperties;
		//vk::PhysicalDeviceFeatures myFeatures;
		//vkGetPhysicalDeviceProperties(aDevice, &myProperties);
		//vkGetPhysicalDeviceFeatures(aDevice, &myFeatures);

		// return myProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		// myFeatures.geometryShader;

		std::cout << "Evaluating device suitability...\n";

		QueueFamilyIndices myIndices = FindQueueFamilies(aDevice);

		bool isExtensionsSupported = CheckDeviceExtensionSupport(aDevice);
		bool isSwapchainAdequate = false;

		if (isExtensionsSupported)
		{
			SwapchainDetails mySwapchainSupport = QuerySwapchainSupport(aDevice);
			isSwapchainAdequate = !mySwapchainSupport.Formats.empty() && !mySwapchainSupport.PresentModes.empty();
		}

		return myIndices.IsComplete() && isExtensionsSupported && isSwapchainAdequate;
	}

	vk::SurfaceFormatKHR Context::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& anAvailableFormats)
	{
		std::cout << "Checking available Swapchain Surface Format...\n";
		if (anAvailableFormats.size() == 1 && anAvailableFormats[0].format == vk::Format::eUndefined)
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

		for (const auto& iAvailableFormat : anAvailableFormats)
		{
			if (iAvailableFormat.format == vk::Format::eB8G8R8A8Unorm && iAvailableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return iAvailableFormat;
		}

		return anAvailableFormats[0];
	}

	vk::PresentModeKHR Context::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> anAvailablePresentModes)
	{
		std::cout << "Determining optimal Swapchain Present mode...\n";
		vk::PresentModeKHR myBestMode = vk::PresentModeKHR::eFifo;

		for (const auto& iAvailableMode : anAvailablePresentModes)
		{
			if (iAvailableMode == vk::PresentModeKHR::eMailbox)
				return iAvailableMode;
			else if (iAvailableMode == vk::PresentModeKHR::eImmediate)
				myBestMode = iAvailableMode;
		}

		return myBestMode;
	}

	vk::Extent2D Context::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& aCapabilities)
	{
		std::cout << "Choosing Swapchain Extent...\n";

		if (aCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return aCapabilities.currentExtent;
		else
		{
			auto mySize = (Size<uint32_t>)mWindow.GetSize();

			vk::Extent2D myActualExtent = { mySize.Width, mySize.Height };

			myActualExtent.width = std::max(aCapabilities.minImageExtent.width, std::min(aCapabilities.maxImageExtent.width, myActualExtent.width));
			myActualExtent.height = std::max(aCapabilities.minImageExtent.height, std::min(aCapabilities.maxImageExtent.height, myActualExtent.height));

			return myActualExtent;
		}
	}

	std::vector<const char*> Context::GetRequiredExtensions()
	{
		std::cout << "Fetching required extensions...\n";
		uint32_t myGlfwExtensionCount = 0;
		const char** myGlfwExtensions;
		myGlfwExtensions = glfwGetRequiredInstanceExtensions(&myGlfwExtensionCount);

		std::vector<const char*> myExtensions(myGlfwExtensions, myGlfwExtensions + myGlfwExtensionCount);

		if (VALIDATION_LAYERS_ENABLED)
			myExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		return myExtensions;
	}

	bool Context::CheckDeviceExtensionSupport(vk::PhysicalDevice aDevice)
	{
		std::cout << "Checking device extension support...\n";

		std::vector<vk::ExtensionProperties> myAvailableExtensions = aDevice.enumerateDeviceExtensionProperties();

		std::set<std::string> myRequiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

		for (const auto& iExtension : myAvailableExtensions)
			myRequiredExtensions.erase(iExtension.extensionName);

		return myRequiredExtensions.empty();
	}

	void Context::CreateInstance()
	{
		if (VALIDATION_LAYERS_ENABLED && !CheckValidationLayerSupport())
			throw std::runtime_error("Validation layers requested, but not available.");

		vk::ApplicationInfo myAppInfo("Binch", VK_MAKE_VERSION(1, 0, 0), "Nyxa3D", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

		auto myExtensions = GetRequiredExtensions();

		vk::InstanceCreateInfo myCreateInfo({}, &myAppInfo);
		myCreateInfo.pApplicationInfo = &myAppInfo;

		myCreateInfo.enabledExtensionCount = static_cast<uint32_t>(myExtensions.size());
		myCreateInfo.ppEnabledExtensionNames = myExtensions.data();

		if (VALIDATION_LAYERS_ENABLED)
		{
			myCreateInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			myCreateInfo.ppEnabledLayerNames = mValidationLayers.data();
		}
		else
			myCreateInfo.enabledLayerCount = 0;

		std::vector<vk::ExtensionProperties> myExtensionProperties = vk::enumerateInstanceExtensionProperties();

		std::cout << "Available extensions:\n";
		for (const auto& iProperty : myExtensionProperties)
		{
			std::cout << "\t" << iProperty.extensionName << "\n";
		}
		std::cout << myExtensionProperties.size() << " extensions found in total.\n\n";

		std::cout << "Checking for GLFW extension compatibility...\n";

		// check whether extensions required by GLFW are in available extension list
		for (size_t i = 0; i < myExtensions.size(); ++i)
		{
			bool isFound = false;
			for (const auto& iProperty : myExtensionProperties)
			{
				if (strcmp(myExtensions[i], iProperty.extensionName) == 0)
				{
					isFound = true;
					break;
				}
			}

			if (!isFound)
				throw std::runtime_error("Could not find required GLFW extension for Vulkan on this system.");

			std::cout << "        " << myExtensions[i] << " found.\n";
		}

		std::cout << "Extension check successful.\n\n";

		mInstance = vk::createInstance(myCreateInfo);
	}

	SwapchainDetails Context::QuerySwapchainSupport(vk::PhysicalDevice aDevice)
	{
		std::cout << "Querying Swapchain support...\n";

		SwapchainDetails myDetails;
		myDetails.Capabilities = aDevice.getSurfaceCapabilitiesKHR(mSurface);
		myDetails.Formats = aDevice.getSurfaceFormatsKHR(mSurface);
		myDetails.PresentModes = aDevice.getSurfacePresentModesKHR(mSurface);

		return myDetails;
	}

	bool Context::CheckValidationLayerSupport()
	{
		std::cout << "Checking for validation layer compatibility...\n";

		std::vector<vk::LayerProperties> myAvailableLayers = vk::enumerateInstanceLayerProperties();

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
				std::cout << iLayerName << " is not supported.\n";
				return false;
			}

			std::cout << iLayerName << " is supported.\n";
		}

		std::cout << "All validation layers are supported.\n\n";

		return true;
	}

	void Context::CleanupSwapchain()
	{
		for (auto iBuffer : mSwapchainFramebuffers)
			mDevice.destroyFramebuffer(iBuffer);

		mDevice.freeCommandBuffers(mCommandPool, mCommandBuffers);
		mPipeline.Destroy();

		for (auto iView : mSwapchainImageViews)
			mDevice.destroyImageView(iView);

		mDevice.destroySwapchainKHR(mSwapchain);
	}

	Context::~Context()
	{
		std::cout << "Terminating program...\n";

		CleanupSwapchain();

		mDevice.destroySemaphore(mRenderFinishedSema);
		mDevice.destroySemaphore(mImageAvailableSema);
		mDevice.destroyCommandPool(mCommandPool);
		mDevice.destroy();

		mInstance.destroySurfaceKHR(mSurface);
		mDebugger.Destroy();

		mInstance.destroy();

		std::cout << "Application terminated. Exiting.\n";
	}

}