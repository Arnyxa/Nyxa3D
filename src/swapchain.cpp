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
        const vk::ResultValue<uint32_t> result_pair = m_device.acquireNextImageKHR(m_swapchain, 
                                                                                   UINT64_MAX, 
                                                                                   m_sema_image_available, 
                                                                                   nullptr);
        const uint32_t image_index = result_pair.value;

		if (print(result_pair.result) == vk::Result::eErrorOutOfDateKHR)
		{
			this->recreate();
			return;
		}
		else if (result_pair.result != vk::Result::eSuccess && result_pair.result != vk::Result::eSuboptimalKHR)
			throw Error("Failed to acquire swapchain image.", Error::Code::SWAPCHAIN_IMAGE_NOT_ACQUIRED);

		const vk::Semaphore sema_wait[] = { m_sema_image_available };
		const vk::Semaphore sema_signal[] = { m_sema_render_finished };
        const vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        const vk::SubmitInfo submit_info(1, 
                                         sema_wait, 
                                         wait_stages, 1, 
                                        &m_commandbuffers[image_index], 1, 
                                         sema_signal);

		m_queue_graphics.submit(submit_info, nullptr);

        const vk::SwapchainKHR swapchains[] = { m_swapchain };
        const vk::PresentInfoKHR present_info(1, sema_signal, 1, swapchains, &image_index);

		const auto result = m_queue_present.presentKHR(present_info);

		if (print(result) == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
		{
			this->recreate();
			return;
		}
		else if (result != vk::Result::eSuccess)
			throw Error("Failed to present swapchain image.", Error::Code::SWAPCHAIN_IMAGE_PRESENT_ERROR);

		m_queue_present.waitIdle();
	}

	void swapchain::create()
	{
		if (!m_initialized)
			printf("Creating swapchain...\n");
		else
			printf("Creating swapchain...\n");

		const swapchain_support support_details = query_support(m_physical_device);

        const vk::SurfaceFormatKHR surface_format = choose_surface_format(support_details.formats);
        const vk::PresentModeKHR present_mode = choose_present_mode(support_details.present_modes);
        const vk::Extent2D extent2D = choose_extent(support_details.capabilities);

		uint32_t image_count = 1 + support_details.capabilities.minImageCount;

		if (support_details.capabilities.maxImageCount > 0 && image_count > support_details.capabilities.maxImageCount)
			image_count = support_details.capabilities.maxImageCount;

        const queue_families family_indices = find_queue_families(m_physical_device);
        const uint32_t queue_families[] = { (uint32_t)family_indices.graphics, (uint32_t)family_indices.present };

        vk::SwapchainCreateInfoKHR swapchain_createinfo({}, m_surface, 
                                                        image_count, 
                                                        surface_format.format, 
                                                        surface_format.colorSpace, 
                                                        extent2D, 1, 
                                                        vk::ImageUsageFlagBits::eColorAttachment);

		printf("Evaluating image sharing mode...\n");
		printf("Using ");
		if (family_indices.graphics != family_indices.present)
		{
			printf("Concurrent ");
			swapchain_createinfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchain_createinfo.queueFamilyIndexCount = 2;
			swapchain_createinfo.pQueueFamilyIndices = queue_families;
		}
		else
		{
			printf("Exclusive ");
			swapchain_createinfo.imageSharingMode = vk::SharingMode::eExclusive;
			swapchain_createinfo.queueFamilyIndexCount = NULL; // optional
			swapchain_createinfo.pQueueFamilyIndices = nullptr; // optional
		}
		printf("sharing mode.\n");

		swapchain_createinfo.preTransform = support_details.capabilities.currentTransform;
		swapchain_createinfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_createinfo.presentMode = present_mode;
		swapchain_createinfo.clipped = true;
		swapchain_createinfo.oldSwapchain = nullptr;


		m_swapchain = m_device.createSwapchainKHR(swapchain_createinfo);
		m_images = m_device.getSwapchainImagesKHR(m_swapchain);

		m_image_format = surface_format.format;
		m_extent2D = extent2D;

		printf("Successfully created swapchain.\n");
	}

	void swapchain::create_imageviews()
	{
		m_image_views.resize(m_images.size());

		for (size_t i = 0; i < m_images.size(); ++i)
		{
			printf("Creating swapchain image view...\n");

            const vk::ImageSubresourceRange subresource_range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
            const vk::ImageViewCreateInfo createinfo({}, m_images[i], vk::ImageViewType::e2D, m_image_format, vk::ComponentMapping(), subresource_range);

			m_image_views[i] = m_device.createImageView(createinfo);
		}

		printf("Image views created.\n\n");
	}

	void swapchain::create_window_surface()
	{
		printf("Creating Vulkan surface...\n");

		m_surface = m_window.create_surface(m_instance);
	}

	queue_families swapchain::find_queue_families(const vk::PhysicalDevice& a_device) const
	{
		printf("Searching for available queue families...\n");

        const std::vector<vk::QueueFamilyProperties> queue_fam_properties = a_device.getQueueFamilyProperties();

        queue_families family_indices;

		for (uint16_t i = 0; i < queue_fam_properties.size(); ++i)
		{
			if (queue_fam_properties[i].queueCount > 0 
             && queue_fam_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
				family_indices.graphics = i;

			vk::Bool32 present_support = a_device.getSurfaceSupportKHR(i, m_surface);

			if (queue_fam_properties[i].queueCount > queue_families::MIN_INDEX 
             && present_support)
				family_indices.present = i;

			if (family_indices.IsComplete())
				break;
		}
		return family_indices;
	}

	void swapchain::destroy_window_surface() const
	{
		m_instance.destroySurfaceKHR(m_surface);
	}

	vk::SurfaceFormatKHR swapchain::choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& an_available_formats) const
	{
		printf("Checking available swapchain Surface Format...\n");
		if (an_available_formats.size() == 1 && an_available_formats[0].format == vk::Format::eUndefined)
			return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

		for (const auto& i_surface : an_available_formats)
		{
			if (i_surface.format == vk::Format::eB8G8R8A8Unorm 
             && i_surface.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return i_surface;
		}

		return an_available_formats[0];
	}

	vk::PresentModeKHR swapchain::choose_present_mode(const std::vector<vk::PresentModeKHR>& an_available_modes) const
	{
		printf("Determining optimal swapchain present mode...\n");
		vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;

		for (const auto& i_present_mode : an_available_modes)
		{
			if (i_present_mode == vk::PresentModeKHR::eMailbox)
				return i_present_mode;
			else if (i_present_mode == vk::PresentModeKHR::eImmediate)
				best_mode = i_present_mode;
		}

		return best_mode;
	}

	vk::Extent2D swapchain::choose_extent(const vk::SurfaceCapabilitiesKHR& a_capabilities) const
	{
		printf("Choosing swapchain Extent...\n");

		if (a_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return a_capabilities.currentExtent;
		else
		{
			auto window_size = (size<uint32_t>)m_window.get_size();
            vk::Extent2D actual_extent = { window_size.width, window_size.height };

			actual_extent.width = std::max(a_capabilities.minImageExtent.width, 
                                      std::min(a_capabilities.maxImageExtent.width, actual_extent.width));
			actual_extent.height = std::max(a_capabilities.minImageExtent.height, 
                                       std::min(a_capabilities.maxImageExtent.height, actual_extent.height));

			return actual_extent;
		}
	}

	swapchain_support swapchain::query_support(const vk::PhysicalDevice& a_device) const
	{
		printf("Querying swapchain support...\n");

		swapchain_support support_details;
		support_details.capabilities  = a_device.getSurfaceCapabilitiesKHR(m_surface);
		support_details.formats       = a_device.getSurfaceFormatsKHR(m_surface);
		support_details.present_modes = a_device.getSurfacePresentModesKHR(m_surface);

		return support_details;
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
		for (auto i_buffer : m_framebuffers)
			m_device.destroyFramebuffer(i_buffer);

		m_device.freeCommandBuffers(m_commandpool, m_commandbuffers);

		m_pipeline.destroy();

		m_vertex_buffer.destroy();

		for (auto i_view : m_image_views)
			m_device.destroyImageView(i_view);

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
            const vk::ImageView attachments[] = { m_image_views[i] };

            const vk::FramebufferCreateInfo framebuffer_info({}, m_pipeline.get_renderpass(), 1, 
                                                             attachments, 
                                                             m_extent2D.width, 
                                                             m_extent2D.height, 1);

			m_framebuffers[i] = m_device.createFramebuffer(framebuffer_info);
		}

		printf("Finished creating framebuffers.\n");
	}

	void swapchain::recreate()
	{
		const auto window_size = m_window.get_size();

		if (window_size.width == 0 || window_size.height == 0)
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
        const vk::SemaphoreCreateInfo semaphore_createinfo({});

		m_sema_image_available = m_device.createSemaphore(semaphore_createinfo);
		m_sema_render_finished = m_device.createSemaphore(semaphore_createinfo);

		// No error checking?
	}

	void swapchain::create_commandbuffers()
	{
		printf("Creating Command Buffers...\n");

		m_commandbuffers.resize(m_framebuffers.size());

        const vk::CommandBufferAllocateInfo cmdbuffer_alloc_info(m_commandpool, 
                                                             vk::CommandBufferLevel::ePrimary, 
                                                       (uint32_t)m_commandbuffers.size());

		m_commandbuffers = m_device.allocateCommandBuffers(cmdbuffer_alloc_info);

		for (size_t i = 0; i < m_commandbuffers.size(); ++i)
		{
            const vk::CommandBufferBeginInfo cmdbuffer_begin_info(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

			m_commandbuffers[i].begin(cmdbuffer_begin_info);

            const vk::ClearValue clear_color(vk::ClearColorValue(std::array<float, 4>({ 0.f, 0.f, 0.f, 1.f })));
            const vk::Rect2D draw_rect({ 0, 0 }, m_extent2D);
            const vk::RenderPassBeginInfo renderpass_info(m_pipeline.get_renderpass(), 
                                                          m_framebuffers[i], 
                                                          draw_rect, 1, 
                                                         &clear_color);

			m_commandbuffers[i].beginRenderPass(renderpass_info, vk::SubpassContents::eInline);
			m_commandbuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

            const vk::Buffer     vertex_buffers[] = { m_vertex_buffer.get() };
            const vk::DeviceSize buffer_offsets[] = { 0 };
			m_commandbuffers[i].bindVertexBuffers(0, 1, vertex_buffers, buffer_offsets);

            const uint32_t vertex_array_size = m_vertex_buffer.get_vertex_array().size();
			m_commandbuffers[i].draw(vertex_array_size, 1, 0, 0);
			m_commandbuffers[i].endRenderPass();
			m_commandbuffers[i].end();
		}
	}

	void swapchain::create_commandpool()
	{
		printf("Creating Command Pool...\n");

        const queue_families family_indices = find_queue_families(m_physical_device);

        const vk::CommandPoolCreateInfo cmdpool_createinfo({}, family_indices.graphics);
		m_commandpool = m_device.createCommandPool(cmdpool_createinfo);
	}

	void swapchain::create_renderpass()
	{
        const vk::AttachmentDescription color_attach_descript({}, m_image_format, vk::SampleCountFlagBits::e1,
			                                                  vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
			                                                  vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
			                                                  vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

        const vk::AttachmentReference color_attach_ref(0, vk::ImageLayout::eColorAttachmentOptimal);

        const vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &color_attach_ref);

        const vk::SubpassDependency subpass_dependency(VK_SUBPASS_EXTERNAL, 0, 
                                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
			                                       vk::PipelineStageFlagBits::eColorAttachmentOutput, 
                                                   vk::AccessFlags(),
			                                       vk::AccessFlagBits::eColorAttachmentRead 
                                                 | vk::AccessFlagBits::eColorAttachmentWrite);

        const vk::RenderPassCreateInfo renderpass_info({}, 1, &color_attach_descript, 1, &subpass, 1, &subpass_dependency);

		m_pipeline.get_renderpass() = m_device.createRenderPass(renderpass_info);
	}
}