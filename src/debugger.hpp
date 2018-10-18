#pragma once

#include "globals.hpp"

#include <vulkan/vulkan.hpp>

#include <iostream>

namespace ppr
{
	class Debugger
	{
	public:
		explicit Debugger(vk::Instance& anInstance);
		~Debugger();

		void Init();
		void Destroy();

		bool CheckValidationLayerSupport() const;
		uint32_t GetEnabledLayerCount() const;
		const char* const* GetEnabledLayerNames() const;

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

		const std::vector<const char*> mValidationLayers = { LNG_STANDARD_VALIDATION_NAME };
	};
}