#include "WndCallbacks.h"

#include <glfw/glfw3.h>

#include <iostream>

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
		GetInstance().Execute(CallType::Resize);
	}

	void WindowCallbacks::OnClose(GLFWwindow* aWindow)
	{
		GetInstance().Execute(CallType::Close);
	}

	void WindowCallbacks::OnFocus(GLFWwindow* aWindow, CBool wasFocusGained)
	{
		if (wasFocusGained != GLFW_TRUE && wasFocusGained != GLFW_FALSE)
			std::cout << "\nWarning: 'wasFocusGained' in 'WindowCallbacks::OnFocus(GLFWwindow* aWindow, CBool wasFocusGained)'"
			<< "\nValue evaluated to: " << wasFocusGained << " (non-bool).\n\n";

		if (wasFocusGained)
			GetInstance().Execute(CallType::Focus);
		else
			GetInstance().Execute(CallType::Unfocus);
	}

	void WindowCallbacks::OnReposition(GLFWwindow* aWindow, int xPos, int yPos)
	{
		GetInstance().Execute(CallType::Reposition);
	}

	void WindowCallbacks::OnRefresh(GLFWwindow* aWindow)
	{
		GetInstance().Execute(CallType::Refresh);
	}

	void WindowCallbacks::OnIconify(GLFWwindow* aWindow, CBool wasIconified)
	{
		GetInstance().Execute(CallType::Iconify);
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