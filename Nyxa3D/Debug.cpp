#include "Debug.h"
#include "Globals.h"
#include "Util.h"

#include <iostream>

// leave this largely in C mode cause the C++ pointers to functions are giving me a headache

namespace nx
{
	Debug::Debug(vk::Instance& anInstance)
		: mInstance(anInstance)
	{}

	void Debug::Init()
	{
		if (!VALIDATION_LAYERS_ENABLED)
			return;

		std::cout << "Initializing Debug Callback...\n";

		VkDebugUtilsMessengerCreateInfoEXT myCreateInfo;
		myCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		myCreateInfo.flags = 0;
		myCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		myCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		myCreateInfo.pfnUserCallback = Callback;

		if (PrintResult(CreateDebugUtilsMessenger(&myCreateInfo, nullptr, &mDbgMessenger)) != VK_SUCCESS)
			throw std::runtime_error("Failed to setup Debug Utilities Messenger.\n");
	}

	Debug::~Debug()
	{
		Destroy();
	}

	void Debug::Destroy()
	{
		if (mDbgMessenger != VK_NULL_HANDLE)
			DestroyDebugUtilsMessenger(mDbgMessenger);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Debug::Callback(VkDebugUtilsMessageSeverityFlagBitsEXT aSeverity,
		VkDebugUtilsMessageTypeFlagsEXT aType,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* aUserData)
	{
		std::cerr << "\nValidation layer: " << aCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult Debug::CreateDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo, const VkAllocationCallbacks* anAllocator, VkDebugUtilsMessengerEXT* aMessenger)
	{
		auto myFunc = (PFN_vkCreateDebugUtilsMessengerEXT)mInstance.getProcAddr(CREATE_DBG_MSGR_EXT);

		if (myFunc != nullptr)
			return myFunc(mInstance, aCreateInfo, anAllocator, aMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void Debug::DestroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT aMessenger, const VkAllocationCallbacks* anAllocator)
	{
		auto myFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)mInstance.getProcAddr(DESTROY_DBG_MSGR_EXT);
		if (myFunc != nullptr)
			myFunc(mInstance, aMessenger, anAllocator);
	}

	uint32_t Debug::GetEnabledLayerCount() const
	{
		return static_cast<uint32_t>(mValidationLayers.size());
	}

	const char* const* Debug::GetEnabledLayerNames() const
	{
		return mValidationLayers.data();
	}

	bool Debug::CheckValidationLayerSupport() const
	{
		std::cout << "Checking for validation layer compatibility...\n";

		std::vector<vk::LayerProperties> myAvailableLayers = vk::enumerateInstanceLayerProperties();

		for (const char* iLayerName : mValidationLayers)
		{
			bool isFound = false;

			for (const auto& iLayerProperties : myAvailableLayers)
			{
				if (std::strcmp(iLayerName, iLayerProperties.layerName) == 0)
				{
					isFound = true;
					break;
				}
			}

			if (!isFound)
			{
				std::cout << iLayerName << " is not supported.\n";
				return false;
			}

			std::cout << iLayerName << " is supported.\n";
		}

		std::cout << "All validation layers are supported.\n\n";

		return true;
	}
}