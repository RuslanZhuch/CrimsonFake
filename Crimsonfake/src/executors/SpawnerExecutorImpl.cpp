#include "SpawnerExecutor.h"

#include <random>

#include <dod/BufferUtils.h>

#include <iostream>
#include <format>

namespace Game::ExecutionBlock
{

    static void spawnSpiders(Context::Enemies::Data& toSpawnContext, float playerX, float playerY, bool bNeedToSpawn)
    {
        const auto spawnAngle{ static_cast<float>(rand() % 360) };
        const auto spawnDistance{ 750.f + static_cast<float>(rand() % 200) };

        const auto localXOffset{ -cosf(spawnAngle) * spawnDistance };
        const auto localYOffset{ -sinf(spawnAngle) * spawnDistance };

        const auto spawnX{ playerX + localXOffset };
        const auto spawnY{ playerY + localYOffset };

        Dod::BufferUtils::populate(toSpawnContext.position, Types::Coord::Vec2f(spawnX, spawnY), bNeedToSpawn);
        Dod::BufferUtils::populate(toSpawnContext.angle, 0.f, bNeedToSpawn);
        Dod::BufferUtils::populate(toSpawnContext.health, 5, bNeedToSpawn);
        toSpawnContext.type = 1;
    }

    void Spawner::initImpl() noexcept
    {

    }

    void Spawner::updateImpl([[maybe_unused]] float dt) noexcept
    {

        this->stateContext.timeLeftToSpawn = std::max(0.f, this->stateContext.timeLeftToSpawn - dt);

        const auto bNeedToSpawnSpider{ this->stateContext.timeLeftToSpawn <= 0.f && this->stateContext.spawnedSpiders < this->parametersContext.maxSpiders };
        this->stateContext.timeLeftToSpawn += this->stateContext.spawnPeriod * bNeedToSpawnSpider;
        this->stateContext.spawnedSpiders += bNeedToSpawnSpider;

        const auto playerX{ Dod::SharedContext::get(this->playerWorldStateContext).x };
        const auto playerY{ Dod::SharedContext::get(this->playerWorldStateContext).y };

        spawnSpiders(
            this->toSpawnContext,
            playerX,
            playerY,
            bNeedToSpawnSpider
        );

    }

}
