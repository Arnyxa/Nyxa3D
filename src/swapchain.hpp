#pragma once

#include "vulkan_structs.hpp"
#include "structs.hpp"
#include "util.hpp"
#include "window.hpp"
#include "pipeline.hpp"
#include "vertex_buffer.hpp"

#include <vulkan/vulkan.hpp>

#include <vector>
#include <functional>

namespace ppr
{
	class swapchain : public common_checks
	{
	public:
		swapchain(const vk::Device& a_device, 
				const window& a_window,
				const vk::Instance& an_instance,
				const vk::PhysicalDevice& a_physical_device);
		~swapchain();

		void draw();
		void init();

		void create();
		void recreate();

		void cleanup();
		void destroy();

		void create_window_surface();
		void on_window_resize();

		swapchain_support query_support(const vk::PhysicalDevice& a_device) const;
		queue_families find_queue_families(const vk::PhysicalDevice& a_device) const;

        vk::Queue& graphics_queue();
        vk::Queue& present_queue();

	private:
		void destroy_window_surface() const;
		void create_imageviews();
		void create_framebuffers();
		void create_commandpool();
		void create_commandbuffers();
		void create_renderpass();
		void create_semaphores();

		vk::Extent2D choose_extent(const vk::SurfaceCapabilitiesKHR& a_capabilities) const;
		vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& an_available_modes) const;
		vk::SurfaceFormatKHR choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& an_available_formats) const;

	private:
		const window& m_window;
		const vk::Device& m_device;
		const vk::Instance& m_instance;
		const vk::PhysicalDevice& m_physical_device;

		pipeline m_pipeline;
		vertex_buffer m_vertex_buffer;

		vk::SurfaceKHR m_surface;
		vk::SwapchainKHR m_swapchain;

		vk::Format m_image_format;
		vk::Extent2D m_extent2D;

		vk::Queue m_queue_present;
		vk::Queue m_queue_graphics;

		vk::CommandPool m_commandpool;
		vk::Semaphore m_sema_image_available;
		vk::Semaphore m_sema_render_finished;

		std::vector<vk::Image> m_images;
		std::vector<vk::ImageView> m_image_views;
		std::vector<vk::Framebuffer> m_framebuffers;
		std::vector<vk::CommandBuffer> m_commandbuffers;
	};
}
