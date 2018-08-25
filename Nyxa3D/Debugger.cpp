#include "Globals.h"
#include "Util.h"
#include "Debug.h"


#include <iostream>

// leave this largely in C mode cause the C++ pointers to functions are giving me a headache

namespace nx
{
	Debugger::Debugger(vk::Instance& anInstance)
		: mInstance(anInstance)
	{}

	void Debugger::Init()
	{
		if (!VALIDATION_LAYERS_ENABLED)
			return;

		DbgPrint("Initializing Debugger Callback...\n");

		VkDebugUtilsMessengerCreateInfoEXT myCreateInfo;
		myCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		myCreateInfo.flags = 0;
		myCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		myCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		myCreateInfo.pfnUserCallback = Callback;

		if (PrintResult(CreateDebugUtilsMessenger(&myCreateInfo, nullptr, &mDbgMessenger)) != VK_SUCCESS)
			throw std::runtime_error("Failed to setup Debugger Utilities Messenger.\n");
	}

	Debugger::~Debugger()
	{
		Destroy();
	}

	void Debugger::Destroy()
	{
		if (mDbgMessenger != VK_NULL_HANDLE)
			DestroyDebugUtilsMessenger(mDbgMessenger);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Debugger::Callback(VkDebugUtilsMessageSeverityFlagBitsEXT aSeverity,
		VkDebugUtilsMessageTypeFlagsEXT aType,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* aUserData)
	{
		std::cerr << "\nValidation layer: " << aCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult Debugger::CreateDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo, const VkAllocationCallbacks* anAllocator, VkDebugUtilsMessengerEXT* aMessenger)
	{
		auto myFunc = (PFN_vkCreateDebugUtilsMessengerEXT)mInstance.getProcAddr(CREATE_DBG_MSGR_EXT);

		if (myFunc != nullptr)
			return myFunc(mInstance, aCreateInfo, anAllocator, aMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void Debugger::DestroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT aMessenger, const VkAllocationCallbacks* anAllocator)
	{
		auto myFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)mInstance.getProcAddr(DESTROY_DBG_MSGR_EXT);
		if (myFunc != nullptr)
			myFunc(mInstance, aMessenger, anAllocator);
	}

	uint32_t Debugger::GetEnabledLayerCount() const
	{
		return static_cast<uint32_t>(mValidationLayers.size());
	}

	const char* const* Debugger::GetEnabledLayerNames() const
	{
		return mValidationLayers.data();
	}

	bool Debugger::CheckValidationLayerSupport() const
	{
		DeepPrint("Checking for validation layer compatibility...\n");

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
				DeepPrint(std::string(iLayerName) + " is not supported.\n");
				return false;
			}

			DeepPrint(std::string(iLayerName) + " is supported.\n");
		}

		DeepPrint("All validation layers are supported.\n\n");

		return true;
	}
}