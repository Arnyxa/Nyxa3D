#pragma once

#include <vulkan/vulkan.hpp>

namespace nx
{
	class Debug
	{
	public:
		explicit Debug(vk::Instance& anInstance);
		~Debug();

		void Init();
		void Destroy();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL Callback(VkDebugUtilsMessageSeverityFlagBitsEXT aSeverity,
														VkDebugUtilsMessageTypeFlagsEXT aType,
														const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
														void* aUserData);

		VkResult CreateDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo,
											const VkAllocationCallbacks* anAllocator, VkDebugUtilsMessengerEXT* aCallback);

		void DestroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT aCallback, const VkAllocationCallbacks* anAllocator = nullptr);

	private:
		VkDebugUtilsMessengerEXT mDbgMessenger;
		vk::Instance& mInstance;

		static constexpr char CREATE_DBG_MSGR_EXT[]  = "vkCreateDebugUtilsMessengerEXT";
		static constexpr char DESTROY_DBG_MSGR_EXT[] = "vkDestroyDebugUtilsMessengerEXT";
	};
}