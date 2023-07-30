#pragma once

#include "internal/supportTypes/coordTypes.h"
#include "engine/String.h"

#include <inttypes.h>

namespace Game::Weapons
{

	struct FireCmd
	{
		Types::Coord::Vec2f spawnCoord;
		float angle{};
	};

	struct Desc
	{
		float bulletVelocity{};
		float bulletLifeTime{ 2 };
		float damage{ 1 };
		float spread{ 0 };
		float fireDelay{ 0 };
		int32_t bulletsPerShot{ 1 };
		Engine::String bulletTextureName;
	};

}
