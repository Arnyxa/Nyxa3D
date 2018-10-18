#include "window_call.hpp"

#include <glfw/glfw3.h>

#include <iostream>

namespace ppr
{
	window_callbacks& window_callbacks::get()
	{
		static window_callbacks myInstance;

		return myInstance;
	}

	void window_callbacks::init(GLFWwindow* a_window)
	{
		static bool m_initialized = false;

		if (!m_initialized)
		{
            if (a_window == nullptr)
                throw Error("Provided window was null pointer.", Error::Code::NULL_PTR);

			printf("Initializing window callbacks...\n");

			glfwSetWindowSizeCallback(a_window, OnResize);
			glfwSetWindowCloseCallback(a_window, OnClose);
			glfwSetWindowFocusCallback(a_window, OnFocus);
			glfwSetWindowPosCallback(a_window, OnReposition);
			glfwSetWindowRefreshCallback(a_window, OnRefresh);
			glfwSetWindowIconifyCallback(a_window, OnIconify);

			m_initialized = true;

			printf("window callbacks initialized.\n\n");
		}
	}

	void window_callbacks::OnResize(GLFWwindow* a_window, int a_width, int a_height)
	{
		get().execute(call_type::RESIZE);
	}

	void window_callbacks::OnClose(GLFWwindow* a_window)
	{
		get().execute(call_type::CLOSE);
	}

	void window_callbacks::OnFocus(GLFWwindow* a_window, cbool was_focused)
	{
		if (was_focused != GLFW_TRUE && was_focused != GLFW_FALSE)
			printf(std::string(std::string("\nWarning: 'was_focused' in 'window_callbacks::OnFocus(GLFWwindow* a_window, cbool was_focused)'") 
								+ "\nValue evaluated to: " + std::to_string(was_focused) + " (non-bool).\n\n").c_str());

		if (was_focused)
			get().execute(call_type::FOCUS);
		else
			get().execute(call_type::UNFOCUS);
	}

	void window_callbacks::OnReposition(GLFWwindow* a_window, int x, int y)
	{
		get().execute(call_type::REPOSITION);
	}

	void window_callbacks::OnRefresh(GLFWwindow* a_window)
	{
		get().execute(call_type::REFRESH);
	}

	void window_callbacks::OnIconify(GLFWwindow* a_window, cbool was_iconified)
	{
		get().execute(call_type::ICONIFY);
	}

	void window_callbacks::execute(call_type a_type)
	{
		for (auto& iCallback : mCallbacks)
		{
			if (iCallback.second == a_type)
				iCallback.first->execute();
		}
	}
}