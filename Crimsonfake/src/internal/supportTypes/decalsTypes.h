#pragma once

#include <renderer.h>

#include "coordTypes.h"

namespace Types::Decals
{

	struct Cmd
	{
		float angle{};
		float scale{};
		Coord::Vec2f position;
	};

}
