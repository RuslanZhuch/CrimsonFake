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

    void Spawner::updateImpl([[maybe_unused]] float dt) noexcept
    {

        this->stateContext.timeLeftToSpawn = std::max(0.f, this->stateContext.timeLeftToSpawn - dt);

        const auto bNeedToSpawn{ this->stateContext.timeLeftToSpawn <= 0.f && this->stateContext.spawnedSpiders < this->parametersContext.maxSpiders };
        this->stateContext.timeLeftToSpawn += this->stateContext.spawnPeriod * bNeedToSpawn;

        this->stateContext.spawnedSpiders += bNeedToSpawn;

        const auto spawnAngle{ static_cast<float>(rand() % 360) };
        const auto spawnDistance{ 750.f + static_cast<float>(rand() % 200) };

        const auto localXOffset{ -cosf(spawnAngle) * spawnDistance };
        const auto localYOffset{ -sinf(spawnAngle) * spawnDistance };

        const auto playerX{ Dod::SharedContext::get(this->playerWorldStateContext).x };
        const auto playerY{ Dod::SharedContext::get(this->playerWorldStateContext).y };

        const auto spawnX{ playerX + localXOffset };
        const auto spawnY{ playerY + localYOffset };

        Dod::BufferUtils::populate(this->toSpawnContext.position, Types::Coord::Vec2f(spawnX, spawnY), bNeedToSpawn);
        Dod::BufferUtils::populate(this->toSpawnContext.angle, 0.f, bNeedToSpawn);
        Dod::BufferUtils::populate(this->toSpawnContext.health, 5, bNeedToSpawn);
        this->toSpawnContext.type = 1;

    }

}
