#include "SpawnerExecutor.h"

#include <random>

#include <dod/BufferUtils.h>

#include <iostream>
#include <format>

namespace Game::ExecutionBlock
{

    void Spawner::initImpl() noexcept
    {

    }

    void Spawner::updateImpl(float dt) noexcept
    {

        this->stateContext.timeLeftToSpawn = std::max(0.f, this->stateContext.timeLeftToSpawn - dt);

        const auto bNeedToSpawn{ this->stateContext.timeLeftToSpawn <= 0.f && this->stateContext.spawnedSpiders < 64 };
        this->stateContext.timeLeftToSpawn += this->stateContext.spawnPeriod * bNeedToSpawn;

        this->stateContext.spawnedSpiders += bNeedToSpawn;

        const auto randX{ rand() % 500 };
        const auto randY{ rand() % 500 };

        Dod::BufferUtils::populate(this->toSpawnContext.position, Types::Coord::Vec2f(randX, randY), bNeedToSpawn);
        Dod::BufferUtils::populate(this->toSpawnContext.angle, 0.f, bNeedToSpawn);
        this->toSpawnContext.type = 1;

        if (bNeedToSpawn)
            std::cout << std::format("Spawn request: {}, x: {}, y: {}\n", bNeedToSpawn, randX, randY);
        
    }

}
