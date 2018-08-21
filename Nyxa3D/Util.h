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