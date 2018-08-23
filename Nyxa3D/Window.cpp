#include "Window.h"
#include "Util.h"

#include <glfw/glfw3.h>

#include <iostream>
#include <string>

namespace nx
{
	Window::Window(size_t aDefaultWidth, size_t aDefaultHeight)
		: mWindow(nullptr)
		, mDefaultWidth(aDefaultWidth)
		, mDefaultHeight(aDefaultHeight)
		, mTitle("Nyxa3D")
	{}

	Window::~Window()
	{
		Destroy();
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(mWindow);
	}

	void Window::Init()
	{
		std::cout << "Initializing GLFW...\n";

		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		std::cout << "Creating window...\n";

		mWindow = glfwCreateWindow((int)mDefaultWidth, (int)mDefaultHeight, mTitle, nullptr, nullptr);

		std::cout << "GLFW Initialized.\n";
	}

	// Defines callback function to invoke on window resize
	void Window::SetResizeCallback(GLFWwindowsizefun aFunction, void* aUserPointer)
	{
		glfwSetWindowUserPointer(mWindow, aUserPointer);
		glfwSetWindowSizeCallback(mWindow, aFunction);
	}

	void Window::Destroy()
	{
		if (mWindow != nullptr)
		{
			glfwDestroyWindow(mWindow);
			glfwTerminate();
		}
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	GLFWwindow* Window::GetGlfwWindowPtr()
	{
		return mWindow;
	}

	void* Window::GetWindowUserPtr()
	{
		return glfwGetWindowUserPointer(mWindow);
	}

	vk::SurfaceKHR Window::CreateSurface(vk::Instance& anInstance)
	{
		VkSurfaceKHR myTempSurface;

		if (PrintResult(glfwCreateWindowSurface(anInstance, mWindow, nullptr, &myTempSurface)) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Vulkan surface for GLFW window.");

		return static_cast<vk::SurfaceKHR>(myTempSurface);

	}

	std::vector<const char*> Window::GetRequiredExtensions()
	{
		uint32_t myCount = 0;
		const char** myExtensionList = glfwGetRequiredInstanceExtensions(&myCount);

		return std::vector<const char*>(myExtensionList, myExtensionList + myCount);
	}

	Size<int> Window::GetSize() const
	{
		Size<int> mySize;

		glfwGetWindowSize(mWindow, &mySize.Width, &mySize.Height);

		return mySize;
	}

	void Window::SetTitle(const std::string& aTitle)
	{
		mTitle = aTitle.c_str();
		glfwSetWindowTitle(mWindow, mTitle);
	}

	std::string Window::GetTitle() const
	{
		return std::string(mTitle);
	}

	void Window::SetSize(size_t aWidth, size_t aHeight)
	{
		glfwSetWindowSize(mWindow, (int)aWidth, (int)aHeight);
	}

	// Sets window size back to initially specified default size
	void Window::ResetSize()
	{
		glfwSetWindowSize(mWindow, (int)mDefaultWidth, (int)mDefaultHeight);
	}
}