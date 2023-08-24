#pragma once

#include "internal/supportTypes/coordTypes.h"
#include "engine/String.h"

#include <inttypes.h>

#include <engine/types.h>

namespace Game::Items
{

	struct Desc
	{
		ENGINE_TYPES_DESERIALIZE;

		float size{ 1.f };
		Engine::String itemTexture;
	};

}
