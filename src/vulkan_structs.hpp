// For reusable structs which require including vulkan.hpp
// Separate from structs.hpp to reduce compile times

#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

namespace ppr
{
	struct SwapchainDetails
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};
}