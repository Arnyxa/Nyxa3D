#pragma once

#pragma once

#include <vulkan\vulkan.hpp>
#include <string>

namespace nx
{
	VkResult PrintResult(VkResult aResult);

	void Print(const std::string& aText);
	vk::Result Print(vk::Result aResult);
}

namespace nx
{
	class CommonChecks
	{
	public:
		CommonChecks() 
			: mDestroyed(false)
			, mInitialized(false) 
		{}

		bool IsInitialized() const
		{ return mInitialized; }

	protected:
		bool mDestroyed;
		bool mInitialized;
	};
}