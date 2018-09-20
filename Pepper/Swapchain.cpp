#include "Swapchain.h"
#include "Context.h"
#include "Callbacks.h"
#include "DbgMsgr.h"

#include <iostream>

namespace ppr
{
	Swapchain::Swapchain(const vk::Device& aDevice, 
						const Window& aWindow, 
						const vk::Instance& anInstance, 
						const vk::PhysicalDevice& aPhysicalDevice)
		: mDevice(aDevice)
		, mPipeline(aDevice, mExtent2D)
		, mVertexBuffer(aDevice)
		, mWindow(aWindow)
		, mInstance(anInstance)
		, mPhysicalDevice(aPhysicalDevice)
	{}

	Swapchain::~Swapchain()
	{
		if (!mDestroyed)
			Destroy();
	}

	void Swapchain::Init()
	{
		WndCallbacks.Add(&Swapchain::OnWindowResize, this, CallType::Resize);
		Create();
		CreateImageViews();
		CreateRenderPass();
		mPipeline.Create();
		CreateFrameBuffers();
		CreateCommandPool();
		mVertexBuffer.Create(mPhysicalDevice);
		CreateCommandBuffers();
		CreateSemaphores();

		mInitialised = true;
	}

	void Swapchain::OnWindowResize()
	{
		this->Recreate();
		this->Draw();
	}

	void Swapchain::Draw()
	{
		vk::ResultValue<uint32_t> myResultPair = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvailableSema, nullptr);

		uint32_t myImageIndex = myResultPair.value;
		vk::Result myResult = myResultPair.result;

		if (Print(myResult) == vk::Result::eErrorOutOfDateKHR)
		{
			this->Recreate();
			return;
		}

		else if (myResult != vk::Result::eSuccess && myResult != vk::Result::eSuboptimalKHR)
			throw Error("Failed to acquire swapchain image.", Error::Code::SWAPCHAIN_IMAGE_NOT_ACQUIRED);

		vk::Semaphore myWaitSemaphores[] = { mImageAvailableSema };
		vk::Semaphore mySignalSemaphores[] = { mRenderFinishedSema };
		vk::PipelineStageFlags myWaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo mySubmitInfo(1, myWaitSemaphores, myWaitStages, 1, &mCommandBuffers[myImageIndex], 1, mySignalSemaphores);

		mGraphicsQueue.submit(mySubmitInfo, nullptr);

		vk::SwapchainKHR mySwapchains[] = { mSwapchain };
		vk::PresentInfoKHR myPresentInfo(1, mySignalSemaphores, 1, mySwapchains, &myImageIndex);

		myResult = mPresentQueue.presentKHR(myPresentInfo);

		if (Print(myResult) == vk::Result::eErrorOutOfDateKHR || myResult == vk::Result::eSuboptimalKHR)
		{
			this->Recreate();
			return;
		}
		else if (myResult != vk::Result::eSuccess)
			throw Error("Failed to present swapchain image.", Error::Code::SWAPCHAIN_IMAGE_PRESENT_ERROR);

		mPresentQueue.waitIdle();
	}

	void Swapchain::Create()
	{
		if (!mInitialised)
			DbgPrint("Creating Swapchain...\n");
		else
			VerbosePrint("Creating Swapchain...\n");

		SwapchainDetails mySwapchainSupport = QuerySupport(mPhysicalDevice);

		vk::SurfaceFormatKHR mySurfaceFormat = ChooseSurfaceFormat(mySwapchainSupport.Formats);
		vk::PresentModeKHR myPresentMode = ChoosePresentMode(mySwapchainSupport.PresentModes);
		vk::Extent2D myExtent2D = ChooseExtent(mySwapchainSupport.Capabilities);

		uint32_t myImageCount = mySwapchainSupport.Capabilities.minImageCount + 1;
		if (mySwapchainSupport.Capabilities.maxImageCount > 0 && myImageCount > mySwapchainSupport.Capabilities.maxImageCount)
			myImageCount = mySwapchainSupport.Capabilities.maxImageCount;

		vk::SwapchainCreateInfoKHR mySwapCreateInfo({}, mSurface, myImageCount, mySurfaceFormat.format, mySurfaceFormat.colorSpace, myExtent2D, 1, vk::ImageUsageFlagBits::eColorAttachment);

		QueueFamilyIndices myIndices = FindQueueFamilies(mPhysicalDevice);
		uint32_t myQueueFamilies[] = { (uint32_t)myIndices.Graphics, (uint32_t)myIndices.Present };

		VerbosePrint("Evaluating image sharing mode...\n");
		VerbosePrint("Using ");
		if (myIndices.Graphics != myIndices.Present)
		{
			VerbosePrint("Concurrent ");
			mySwapCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			mySwapCreateInfo.queueFamilyIndexCount = 2;
			mySwapCreateInfo.pQueueFamilyIndices = myQueueFamilies;
		}
		else
		{
			VerbosePrint("Exclusive ");
			mySwapCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			mySwapCreateInfo.queueFamilyIndexCount = NULL; // optional
			mySwapCreateInfo.pQueueFamilyIndices = nullptr; // optional
		}
		VerbosePrint("sharing mode.\n");

		mySwapCreateInfo.preTransform = mySwapchainSupport.Capabilities.currentTransform;
		mySwapCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		mySwapCreateInfo.presentMode = myPresentMode;
		mySwapCreateInfo.clipped = true;
		mySwapCreateInfo.oldSwapchain = nullptr;


		mSwapchain = mDevice.createSwapchainKHR(mySwapCreateInfo);
		mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);

		mImageFormat = mySurfaceFormat.format;
		mExtent2D = myExtent2D;

		VerbosePrint("Successfully created Swapchain.\n");
	}

	void Swapchain::CreateImageViews()
	{
		mImageViews.resize(mSwapchainImages.size());

		for (size_t i = 0; i < mSwapchainImages.size(); ++i)
		{
			VerbosePrint("Creating Swapchain image view...\n");

			vk::ImageSubresourceRange mySubRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			vk::ImageViewCreateInfo myCreateInfo({}, mSwapchainImages[i], vk::ImageViewType::e2D, mImageFormat, vk::ComponentMapping(), mySubRange);

			mImageViews[i] = mDevice.createImageView(myCreateInfo);
		}

		VerbosePrint("Image views created.\n\n");
	}

	void Swapchain::CreateWindowSurface()
	{
		VerbosePrint("Creating Vulkan surface...\n");

		mSurface = mWindow.CreateSurface(mInstance);
	}

	QueueFamilyIndices Swapchain::FindQueueFamilies(vk::PhysicalDevice aDevice)
	{
		VerbosePrint("Searching for available queue families...\n");

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

	void Swapchain::DestroyWindowSurface()
	{
		mInstance.destroySurfaceKHR(mSurface);
	}

	vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& anAvailableFormats)
	{
		VerbosePrint("Checking available Swapchain Surface Format...\n");
		if (anAvailableFormats.size() == 1 && anAvailableFormats[0].format == vk::Format::eUndefined)
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

		for (const auto& iAvailableFormat : anAvailableFormats)
		{
			if (iAvailableFormat.format == vk::Format::eB8G8R8A8Unorm && iAvailableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return iAvailableFormat;
		}

		return anAvailableFormats[0];
	}

	vk::PresentModeKHR Swapchain::ChoosePresentMode(const std::vector<vk::PresentModeKHR> anAvailablePresentModes)
	{
		VerbosePrint("Determining optimal Swapchain Present mode...\n");
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

	vk::Extent2D Swapchain::ChooseExtent(const vk::SurfaceCapabilitiesKHR& aCapabilities)
	{
		VerbosePrint("Choosing Swapchain Extent...\n");

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

	SwapchainDetails Swapchain::QuerySupport(vk::PhysicalDevice aDevice)
	{
		VerbosePrint("Querying Swapchain support...\n");

		SwapchainDetails myDetails;
		myDetails.Capabilities = aDevice.getSurfaceCapabilitiesKHR(mSurface);
		myDetails.Formats = aDevice.getSurfaceFormatsKHR(mSurface);
		myDetails.PresentModes = aDevice.getSurfacePresentModesKHR(mSurface);

		return myDetails;
	}

	vk::Queue& Swapchain::GetGraphicsQueue()
	{
		return mGraphicsQueue;
	}

	vk::Queue& Swapchain::GetPresentQueue()
	{
		return mPresentQueue;
	}

	void Swapchain::Cleanup()
	{
		for (auto iBuffer : mFramebuffers)
			mDevice.destroyFramebuffer(iBuffer);

		mDevice.freeCommandBuffers(mCommandPool, mCommandBuffers);

		mPipeline.Destroy();
		mVertexBuffer.Destroy();

		for (auto iView : mImageViews)
			mDevice.destroyImageView(iView);

		mDevice.destroySwapchainKHR(mSwapchain);

		mDestroyed = true;
	}

	void Swapchain::Destroy()
	{
		if (!mDestroyed)
			Cleanup();

		mDevice.destroySemaphore(mRenderFinishedSema);
		mDevice.destroySemaphore(mImageAvailableSema);
		mDevice.destroyCommandPool(mCommandPool);
	}

	void Swapchain::CreateFrameBuffers()
	{
		VerbosePrint("Creating framebuffers...\n");

		mFramebuffers.resize(mImageViews.size());

		for (size_t i = 0; i < mImageViews.size(); ++i)
		{
			vk::ImageView myAttachments[] = { mImageViews[i] };

			vk::FramebufferCreateInfo myFramebufferInfo({}, mPipeline.GetRenderPass(), 1, myAttachments, mExtent2D.width, mExtent2D.height, 1);

			mFramebuffers[i] = mDevice.createFramebuffer(myFramebufferInfo);
		}

		VerbosePrint("Finished creating framebuffers.\n");
	}

	void Swapchain::Recreate()
	{
		auto mySize = mWindow.GetSize();

		if (mySize.Width == 0 || mySize.Height == 0)
			return;

		mDevice.waitIdle();

		Cleanup();

		Create();
		CreateImageViews();
		CreateRenderPass();
		mPipeline.Create();
		CreateFrameBuffers();
		mVertexBuffer.Create(mPhysicalDevice);
		CreateCommandBuffers();

		mDestroyed = false;
	}

	void Swapchain::CreateSemaphores()
	{
		vk::SemaphoreCreateInfo mySemaInfo = {};

		mImageAvailableSema = mDevice.createSemaphore(mySemaInfo);
		mRenderFinishedSema = mDevice.createSemaphore(mySemaInfo);

		// No error checking?
	}

	void Swapchain::CreateCommandBuffers()
	{
		VerbosePrint("Creating Command Buffers...\n");

		mCommandBuffers.resize(mFramebuffers.size());

		vk::CommandBufferAllocateInfo myAllocInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, (uint32_t)mCommandBuffers.size());
		mCommandBuffers = mDevice.allocateCommandBuffers(myAllocInfo);

		for (size_t i = 0; i < mCommandBuffers.size(); ++i)
		{
			vk::CommandBufferBeginInfo myBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			mCommandBuffers[i].begin(myBeginInfo);

			vk::ClearValue myClearColor(vk::ClearColorValue(std::array<float, 4>({ 0.f, 0.f, 0.f, 1.f })));
			vk::Rect2D myRect({ 0, 0 }, mExtent2D);
			vk::RenderPassBeginInfo myRenderPassInfo(mPipeline.GetRenderPass(), mFramebuffers[i], myRect, 1, &myClearColor);

			mCommandBuffers[i].beginRenderPass(myRenderPassInfo, vk::SubpassContents::eInline);
			mCommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline.GetRef());

			vk::Buffer myVertexBuffers[] = { mVertexBuffer.Get() };
			vk::DeviceSize myOffsets[] = { 0 };
			mCommandBuffers[i].bindVertexBuffers(0, 1, myVertexBuffers, myOffsets);

			// OWO WHAT'S THIS
			mCommandBuffers[i].draw((uint32_t)mVertexBuffer.GetVertexArray().size(), 1, 0, 0);
			mCommandBuffers[i].endRenderPass();
			mCommandBuffers[i].end();
		}
	}

	void Swapchain::CreateCommandPool()
	{
		VerbosePrint("Creating Command Pool...\n");

		QueueFamilyIndices myQueueFamilyIndices = FindQueueFamilies(mPhysicalDevice);

		vk::CommandPoolCreateInfo myPoolInfo({}, myQueueFamilyIndices.Graphics);
		mCommandPool = mDevice.createCommandPool(myPoolInfo);
	}

	void Swapchain::CreateRenderPass()
	{
		vk::AttachmentDescription myColorAttachment({}, mImageFormat, vk::SampleCountFlagBits::e1,
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
}