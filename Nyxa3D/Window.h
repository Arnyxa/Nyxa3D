#pragma once

#include "Structs.h"
#include "Util.h"

#include <string>
#include <vector>

struct GLFWwindow;
typedef void(*GLFWwindowsizefun)(GLFWwindow*, int, int);

namespace nx
{	
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
		void SetResizeCallback(GLFWwindowsizefun aFunction, void* aUserPointer);

		void SetTitle(const std::string& aTitle);
		std::string GetTitle() const;

		vk::SurfaceKHR CreateSurface(vk::Instance& anInstance);
		std::vector<const char*> GetRequiredExtensions();

	private:
		// GLFW
		GLFWwindow* mWindow;

		size_t mDefaultWidth;
		size_t mDefaultHeight;

		const char* mTitle;
	};
}

