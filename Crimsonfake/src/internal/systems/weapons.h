#pragma once

#include "internal/supportTypes/coordTypes.h"
#include "engine/String.h"

#include <inttypes.h>

#include <engine/types.h>

namespace Game::Weapons
{

	struct FireCmd
	{

		ENGINE_TYPES_DESERIALIZE;

		Types::Coord::Vec2f spawnCoord;
		float angle{};
	};

	struct Desc
	{

		ENGINE_TYPES_DESERIALIZE;

		int32_t type{};
		float bulletVelocity{};
		float bulletLifeTime{ 2 };
		int32_t damage{ 1 };
		float spread{ 0 };
		float fireDelay{ 0 };
		int32_t bulletsPerShot{ 1 };
		int32_t totalShots{ INT_MAX };
		Engine::String bulletTextureName;
	};

}
