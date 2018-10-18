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
	using CBool = int;

	enum class CallType
	{
		Resize,
		Close,
		Focus,
		Unfocus,
		Reposition,
		Refresh,
		Iconify,
	};

	class WindowCallbacks : public CommonChecks
	{
	public:
		static WindowCallbacks& GetInstance();

	private:
		static void OnResize(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnClose(GLFWwindow* aWindow);
		static void OnFocus(GLFWwindow* aWindow, CBool wasFocusGained);
		static void OnReposition(GLFWwindow* aWindow, int xPos, int yPos);
		static void OnRefresh(GLFWwindow* aWindow);
		static void OnIconify(GLFWwindow* aWindow, CBool wasIconified);

	public:
		void Init(GLFWwindow* aWindow);

		template<typename T>
		void Add(void (T::*aFunction)(), T* anObjPtr, CallType aType);

	private:
		WindowCallbacks() {}

		void Execute(CallType aType);

	private:
		std::vector<std::pair<Callbacks::ptr, CallType>> mCallbacks;
	};

#define WndCallbacks WindowCallbacks::GetInstance() 
}

#include "window_call.inl"