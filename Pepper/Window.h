#pragma once

#include "Structs.h"
#include "Util.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace vk
{
	class SurfaceKHR;
	class Instance;
}

struct GLFWwindow;

namespace ppr
{	
	class Window
	{
	public:
		Window(const std::string& aTitle, size_t aDefaultWidth = 800, size_t aDefaultHeight = 600);
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

		void SetTitle(const std::string& aTitle);
		std::string GetTitle() const;

		vk::SurfaceKHR CreateSurface(const vk::Instance& anInstance) const;
		std::vector<const char*> GetRequiredExtensions();

	private:
		// GLFW
		GLFWwindow* mWindow;

		size_t mDefaultWidth;
		size_t mDefaultHeight;

		std::string mTitle;
	};
}