#pragma once

#include <engine/types.h>

namespace Types::Collision
{

	struct Circle
	{

		ENGINE_TYPES_DESERIALIZE;

		float x{};
		float y{};
		float r{};
	};

}