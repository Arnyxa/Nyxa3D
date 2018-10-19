#include "window.hpp"
#include "callbacks.hpp"
#include "util.hpp"
#include "logger.hpp"

#include <vulkan/vulkan.hpp>
#include <glfw/glfw3.h>


#include <string>

namespace ppr
{
	window::window(const std::string& a_title, size_t a_width, size_t a_height)
		: m_window(nullptr)
		, m_default_width(a_width)
		, m_default_height(a_height)
		, m_title(a_title.c_str())
	{
        log->info("Setting up window...");

        if (!glfwInit())
            log->critical("GLFW failed to initialize.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        log->debug("GLFW Initialized.");

        log->trace("Creating window...");
        m_window = glfwCreateWindow(m_default_width, m_default_height, m_title.c_str(), nullptr, nullptr);

        wndcall.init(m_window);

        log->info("Window successfully created.\n");
    }

	window::~window()
	{
		destroy();
	}

	bool window::should_close() const
	{
		return glfwWindowShouldClose(m_window);
	}

	void window::init()
	{

	}

	void window::destroy()
	{
		if (m_window != nullptr)
		{
			glfwDestroyWindow(m_window);
			glfwTerminate();
		}
	}

	void window::poll_events() const
	{
		glfwPollEvents();
	}

	GLFWwindow* window::get_glfw_window() const
	{
		return m_window;
	}

	void* window::get_user_ptr() const
	{
		return glfwGetWindowUserPointer(m_window);
	}

	vk::SurfaceKHR window::create_surface(const vk::Instance& an_instance) const
	{
		VkSurfaceKHR temp_surface;

        if (print(glfwCreateWindowSurface(an_instance, m_window, nullptr, &temp_surface)) != VK_SUCCESS)
            log->critical("Failed to create Vulkan surface for GLFW window.");

        log->trace("Window surface successfully created");
		return static_cast<vk::SurfaceKHR>(temp_surface);
	}

	std::vector<const char*> window::required_extensions() const
	{
		uint32_t ext_count = 0;
		const char** instance_ext_list = glfwGetRequiredInstanceExtensions(&ext_count);

		return std::vector<const char*>(instance_ext_list, instance_ext_list + ext_count);
	}

	size<int> window::get_size() const
	{
		size<int> window_size;

		glfwGetWindowSize(m_window, &window_size.width, &window_size.height);

		return window_size;
	}

	void window::set_title(const std::string& a_title)
	{
		m_title = a_title;
		glfwSetWindowTitle(m_window, m_title.c_str());
        log->trace("New window title set: {}", m_title);
	}

	std::string window::get_title() const
	{
		return std::string(m_title);
	}

	void window::set_size(size_t a_width, size_t a_height)
	{
		glfwSetWindowSize(m_window, (int)a_width, (int)a_height);
        log->trace("Window resized to: {}x{}", a_width, a_height);
	}

	// Sets window size back to initially specified default size
	void window::reset_size()
	{
		glfwSetWindowSize(m_window, (int)m_default_width, (int)m_default_height);
        log->trace("Window size reset to: {}x{}", m_default_width, m_default_height);
	}

    void window::check_extension_compatibility(const std::vector<const char*>& a_extensions, const std::vector<vk::ExtensionProperties>& a_properties) const
    {
        log->trace("Checking for GLFW extension compatibility...");

        // check whether extensions required by GLFW are in available extension list
        for (size_t i = 0; i < a_extensions.size(); ++i)
        {
            bool was_found = false;
            for (const auto& i_property : a_properties)
            {
                if (std::strcmp(a_extensions[i], i_property.extensionName) == 0)
                {
                    was_found = true;
                    break;
                }
            }

            if (!was_found)
                log->critical("Could not find required GLFW extension for Vulkan on this system.");

            log->trace("        {} found.", a_extensions[i]);
        }
    }
}