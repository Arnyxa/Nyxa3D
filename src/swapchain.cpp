#include "swapchain.hpp"
#include "context.hpp"
#include "callbacks.hpp"


#include <iostream>

namespace ppr
{
	swapchain::swapchain(const vk::Device& a_device, 
						const window& a_window, 
						const vk::Instance& an_instance, 
						const vk::PhysicalDevice& a_physical_device)
		: m_device(a_device)
		, m_pipeline(a_device, m_extent2D)
		, m_vertex_buffer(a_device)
		, m_window(a_window)
		, m_instance(an_instance)
		, m_physical_device(a_physical_device)
	{}

	swapchain::~swapchain()
	{
		if (!m_destroyed)
			destroy();
	}

	void swapchain::init()
	{
		wndcall.add(&swapchain::on_window_resize, this, call_type::RESIZE);
		create();
		create_imageviews();
		create_renderpass();
		m_pipeline.create();
		create_framebuffers();
		create_commandpool();
		m_vertex_buffer.create(m_physical_device);
		create_commandbuffers();
		create_semaphores();

		m_initialized = true;
	}

	void swapchain::on_window_resize()
	{
		this->recreate();
		this->draw();
	}

	void swapchain::draw()
	{
		vk::ResultValue<uint32_t> myResultPair = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_sema_image_available, nullptr);

		uint32_t myImageIndex = myResultPair.value;
		vk::Result myResult = myResultPair.result;

		if (Print(myResult) == vk::Result::eErrorOutOfDateKHR)
		{
			this->recreate();
			return;
		}

		else if (myResult != vk::Result::eSuccess && myResult != vk::Result::eSuboptimalKHR)
			throw Error("Failed to acquire swapchain image.", Error::Code::SWAPCHAIN_IMAGE_NOT_ACQUIRED);

		vk::Semaphore myWaitSemaphores[] = { m_sema_image_available };
		vk::Semaphore mySignalSemaphores[] = { m_sema_render_finished };
		vk::PipelineStageFlags myWaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo mySubmitInfo(1, myWaitSemaphores, myWaitStages, 1, &m_commandbuffers[myImageIndex], 1, mySignalSemaphores);

		m_queue_graphics.submit(mySubmitInfo, nullptr);

		vk::SwapchainKHR mySwapchains[] = { m_swapchain };
		vk::PresentInfoKHR myPresentInfo(1, mySignalSemaphores, 1, mySwapchains, &myImageIndex);

		myResult = m_queue_present.presentKHR(myPresentInfo);

		if (Print(myResult) == vk::Result::eErrorOutOfDateKHR || myResult == vk::Result::eSuboptimalKHR)
		{
			this->recreate();
			return;
		}
		else if (myResult != vk::Result::eSuccess)
			throw Error("Failed to present swapchain image.", Error::Code::SWAPCHAIN_IMAGE_PRESENT_ERROR);

		m_queue_present.waitIdle();
	}

	void swapchain::create()
	{
		if (!m_initialized)
			printf("Creating swapchain...\n");
		else
			printf("Creating swapchain...\n");

		swapchain_support mySwapchainSupport = query_support(m_physical_device);

		vk::SurfaceFormatKHR mySurfaceFormat = choose_surface_format(mySwapchainSupport.formats);
		vk::PresentModeKHR myPresentMode = choose_present_mode(mySwapchainSupport.present_modes);
		vk::Extent2D myExtent2D = choose_extent(mySwapchainSupport.capabilities);

		uint32_t myImageCount = mySwapchainSupport.capabilities.minImageCount + 1;
		if (mySwapchainSupport.capabilities.maxImageCount > 0 && myImageCount > mySwapchainSupport.capabilities.maxImageCount)
			myImageCount = mySwapchainSupport.capabilities.maxImageCount;

		vk::SwapchainCreateInfoKHR mySwapCreateInfo({}, m_surface, myImageCount, mySurfaceFormat.format, mySurfaceFormat.colorSpace, myExtent2D, 1, vk::ImageUsageFlagBits::eColorAttachment);

		queue_families family_indices = find_queue_families(m_physical_device);
		uint32_t myQueueFamilies[] = { (uint32_t)family_indices.graphics, (uint32_t)family_indices.present };

		printf("Evaluating image sharing mode...\n");
		printf("Using ");
		if (family_indices.graphics != family_indices.present)
		{
			printf("Concurrent ");
			mySwapCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			mySwapCreateInfo.queueFamilyIndexCount = 2;
			mySwapCreateInfo.pQueueFamilyIndices = myQueueFamilies;
		}
		else
		{
			printf("Exclusive ");
			mySwapCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			mySwapCreateInfo.queueFamilyIndexCount = NULL; // optional
			mySwapCreateInfo.pQueueFamilyIndices = nullptr; // optional
		}
		printf("sharing mode.\n");

		mySwapCreateInfo.preTransform = mySwapchainSupport.capabilities.currentTransform;
		mySwapCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		mySwapCreateInfo.presentMode = myPresentMode;
		mySwapCreateInfo.clipped = true;
		mySwapCreateInfo.oldSwapchain = nullptr;


		m_swapchain = m_device.createSwapchainKHR(mySwapCreateInfo);
		m_images = m_device.getSwapchainImagesKHR(m_swapchain);

		m_image_format = mySurfaceFormat.format;
		m_extent2D = myExtent2D;

		printf("Successfully created swapchain.\n");
	}

	void swapchain::create_imageviews()
	{
		m_image_views.resize(m_images.size());

		for (size_t i = 0; i < m_images.size(); ++i)
		{
			printf("Creating swapchain image view...\n");

			vk::ImageSubresourceRange mySubRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			vk::ImageViewCreateInfo createinfo({}, m_images[i], vk::ImageViewType::e2D, m_image_format, vk::ComponentMapping(), mySubRange);

			m_image_views[i] = m_device.createImageView(createinfo);
		}

		printf("Image views created.\n\n");
	}

	void swapchain::create_window_surface()
	{
		printf("Creating Vulkan surface...\n");

		m_surface = m_window.create_surface(m_instance);
	}

	queue_families swapchain::find_queue_families(vk::PhysicalDevice a_device)
	{
		printf("Searching for available queue families...\n");

		queue_families family_indices;

		std::vector<vk::QueueFamilyProperties> myQFamilies = a_device.getQueueFamilyProperties();;

		for (uint16_t i = 0; i < myQFamilies.size(); ++i)
		{
			if (myQFamilies[i].queueCount > 0 && myQFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
				family_indices.graphics = i;

			vk::Bool32 myPresentSupport = a_device.getSurfaceSupportKHR(i, m_surface);

			if (myQFamilies[i].queueCount > queue_families::MIN_INDEX && myPresentSupport)
				family_indices.present = i;

			if (family_indices.IsComplete())
				break;
		}
		return family_indices;
	}

	void swapchain::destroy_window_surface()
	{
		m_instance.destroySurfaceKHR(m_surface);
	}

	vk::SurfaceFormatKHR swapchain::choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& an_available_formats)
	{
		printf("Checking available swapchain Surface Format...\n");
		if (an_available_formats.size() == 1 && an_available_formats[0].format == vk::Format::eUndefined)
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

		for (const auto& iAvailableFormat : an_available_formats)
		{
			if (iAvailableFormat.format == vk::Format::eB8G8R8A8Unorm && iAvailableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return iAvailableFormat;
		}

		return an_available_formats[0];
	}

	vk::PresentModeKHR swapchain::choose_present_mode(const std::vector<vk::PresentModeKHR> an_available_modes)
	{
		printf("Determining optimal swapchain present mode...\n");
		vk::PresentModeKHR myBestMode = vk::PresentModeKHR::eFifo;

		for (const auto& iAvailableMode : an_available_modes)
		{
			if (iAvailableMode == vk::PresentModeKHR::eMailbox)
				return iAvailableMode;
			else if (iAvailableMode == vk::PresentModeKHR::eImmediate)
				myBestMode = iAvailableMode;
		}

		return myBestMode;
	}

	vk::Extent2D swapchain::choose_extent(const vk::SurfaceCapabilitiesKHR& a_capabilities)
	{
		printf("Choosing swapchain Extent...\n");

		if (a_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return a_capabilities.currentExtent;
		else
		{
			auto mySize = (size<uint32_t>)m_window.get_size();

			vk::Extent2D myActualExtent = { mySize.width, mySize.height };

			myActualExtent.width = std::max(a_capabilities.minImageExtent.width, std::min(a_capabilities.maxImageExtent.width, myActualExtent.width));
			myActualExtent.height = std::max(a_capabilities.minImageExtent.height, std::min(a_capabilities.maxImageExtent.height, myActualExtent.height));

			return myActualExtent;
		}
	}

	swapchain_support swapchain::query_support(vk::PhysicalDevice a_device)
	{
		printf("Querying swapchain support...\n");

		swapchain_support myDetails;
		myDetails.capabilities = a_device.getSurfaceCapabilitiesKHR(m_surface);
		myDetails.formats = a_device.getSurfaceFormatsKHR(m_surface);
		myDetails.present_modes = a_device.getSurfacePresentModesKHR(m_surface);

		return myDetails;
	}

	vk::Queue& swapchain::graphics_queue()
	{
		return m_queue_graphics;
	}

	vk::Queue& swapchain::present_queue()
	{
		return m_queue_present;
	}

	void swapchain::cleanup()
	{
		for (auto iBuffer : m_framebuffers)
			m_device.destroyFramebuffer(iBuffer);

		m_device.freeCommandBuffers(m_commandpool, m_commandbuffers);

		m_pipeline.destroy();
		m_vertex_buffer.destroy();

		for (auto iView : m_image_views)
			m_device.destroyImageView(iView);

		m_device.destroySwapchainKHR(m_swapchain);

		m_destroyed = true;
	}

	void swapchain::destroy()
	{
		if (!m_destroyed)
			cleanup();

		m_device.destroySemaphore(m_sema_render_finished);
		m_device.destroySemaphore(m_sema_image_available);
		m_device.destroyCommandPool(m_commandpool);
	}

	void swapchain::create_framebuffers()
	{
		printf("Creating framebuffers...\n");

		m_framebuffers.resize(m_image_views.size());

		for (size_t i = 0; i < m_image_views.size(); ++i)
		{
			vk::ImageView myAttachments[] = { m_image_views[i] };

			vk::FramebufferCreateInfo myFramebufferInfo({}, m_pipeline.get_renderpass(), 1, myAttachments, m_extent2D.width, m_extent2D.height, 1);

			m_framebuffers[i] = m_device.createFramebuffer(myFramebufferInfo);
		}

		printf("Finished creating framebuffers.\n");
	}

	void swapchain::recreate()
	{
		auto mySize = m_window.get_size();

		if (mySize.width == 0 || mySize.height == 0)
			return;

		m_device.waitIdle();

		cleanup();

		create();
		create_imageviews();
		create_renderpass();
		m_pipeline.create();
		create_framebuffers();
		m_vertex_buffer.create(m_physical_device);
		create_commandbuffers();

		m_destroyed = false;
	}

	void swapchain::create_semaphores()
	{
		vk::SemaphoreCreateInfo mySemaInfo = {};

		m_sema_image_available = m_device.createSemaphore(mySemaInfo);
		m_sema_render_finished = m_device.createSemaphore(mySemaInfo);

		// No error checking?
	}

	void swapchain::create_commandbuffers()
	{
		printf("Creating Command Buffers...\n");

		m_commandbuffers.resize(m_framebuffers.size());

		vk::CommandBufferAllocateInfo myAllocInfo(m_commandpool, vk::CommandBufferLevel::ePrimary, (uint32_t)m_commandbuffers.size());
		m_commandbuffers = m_device.allocateCommandBuffers(myAllocInfo);

		for (size_t i = 0; i < m_commandbuffers.size(); ++i)
		{
			vk::CommandBufferBeginInfo myBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			m_commandbuffers[i].begin(myBeginInfo);

			vk::ClearValue myClearColor(vk::ClearColorValue(std::array<float, 4>({ 0.f, 0.f, 0.f, 1.f })));
			vk::Rect2D myRect({ 0, 0 }, m_extent2D);
			vk::RenderPassBeginInfo myRenderPassInfo(m_pipeline.get_renderpass(), m_framebuffers[i], myRect, 1, &myClearColor);

			m_commandbuffers[i].beginRenderPass(myRenderPassInfo, vk::SubpassContents::eInline);
			m_commandbuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			vk::Buffer myVertexBuffers[] = { m_vertex_buffer.get() };
			vk::DeviceSize myOffsets[] = { 0 };
			m_commandbuffers[i].bindVertexBuffers(0, 1, myVertexBuffers, myOffsets);

			// OWO WHAT'S THIS
			m_commandbuffers[i].draw((uint32_t)m_vertex_buffer.get_vertex_array().size(), 1, 0, 0);
			m_commandbuffers[i].endRenderPass();
			m_commandbuffers[i].end();
		}
	}

	void swapchain::create_commandpool()
	{
		printf("Creating Command Pool...\n");

		queue_families myQueueFamilyIndices = find_queue_families(m_physical_device);

		vk::CommandPoolCreateInfo myPoolInfo({}, myQueueFamilyIndices.graphics);
		m_commandpool = m_device.createCommandPool(myPoolInfo);
	}

	void swapchain::create_renderpass()
	{
		vk::AttachmentDescription myColorAttachment({}, m_image_format, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference myColorAttachRef(0, vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription mySubpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &myColorAttachRef);

		vk::SubpassDependency myDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlags(),
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo myRenderPassInfo({}, 1, &myColorAttachment, 1, &mySubpass, 1, &myDependency);

		m_pipeline.get_renderpass() = m_device.createRenderPass(myRenderPassInfo);
	}
}