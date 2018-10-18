#include "window.hpp"
#include "callbacks.hpp"
#include "util.hpp"


#include <vulkan/vulkan.hpp>
#include <glfw/glfw3.h>

#include <iostream>
#include <string>

namespace ppr
{
	window::window(const std::string& a_title, size_t a_width, size_t a_height)
		: m_window(nullptr)
		, m_default_width(a_width)
		, m_default_height(a_height)
		, m_title(a_title.c_str())
	{}

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
		printf("Initializing GLFW...\n");

		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		printf("Creating window...\n");

		m_window = glfwCreateWindow((int)m_default_width, (int)m_default_height, m_title.c_str(), nullptr, nullptr);

		wndcall.init(m_window);

		printf("GLFW Initialized.\n");
	}

	void window::destroy()
	{
		if (m_window != nullptr)
		{
			glfwDestroyWindow(m_window);
			glfwTerminate();
		}
	}

	void window::poll_events()
	{
		glfwPollEvents();
	}

	GLFWwindow* window::get_glfw_window()
	{
		return m_window;
	}

	void* window::get_user_ptr()
	{
		return glfwGetWindowUserPointer(m_window);
	}

	vk::SurfaceKHR window::create_surface(const vk::Instance& an_instance) const
	{
		VkSurfaceKHR myTempSurface;

		if (print_vkresult(glfwCreateWindowSurface(an_instance, m_window, nullptr, &myTempSurface)) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan surface for GLFW window.");

		return static_cast<vk::SurfaceKHR>(myTempSurface);

	}

	std::vector<const char*> window::required_extensions()
	{
		uint32_t myCount = 0;
		const char** myExtensionList = glfwGetRequiredInstanceExtensions(&myCount);

		return std::vector<const char*>(myExtensionList, myExtensionList + myCount);
	}

	size<int> window::get_size() const
	{
		size<int> mySize;

		glfwGetWindowSize(m_window, &mySize.width, &mySize.height);

		return mySize;
	}

	void window::set_title(const std::string& a_title)
	{
		m_title = a_title;
		glfwSetWindowTitle(m_window, m_title.c_str());
	}

	std::string window::get_title() const
	{
		return std::string(m_title);
	}

	void window::set_size(size_t a_width, size_t a_height)
	{
		glfwSetWindowSize(m_window, (int)a_width, (int)a_height);
	}

	// Sets window size back to initially specified default size
	void window::reset_size()
	{
		glfwSetWindowSize(m_window, (int)m_default_width, (int)m_default_height);
	}
}