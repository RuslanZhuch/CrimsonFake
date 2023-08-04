#include "EnemiesExecutor.h"

#include <dod/BufferUtils.h>
#include <dod/Algorithms.h>

#include <numbers>
#include <iostream>
#include <format>

static constexpr auto pi{ static_cast<float>(std::numbers::pi) };
namespace Game::ExecutionBlock
{

    [[nodiscard]] static auto generateVelocityNorm(
        float currentTime,
        float period,
        float standTime
    )
    {

        const auto moveTime{ period - standTime };
        const auto bNeedMove{ currentTime <= moveTime };

        const auto normMoveTime{ currentTime / moveTime };

        const auto scale{ moveTime / 2.f };

        const auto time{ currentTime - scale };
        const auto output{ 4.f / scale * (-time * time + scale * scale) * bNeedMove };
        return std::min(output, 1.f);

    }

    [[nodiscard]] static auto applyVelocity(
        float velocityTime,
        float minVeloicty,
        float maxVelocity
    )
    {

        return minVeloicty + (maxVelocity - minVeloicty) * velocityTime;

    }

    [[nodiscard]] static auto updateCurrentMoveTime(
        float currentTime,
        float period,
        float dt
    )
    {

        const auto newTime{ currentTime + dt };
        const auto overflow{ std::min(newTime - period, period) };

        if (overflow > 0.f)
            return overflow;

        return newTime;

    }

    void Enemies::initImpl() noexcept
    {

    }

    void Enemies::updateImpl(float dt) noexcept
    {

        const auto collisions{ Dod::SharedContext::get(this->collisionsInputContext).enemyIds };
        Dod::BufferUtils::append(this->toHitContext.ids, Dod::BufferUtils::createImFromBuffer(collisions));

        for (int32_t id{}; id < Dod::BufferUtils::getNumFilledElements(this->toHitContext.ids); ++id)
        {
            const auto enemyId{ Dod::BufferUtils::get(this->toHitContext.ids, id) };
            Dod::BufferUtils::get(this->spidersContext.health, enemyId) -= 1;
        }

        constexpr auto offset{ 10.f };
        const auto hits{ Dod::SharedContext::get(this->collisionsInputContext).hitDirection };
        for (int32_t id{}; id < Dod::BufferUtils::getNumFilledElements(hits); ++id)
        {
            const auto enemyId{ Dod::BufferUtils::get(this->toHitContext.ids, id) };
            const auto hit{ Dod::BufferUtils::get(hits, id) };
            Dod::BufferUtils::get(this->spidersContext.position, enemyId).x += hit.x * offset;
            Dod::BufferUtils::get(this->spidersContext.position, enemyId).y += hit.y * offset;
        }

        const auto bloodDecalName{ std::hash<std::string_view>{}("alienBlood01.png") };
        for (int32_t id{}; id < Dod::BufferUtils::getNumFilledElements(toHitContext.ids); ++id)
        {
            const auto toRemoveId{ Dod::BufferUtils::get(toHitContext.ids, id) };
            const auto position{ Dod::BufferUtils::get(this->spidersContext.position, toRemoveId) };

            const auto randOffsetX{ rand() % 64 };
            const auto randOffsetY{ rand() % 64 };

            Types::Decals::Cmd cmd;
            cmd.angle = rand() % 128;
            cmd.scale = 32.f;
            cmd.position.x = position.x - (randOffsetX - 64 / 2);
            cmd.position.y = position.y - (randOffsetY - 64 / 2);
            Dod::BufferUtils::populate(this->decalsCmdsContext.commands, cmd, true);
            Dod::BufferUtils::populate(this->decalsCmdsContext.texture, bloodDecalName, true);
        }

        for (int32_t id{}; id < Dod::BufferUtils::getNumFilledElements(this->spidersContext.health); ++id)
        {
            const auto health = Dod::BufferUtils::get(this->spidersContext.health, id);
            const auto bNeedRemove{ health <= 0 };
            Dod::BufferUtils::populate(this->toRemoveContext.ids, id, bNeedRemove);
        }

        Dod::Algorithms::leftUniques(this->toRemoveContext.ids);

        Dod::BufferUtils::remove(this->spidersContext.position, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::BufferUtils::remove(this->spidersContext.angle, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::BufferUtils::remove(this->spidersContext.health, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));

        const auto toSpawnPosition{ Dod::SharedContext::get(this->toSpawnContext).position };
        const auto toSpawnAngle{ Dod::SharedContext::get(this->toSpawnContext).angle };
        const auto toSpawnType{ Dod::SharedContext::get(this->toSpawnContext).type };
        const auto toSpawnHealth{ Dod::SharedContext::get(this->toSpawnContext).health };

        Dod::BufferUtils::append(this->spidersContext.position, Dod::BufferUtils::createImFromBuffer(toSpawnPosition));
        Dod::BufferUtils::append(this->spidersContext.angle, Dod::BufferUtils::createImFromBuffer(toSpawnAngle));
        Dod::BufferUtils::append(this->spidersContext.health, Dod::BufferUtils::createImFromBuffer(toSpawnHealth));

        const auto playerX{ Dod::SharedContext::get(this->playerWorldStateContext).x };
        const auto playerY{ Dod::SharedContext::get(this->playerWorldStateContext).y };

        for (int32_t enemyLeftId{}; enemyLeftId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++enemyLeftId)
        {

            const auto leftPosition{ Dod::BufferUtils::get(this->spidersContext.position, enemyLeftId) };
            auto leftTotalPushX{ 0.f };
            auto leftTotalPushY{ 0.f };

            for (int32_t enemyRightId{ enemyLeftId + 1 }; enemyRightId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++enemyRightId)
            {

                const auto rightPosition{ Dod::BufferUtils::get(this->spidersContext.position, enemyRightId) };

                const auto vecX{ leftPosition.x - rightPosition.x };
                const auto vecY{ leftPosition.y - rightPosition.y };

                const auto dist{ std::sqrtf(vecX * vecX + vecY * vecY) + 0.01f };
                const auto dirX{ vecX / dist };
                const auto dirY{ vecY / dist };

                const auto overlap{ std::max(64.f - dist, 0.f) };

                leftTotalPushX += dirX * overlap;
                leftTotalPushY += dirY * overlap;

                Dod::BufferUtils::get(this->spidersContext.position, enemyRightId).x -= dirX * overlap;
                Dod::BufferUtils::get(this->spidersContext.position, enemyRightId).y -= dirY * overlap;

            }

            Dod::BufferUtils::get(this->spidersContext.position, enemyLeftId).x += leftTotalPushX;
            Dod::BufferUtils::get(this->spidersContext.position, enemyLeftId).y += leftTotalPushX;

        }

        this->stateContext.currentTime = updateCurrentMoveTime(
            this->stateContext.currentTime,
            this->parametersContext.movePeriod,
            dt
        );

        const auto velocity{ 
            generateVelocityNorm(this->stateContext.currentTime, this->parametersContext.movePeriod, this->parametersContext.standTime) * 
            this->parametersContext.topVelocity 
        };

        const auto dot = [](Types::Coord::Vec2f left, Types::Coord::Vec2f right) -> float {
            return left.x * right.x + left.y * right.y;
        };
        const auto cross = [](Types::Coord::Vec2f left, Types::Coord::Vec2f right) -> float {
            return left.x * right.y - left.y * right.x;
        };

        for (int32_t enemyId{}; enemyId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++enemyId)
        {

            const auto enemyPosition{ Dod::BufferUtils::get(this->spidersContext.position, enemyId) };

            const auto vecX{ playerX - enemyPosition.x };
            const auto vecY{ playerY - enemyPosition.y };

            const auto dist{ std::sqrtf(vecX * vecX + vecY * vecY) + 0.01f };
            const auto needDirX{ vecX / dist };
            const auto needDirY{ vecY / dist };

            const auto currentAngle{ Dod::BufferUtils::get(this->spidersContext.angle, enemyId) };
            const auto currentDirX{ sinf(currentAngle) };
            const auto currentDirY{ -cosf(currentAngle) };

            const auto control{ cross({needDirX, needDirY}, {-currentDirX, -currentDirY}) };

            const auto moveX{ currentDirX * velocity * dt };
            const auto moveY{ currentDirY * velocity * dt };

            Dod::BufferUtils::get(this->spidersContext.position, enemyId).x += moveX;
            Dod::BufferUtils::get(this->spidersContext.position, enemyId).y += moveY;
            Dod::BufferUtils::get(this->spidersContext.angle, enemyId) += control * this->parametersContext.rotationSpeed * dt * (velocity > 0.f);

        }

        const auto textureKey{ std::hash<std::string_view>{}("spider.png") };
        Types::Render::Cmd cmd;
        const auto totalSpiders{ Dod::BufferUtils::getNumFilledElements(this->spidersContext.position) };
        for (int32_t enemyId{}; enemyId < totalSpiders; ++enemyId)
        {

            cmd.transform = ProtoRenderer::transform_t();
            const auto coord{ Dod::BufferUtils::get(this->spidersContext.position, enemyId) };
            cmd.transform.translate({ coord.x, coord.y });
            cmd.transform.scale({ 32.f, 32.f });
            cmd.transform.rotate(Dod::BufferUtils::get(this->spidersContext.angle, enemyId) * 180.f / pi);
            Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);

        }
        Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, textureKey, totalSpiders > 0);
        Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, 10, totalSpiders > 0);
        Dod::BufferUtils::populate(this->renderCmdsContext.batches, { totalSpiders }, totalSpiders > 0);

        Types::Collision::Circle collision;
        collision.r = 32.f;
        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++bulletId)
        {

            collision.x = Dod::BufferUtils::get(this->spidersContext.position, bulletId).x;
            collision.y = Dod::BufferUtils::get(this->spidersContext.position, bulletId).y;
            Dod::BufferUtils::populate(this->collisionsOutputContext.enemies, collision, true);

        }

    }

}
