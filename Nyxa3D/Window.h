#pragma once

#include "Structs.h"
#include "Util.h"

#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;
typedef void(*GLFWwindowsizefun)(GLFWwindow*, int, int);

namespace nx
{	
	enum class CallbackType
	{
		RESIZE,
		Total
	};

	class Window
	{
	public:
		Window(size_t aDefaultWidth = 800, size_t aDefaultHeight = 600);
		~Window();

		void Destroy();

		void Init();

		bool ShouldClose() const;
		void PollEvents();

		GLFWwindow* GetGlfwWindowPtr();
		void* GetWindowUserPtr();

		void ResetSize();
		Size<int> GetSize() const;
		void SetSize(size_t aWidth, size_t aHeight);

		void AddCallback(std::function<void(void*)> aCallbackFn, void* anObjPtr) const;

		void SetTitle(const std::string& aTitle);
		std::string GetTitle() const;

		vk::SurfaceKHR CreateSurface(const vk::Instance& anInstance) const;
		std::vector<const char*> GetRequiredExtensions();

		static void OnResize(GLFWwindow* aWindow, int aWidth, int aHeight);

	private:
		void ExecuteResizeCallbacks(); // may add callbacks for other things


	private:
		// GLFW
		GLFWwindow* mWindow;

		size_t mDefaultWidth;
		size_t mDefaultHeight;

		// I KNOW THIS IS ATROCIOUS i'll change it later
		mutable std::vector<std::pair<void*, std::function<void(void*)>>> mResizeCallbackFunctions;

		const char* mTitle;
	};
}