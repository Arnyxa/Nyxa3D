#pragma once

#include "structs.hpp"
#include "util.hpp"

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
	class window
	{
	public:
		window(const std::string& a_title, size_t a_width = 800, size_t a_height = 600);
		~window();

		void destroy();

		void init();

		bool should_close() const;
		void poll_events();

		GLFWwindow* get_glfw_window();
		void* get_user_ptr();

		void reset_size();
		size<int> get_size() const;
		void set_size(size_t a_width, size_t a_height);

		void set_title(const std::string& a_title);
		std::string get_title() const;

		vk::SurfaceKHR create_surface(const vk::Instance& an_instance) const;
		std::vector<const char*> required_extensions();

	private:
		// GLFW
		GLFWwindow* m_window;

		size_t m_default_width;
		size_t m_default_height;

		std::string m_title;
	};
}