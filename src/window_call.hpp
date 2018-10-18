#pragma once

#include "callback.hpp"
#include "util.hpp"

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

	class window_callbacks : public common_checks
	{
	public:
		static window_callbacks& get();

	private:
		static void OnResize(GLFWwindow* a_window, int a_width, int a_height);
		static void OnClose(GLFWwindow* a_window);
		static void OnFocus(GLFWwindow* a_window, cbool was_focused);
		static void OnReposition(GLFWwindow* a_window, int x, int y);
		static void OnRefresh(GLFWwindow* a_window);
		static void OnIconify(GLFWwindow* a_window, cbool was_iconified);

	public:
		void init(GLFWwindow* a_window);

		template<typename T>
		void add(void (T::*a_function)(), T* obj_ptr, call_type a_type);

	private:
		window_callbacks() {}

		void execute(call_type a_type);

	private:
		std::vector<std::pair<callbacks::ptr, call_type>> mCallbacks;
	};

#define wndcall window_callbacks::get() 
}

#include "window_call.inl"