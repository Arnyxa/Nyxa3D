#pragma once

#include "structs.hpp"
#include "util.hpp"

#include <string>
#include <vector>
#include <functional>

namespace vk
{
	class SurfaceKHR;
	class Instance;

    struct ExtensionProperties;
}

struct GLFWwindow;

namespace ppr
{	
	class window
	{
	public:
		window(const std::string& a_title, size_t a_default_width = 800, size_t a_default_height = 600);
		~window();

		void destroy();

		void init();

		bool should_close() const;
		void poll_events() const;

		GLFWwindow* get_glfw_window() const;
		void* get_user_ptr() const;

		void reset_size();
		size<int> get_size() const;
		void set_size(size_t a_width, size_t a_height);

		void set_title(const std::string& a_title);
		std::string get_title() const;

		vk::SurfaceKHR create_surface(const vk::Instance& an_instance) const;
		std::vector<const char*> required_extensions() const;
        void check_extension_compatibility(const std::vector<const char*>& a_extensions, 
                                           const std::vector<vk::ExtensionProperties>& a_properties) const;

	private:
		// GLFW
		GLFWwindow* m_window;

		size_t m_default_width;
		size_t m_default_height;

		std::string m_title;
	};
}