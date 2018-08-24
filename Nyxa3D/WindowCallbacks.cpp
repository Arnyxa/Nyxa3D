#include "WindowCallbacks.h"

#include <glfw/glfw3.h>

namespace nx
{
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
		_WindowCallbacks.Execute(CallType::Resize);
	}

	void WindowCallbacks::OnClose(GLFWwindow* aWindow)
	{
		_WindowCallbacks.Execute(CallType::Close);
	}

	void WindowCallbacks::OnFocus(GLFWwindow* aWindow, CBool wasFocusGained)
	{
		_WindowCallbacks.Execute(CallType::Focus);
	}

	void WindowCallbacks::OnReposition(GLFWwindow* aWindow, int xPos, int yPos)
	{
		_WindowCallbacks.Execute(CallType::Reposition);
	}

	void WindowCallbacks::OnRefresh(GLFWwindow* aWindow)
	{
		_WindowCallbacks.Execute(CallType::Refresh);
	}

	void WindowCallbacks::OnIconify(GLFWwindow* aWindow, CBool wasIconified)
	{
		_WindowCallbacks.Execute(CallType::Iconify);
	}

	void WindowCallbacks::Execute(CallType aType)
	{
		for (auto& iCallback : mCallbacks)
		{
			if (iCallback.second == aType)
				iCallback.first->Execute();
		}
	}
}