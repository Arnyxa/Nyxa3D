#pragma once

namespace ppr
{
	constexpr const char* LNG_STANDARD_VALIDATION_NAME = "VK_LAYER_LUNARG_standard_validation";
    constexpr const char* PROJECT_TITLE = "Pepper";
}

namespace ppr
{

#ifdef PPR_DEBUG
	constexpr bool VALIDATION_LAYERS_ENABLED = true;
#else
	constexpr bool VALIDATION_LAYERS_ENABLED = false;
#endif

}