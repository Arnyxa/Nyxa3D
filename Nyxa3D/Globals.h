#pragma once

namespace nx
{
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