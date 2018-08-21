// If identifier name already exists, suffix an underscore
// Otherwise all caps with underscore for spaces
#pragma once

#include <algorithm>

namespace nx
{
	extern uint64_t UINT64_MAX_ = std::numeric_limits<uint64_t>::max();
	extern constexpr char LNG_STANDARD_VALIDATION_NAME[] = "VK_LAYER_LUNARG_standard_validation";
}

namespace nx
{

#ifdef NXDBG
	extern constexpr bool VALIDATION_LAYERS_ENABLED = true;
#else
	extern constexpr bool VALIDATION_LAYERS_ENABLED = false;
#endif

}