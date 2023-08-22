#pragma once

#include "internal/supportTypes/coordTypes.h"
#include "engine/String.h"

#include <inttypes.h>

#include <engine/types.h>

namespace Game::Explosions
{

	struct Desc
	{
		ENGINE_TYPES_DESERIALIZE;

		float radius{};
		Types::Coord::Vec2f position;
	};

	struct Cmd
	{
		ENGINE_TYPES_DESERIALIZE;

		float magnitude{};
		Desc desc;
	};

}
