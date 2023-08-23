#pragma once

#include "internal/supportTypes/coordTypes.h"
#include "engine/String.h"

#include <inttypes.h>

#include <engine/types.h>

namespace Game::Perks
{

	struct Desc
	{
		ENGINE_TYPES_DESERIALIZE;

		int32_t type{};
		Types::Coord::Vec2f coord;
	};

}
