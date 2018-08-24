#include "Util.h"

#include <iostream>

namespace nx
{
	void Print(const std::string& aText)
	{
		std::cout << aText << "\n";
	}

	vk::Result Print(vk::Result aResult)
	{
		if (aResult != vk::Result::eSuccess)
			Print(vk::to_string(aResult));

		return aResult;
	}

	VkResult PrintResult(VkResult aResult)
	{
		std::string myTextResult;

		switch (aResult)
		{
		case VK_SUCCESS:
			/*myTextResult = "VK_SUCCESS -- Command successfully completed.";*/
			return aResult;
			break;
		case VK_NOT_READY:
			myTextResult = "VK_NOT_READY -- A fence or query has not yet completed.";
			break;
		case VK_TIMEOUT:
			myTextResult = "VK_TIMEOUT -- A wait operation has not completed in the specified time.";
			break;
		case VK_EVENT_SET:
			myTextResult = "VK_EVENT_SET -- An event is signaled.";
			break;
		case VK_EVENT_RESET:
			myTextResult = "VK_EVENT_RESET -- An event is unsignaled.";
			break;
		case VK_INCOMPLETE:
			myTextResult = "VK_INCOMPLETE -- A return array was too small for the result.";
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			myTextResult = "VK_ERROR_OUT_OF_HOST_MEMORY -- A host memory allocation has failed.";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			myTextResult = "VK_ERROR_OUT_OF_DEVICE_MEMORY -- A device memory allocation has failed.";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			myTextResult = "VK_ERROR_INITIALIZATION_FAILED -- Initialization of an object could not be completed for implementation-specific reasons.";
			break;
		case VK_ERROR_DEVICE_LOST:
			myTextResult = "VK_ERROR_DEVICE_LOST -- The logical or physical device has been lost.";
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			myTextResult = "VK_ERROR_MEMORY_MAP_FAILED -- Mapping of a memory object has failed.";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			myTextResult = "VK_ERROR_LAYER_NOT_PRESENT -- A requested layer is not present or could not be loaded.";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			myTextResult = "VK_ERROR_EXTENSION_NOT_PRESENT -- A requested extension is not supported.";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			myTextResult = "VK_ERROR_FEATURE_NOT_PRESENT -- A requested feature is not supported.";
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			myTextResult = "VK_ERROR_INCOMPATIBLE_DRIVER -- The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			myTextResult = "VK_ERROR_TOO_MANY_OBJECTS -- Too many objects of the type have already been created.";
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			myTextResult = "VK_ERROR_FORMAT_NOT_SUPPORTED -- A requested format is not supported on this device.";
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			myTextResult = "VK_ERROR_FRAGMENTED_POOL -- A pool allocation has failed due to fragmentation of the pool�fs memory.\n This *must* only be returned if no attempt to allocate host or device memory was made to accomodate the new allocation.";
			break;
		}

		std::cout << myTextResult << "\n" << std::endl;

		return aResult;
	}
}