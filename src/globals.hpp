#pragma once

namespace ppr
{
	extern constexpr char LNG_STANDARD_VALIDATION_NAME[] = "VK_LAYER_LUNARG_standard_validation";
}

namespace ppr
{

#ifdef PPR_DEBUG
	extern constexpr bool VALIDATION_LAYERS_ENABLED = true;
#else
	extern constexpr bool VALIDATION_LAYERS_ENABLED = false;
#endif

}