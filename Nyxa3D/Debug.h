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
		static VKAPI_ATTR vk::Bool32 VKAPI_CALL CallbackFunc(VkDebugReportFlagsEXT aFlags, VkDebugReportObjectTypeEXT anObjType, uint64_t anObj,
			size_t aLocation, int32_t aCode, const char* aLayerPrefix, const char* aMsg, void* aUserData);

	private:
		vk::DebugReportCallbackEXT mCallback;
		vk::Instance& mInstance;
	};
}