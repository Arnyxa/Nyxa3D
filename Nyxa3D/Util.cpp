#include "Util.h"

#include <iostream>
#include <string>

namespace nx
{
	vk::Result PrintResult(vk::Result aResult)
	{
		std::string myTextResult;

		switch (aResult)
		{
		case vk::Result::eSuccess:
			/*myTextResult = "vk::Result::eSuccess -- Command successfully completed.";*/
			return aResult;
			break;
		case vk::Result::eNotReady:
			myTextResult = "vk::Result::eNotready -- A fence or query has not yet completed.";
			break;
		case vk::Result::eTimeout:
			myTextResult = "vk::Result::eTimeout -- A wait operation has not completed in the specified time.";
			break;
		case vk::Result::eEventSet:
			myTextResult = "vk::Result::eEventSet -- An event is signaled.";
			break;
		case vk::Result::eEventReset:
			myTextResult = "vk::Result::eEventReset -- An event is unsignaled.";
			break;
		case vk::Result::eIncomplete:
			myTextResult = "vk::Result::eIncomplete -- A return array was too small for the result.";
			break;
		case vk::Result::eErrorOutOfHostMemory:
			myTextResult = "vk::Result::eErrorOutOfHostMemory -- A host memory allocation has failed.";
			break;
		case vk::Result::eErrorOutOfDeviceMemory:
			myTextResult = "vk::Result::eErrorOutOfDeviceMemory -- A device memory allocation has failed.";
			break;
		case vk::Result::eErrorInitializationFailed:
			myTextResult = "vk::Result::eErrorInitializationFailed -- Initialization of an object could not be completed for implementation-specific reasons.";
			break;
		case vk::Result::eErrorDeviceLost:
			myTextResult = "vk::Result::eErrorDeviceLost -- The logical or physical device has been lost.";
			break;
		case vk::Result::eErrorMemoryMapFailed:
			myTextResult = "vk::Result::eErrorMemoryMapFailed -- Mapping of a memory object has failed.";
			break;
		case vk::Result::eErrorLayerNotPresent:
			myTextResult = "vk::Result::eErrorLayerNotPresent -- A requested layer is not present or could not be loaded.";
			break;
		case vk::Result::eErrorExtensionNotPresent:
			myTextResult = "vk::Result::eErrorExtensionNotPresent -- A requested extension is not supported.";
			break;
		case vk::Result::eErrorFeatureNotPresent:
			myTextResult = "vk::Result::eErrorFeatureNotPresent -- A requested feature is not supported.";
			break;
		case vk::Result::eErrorIncompatibleDriver:
			myTextResult = "vk::Result::eErrorIncompatibleDriver -- The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
			break;
		case vk::Result::eErrorTooManyObjects:
			myTextResult = "vk::Result::eErrorTooManyObjects -- Too many objects of the type have already been created.";
			break;
		case vk::Result::eErrorFormatNotSupported:
			myTextResult = "vk::Result::eErrorFormatNotSupported -- A requested format is not supported on this device.";
			break;
		case vk::Result::eErrorFragmentedPool:
			myTextResult = "vk::Result::eErrorFragmentedPool -- A pool allocation has failed due to fragmentation of the poolÅfs memory.\n This *must* only be returned if no attempt to allocate host or device memory was made to accomodate the new allocation.";
			break;
		}

		std::cout << myTextResult << "\n\n";

		return aResult;
	}

}