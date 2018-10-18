#pragma once

#include "globals.hpp"
#include "vulkan_structs.hpp"
#include "structs.hpp"
#include "window.hpp"
#include "debugger.hpp"
#include "pipeline.hpp"
#include "vertex_buffer.hpp"
#include "swapchain.hpp"

#include <vector>
#include <sstream>
#include <string>
#include <iostream>

namespace ppr
{
	class context
	{
	public:
		context(const std::string& a_title = DEFAULT_NAME);
		~context();

		void run();

	private:
		void init();
        void main_loop();

		void create_instance();
		void create_device();

		void select_physical_device();
		bool device_is_suitable(const vk::PhysicalDevice& a_device) const;

		bool check_ext_support(const vk::PhysicalDevice& a_device) const;

		std::vector<const char*> required_extensions() const;

	private:
        // Pepper
        window m_window;
        debugger m_debugger;
        swapchain m_swapchain;

		// Vulkan
		vk::Instance m_instance;
		vk::Device m_device;
		vk::PhysicalDevice m_physical_device;

		const std::vector<const char*> m_device_ext = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}