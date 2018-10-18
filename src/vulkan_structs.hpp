// For reusable structs which require including vulkan.hpp
// Separate from structs.hpp to reduce compile times

#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

namespace ppr
{
	struct swapchain_support
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;
	};
}