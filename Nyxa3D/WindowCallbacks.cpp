#include "WindowCallbacks.h"

#include <glfw/glfw3.h>

namespace nx
{
	enum class WindowCallbacks::Type
	{
		Resize,
		Close,
		Focus,
		Reposition,
		Refresh,
		Iconify
	};

	WindowCallbacks& WindowCallbacks::GetInstance()
	{
		static WindowCallbacks myInstance;

		return myInstance;
	}

	void WindowCallbacks::Init(GLFWwindow* aWindow)
	{
		static bool mInitialised = false;

		if (!mInitialised)
		{
			glfwSetWindowSizeCallback(aWindow, OnResize);
			glfwSetWindowCloseCallback(aWindow, OnClose);
			glfwSetWindowFocusCallback(aWindow, OnFocus);
			glfwSetWindowPosCallback(aWindow, OnReposition);
			glfwSetWindowRefreshCallback(aWindow, OnRefresh);
			glfwSetWindowIconifyCallback(aWindow, OnIconify);

			mInitialised = true;
		}
	}

	void WindowCallbacks::OnResize(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		_WindowCallbacks.Execute(Type::Resize);
	}

	void WindowCallbacks::OnClose(GLFWwindow* aWindow)
	{
		_WindowCallbacks.Execute(Type::Close);
	}

	void WindowCallbacks::OnFocus(GLFWwindow* aWindow, CBool wasFocusGained)
	{
		_WindowCallbacks.Execute(Type::Focus);
	}

	void WindowCallbacks::OnReposition(GLFWwindow* aWindow, int xPos, int yPos)
	{
		_WindowCallbacks.Execute(Type::Reposition);
	}

	void WindowCallbacks::OnRefresh(GLFWwindow* aWindow)
	{
		_WindowCallbacks.Execute(Type::Refresh);
	}

	void WindowCallbacks::OnIconify(GLFWwindow* aWindow, CBool wasIconified)
	{
		_WindowCallbacks.Execute(Type::Iconify);
	}

	void WindowCallbacks::Execute(Type aType)
	{
		for (auto& iCallback : mCallbacks)
		{
			if (iCallback.second == aType)
				iCallback.first->Execute();
		}
	}
}