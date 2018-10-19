#pragma once

#include "globals.hpp"

#include <vulkan/vulkan.hpp>



namespace ppr
{
	class debugger
	{
	public:
		explicit debugger(const vk::Instance& an_instance);
		~debugger();

		void init();
		void destroy();

		bool supports_validation_layers() const;
		uint32_t enabled_layer_count() const;
		const char* const* enabled_layer_names() const;

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL callback(VkDebugUtilsMessageSeverityFlagBitsEXT a_severity,
														VkDebugUtilsMessageTypeFlagsEXT a_type,
														const VkDebugUtilsMessengerCallbackDataEXT* a_callback_data,
														void* a_user_data);

		VkResult CreateDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT* a_createinfo,
											const VkAllocationCallbacks* an_allocator, VkDebugUtilsMessengerEXT* a_callback) const;

		void DestroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT a_callback, const VkAllocationCallbacks* an_allocator = nullptr) const;

	private:
		VkDebugUtilsMessengerEXT m_messenger;
		const vk::Instance& m_instance;

		static constexpr char CREATE_DBG_MSGR_EXT[]  = "vkCreateDebugUtilsMessengerEXT";
		static constexpr char DESTROY_DBG_MSGR_EXT[] = "vkDestroyDebugUtilsMessengerEXT";

		const std::vector<const char*> m_validation_layers = { LNG_STANDARD_VALIDATION_NAME };
	};
}