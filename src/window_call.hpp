#pragma once

#include "callback.hpp"
#include "util.hpp"
#include "logger.hpp"

#include <vector>

struct GLFWwindow;
typedef void(*GLFWwindowsizefun) (GLFWwindow*, int, int);
typedef void(*GLFWwindowclosefun) (GLFWwindow*);
typedef void(*GLFWwindowposfun) (GLFWwindow*, int, int);
typedef void(*GLFWwindowrefreshfun) (GLFWwindow*);
typedef void(*GLFWwindowfocusfun) (GLFWwindow*, int);
typedef void(*GLFWwindowiconifyfun) (GLFWwindow*, int);

namespace ppr
{
	using cbool = int;

	enum class call_type
	{
		RESIZE,
		CLOSE,
		FOCUS,
		UNFOCUS,
		REPOSITION,
		REFRESH,
		ICONIFY,
	};

	class window_callbacks
	{
	public:
		static window_callbacks& get();

	private:
		static void on_resize(GLFWwindow* a_window, int a_width, int a_height);
		static void on_close(GLFWwindow* a_window);
		static void on_focus(GLFWwindow* a_window, cbool was_focused);
		static void on_reposition(GLFWwindow* a_window, int x, int y);
		static void on_refresh(GLFWwindow* a_window);
		static void on_iconify(GLFWwindow* a_window, cbool was_iconified);

	public:
		void init(GLFWwindow* a_window) const;

		template<typename T>
		void add(void (T::*a_function)(), T* obj_ptr, call_type a_type);

	private:
		window_callbacks() {}

		void execute(call_type a_type);

	private:
		std::vector<std::pair<callbacks::ptr, call_type>> m_callbacks;
	};

#define wndcall window_callbacks::get() 
}

#include "window_call.inl"