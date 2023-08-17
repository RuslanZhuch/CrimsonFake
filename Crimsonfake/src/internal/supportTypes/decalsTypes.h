#pragma once

#include <renderer.h>

#include "coordTypes.h"

#include <engine/types.h>

namespace Types::Decals
{
	struct Cmd
	{

		ENGINE_TYPES_DESERIALIZE;

		float angle{};
		float scale{};
		Coord::Vec2f position;
	};

}
