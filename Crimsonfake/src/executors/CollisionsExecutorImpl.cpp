#include "CollisionsExecutor.h"

#include <dod/BufferUtils.h>

#include <cmath>

namespace Game::ExecutionBlock
{

    void Collisions::initImpl() noexcept
    {

    }

    void Collisions::updateImpl([[maybe_unused]] float dt) noexcept
    {;
        const auto bullets{ Dod::SharedContext::get(this->inputContext).playerBullets };
        const auto enemies{ Dod::SharedContext::get(this->inputContext).enemies };
        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(bullets); ++bulletId)
        {
            const auto& bullet{ Dod::BufferUtils::get(bullets, bulletId) };

            for (int32_t enemyId{}; enemyId < Dod::BufferUtils::getNumFilledElements(enemies); ++enemyId)
            {
                const auto& enemy{ Dod::BufferUtils::get(enemies, enemyId) };

                const auto vecX{ enemy.x - bullet.x };
                const auto vecY{ enemy.y - bullet.y };
                const auto distance{ std::sqrtf(vecX * vecX + vecY * vecY) };

                const auto bCollide{ distance <= bullet.r + enemy.r };

                const auto dirX{ vecX / (distance + 0.01f) };
                const auto dirY{ vecY / (distance + 0.01f) };

                Dod::BufferUtils::populate(this->outputContext.enemyIds, enemyId, bCollide);
                Dod::BufferUtils::populate(this->outputContext.hitDirection, Types::Coord::Vec2f(dirX, dirY), bCollide);
                Dod::BufferUtils::populate(this->outputContext.bulletIds, bulletId, bCollide);
            }
        }
    }

}
