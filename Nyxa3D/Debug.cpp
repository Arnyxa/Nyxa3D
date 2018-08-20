#include "Debug.h"
#include "Globals.h"

#include <iostream>

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

		vk::DebugReportCallbackCreateInfoEXT myCreateInfo(vk::DebugReportFlagBitsEXT::eError 
														| vk::DebugReportFlagBitsEXT::eWarning, 
															CallbackFunc);

		mCallback = mInstance.createDebugReportCallbackEXT(myCreateInfo);
	}

	Debug::~Debug()
	{
		Destroy();
	}

	void Debug::Destroy()
	{
		mInstance.destroyDebugReportCallbackEXT(mCallback);
	}

	VKAPI_ATTR vk::Bool32 VKAPI_CALL Debug::CallbackFunc(VkDebugReportFlagsEXT aFlags, VkDebugReportObjectTypeEXT anObjType, uint64_t anObj,
		size_t aLocation, int32_t aCode, const char* aLayerPrefix, const char* aMsg, void* aUserData)
	{
		std::cerr << "\nValidation layer: " << aMsg << std::endl;

		return VK_FALSE;
	}
}