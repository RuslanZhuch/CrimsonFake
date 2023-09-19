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

        const auto scale{ moveTime / 2.f };

        const auto time{ currentTime - scale };
        const auto output{ 4.f / scale * (-time * time + scale * scale) * bNeedMove };
        return std::min(output, 1.f);

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

    [[nodiscard]] auto getBulletDamage(
        int32_t bulletType, 
        Dod::ImBuffer<int32_t> bulletTypes,
        Dod::ImBuffer<int32_t> bulletDamage
    )
    {
        int32_t damage{};
        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(bulletTypes); ++elId)
        {
            const auto bMatch{ Dod::DataUtils::get(bulletTypes, elId) == bulletType };
            damage += Dod::DataUtils::get(bulletDamage, elId) * bMatch;
        }
        return damage;
    }

    void Enemies::initImpl() noexcept
    {

        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(this->weaponsConfigContext.descriptions); ++elId)
        {

            const auto weaponConfig{ Dod::DataUtils::get(this->weaponsConfigContext.descriptions, elId) };
            Game::Context::EnemiesInternal::addData(this->internalContext, weaponConfig.damage, weaponConfig.type);

        }

    }

    void Enemies::updateImpl([[maybe_unused]] float dt) noexcept
    {

        const auto explosionDescs{ Dod::SharedContext::get(this->explosionsSharedContext).descs };
        const auto explosionMagnitudes{ Dod::SharedContext::get(this->explosionsSharedContext).magnitudes };

        for (int32_t enemyElId{}; enemyElId < Dod::DataUtils::getNumFilledElements(this->spidersContext.position); ++enemyElId)
        {

            const auto enemyPosition{ Dod::DataUtils::get(this->spidersContext.position, enemyElId) };

            for (int32_t explosionElId{}; explosionElId < Dod::DataUtils::getNumFilledElements(explosionDescs); ++explosionElId)
            {

                const auto explosionDesc{ Dod::DataUtils::get(explosionDescs, explosionElId) };

                const auto distanceSignedX{ enemyPosition.x - explosionDesc.position.x };
                const auto distanceSignedY{ enemyPosition.y - explosionDesc.position.y };

                const auto distanceToCenter{ std::sqrtf(distanceSignedX * distanceSignedX + distanceSignedY * distanceSignedY) };

                const auto bAffected{ distanceToCenter < explosionDesc.radius };

                Dod::DataUtils::populate(this->toHitContext.ids, enemyElId, bAffected);

            }

        }

        for (int32_t id{}; id < Dod::DataUtils::getNumFilledElements(this->toHitContext.ids); ++id)
        {
            const auto enemyId{ Dod::DataUtils::get(this->toHitContext.ids, id) };
            Dod::DataUtils::get(this->spidersContext.health, enemyId) -= 10;
        }

        this->toHitContext.reset();

        const auto collisions{ Dod::SharedContext::get(this->collisionsInputContext).enemyIds };
        Dod::DataUtils::append(this->toHitContext.ids, Dod::DataUtils::createImFromBuffer(collisions));
        const auto bulletTypes{ Dod::SharedContext::get(this->collisionsInputContext).bulletTypes };

        const auto configData{ Game::Context::EnemiesInternal::getData(this->internalContext) };

        for (int32_t id{}; id < Dod::DataUtils::getNumFilledElements(this->toHitContext.ids); ++id)
        {
            const auto enemyId{ Dod::DataUtils::get(this->toHitContext.ids, id) };
            const auto bulletType{ Dod::DataUtils::get(bulletTypes, id) };
            Dod::DataUtils::get(this->spidersContext.health, enemyId) -= getBulletDamage(
                bulletType,
                Dod::DataUtils::createImFromBuffer(configData.weaponTypes),
                Dod::DataUtils::createImFromBuffer(configData.bulletsDamage)
            );
        }

        constexpr auto offset{ 10.f };
        const auto hits{ Dod::SharedContext::get(this->collisionsInputContext).hitDirection };
        for (int32_t id{}; id < Dod::DataUtils::getNumFilledElements(hits); ++id)
        {
            const auto enemyId{ Dod::DataUtils::get(this->toHitContext.ids, id) };
            const auto hit{ Dod::DataUtils::get(hits, id) };
            Dod::DataUtils::get(this->spidersContext.position, enemyId).x += hit.x * offset;
            Dod::DataUtils::get(this->spidersContext.position, enemyId).y += hit.y * offset;
        }

        const auto bloodDecalName{ std::hash<std::string_view>{}("alienBlood01.png") };
        for (int32_t id{}; id < Dod::DataUtils::getNumFilledElements(toHitContext.ids); ++id)
        {
            const auto toRemoveId{ Dod::DataUtils::get(toHitContext.ids, id) };
            const auto position{ Dod::DataUtils::get(this->spidersContext.position, toRemoveId) };

            constexpr auto diameter{ 64 };

            const auto randOffsetX{ static_cast<float>(rand() % diameter) };
            const auto randOffsetY{ static_cast<float>(rand() % diameter) };

            Types::Decals::Cmd cmd;
            cmd.angle = static_cast<float>(rand() % 128);
            cmd.scale = 32.f;
            cmd.position.x = position.x - (randOffsetX - diameter / 2);
            cmd.position.y = position.y - (randOffsetY - diameter / 2);
            Dod::DataUtils::populate(this->decalsCmdsContext.commands, cmd, true);
            Dod::DataUtils::populate(this->decalsCmdsContext.texture, bloodDecalName, true);
        }

        for (int32_t id{}; id < Dod::DataUtils::getNumFilledElements(this->spidersContext.health); ++id)
        {
            const auto health = Dod::DataUtils::get(this->spidersContext.health, id);
            const auto bNeedRemove{ health <= 0 };
            Dod::DataUtils::populate(this->toRemoveContext.ids, id, bNeedRemove);
        }

        Dod::Algorithms::leftUniques(this->toRemoveContext.ids);

        for (int32_t id{}; id < Dod::DataUtils::getNumFilledElements(toRemoveContext.ids); ++id)
        {
            const auto toRemoveId{ Dod::DataUtils::get(toRemoveContext.ids, id) };
            const auto position{ Dod::DataUtils::get(this->spidersContext.position, toRemoveId) };

            const auto bAllowToSpawn{ (rand() % 100) <= static_cast<int32_t>(100 * this->parametersContext.itemSpawnProbability) };

            Dod::DataUtils::populate(this->itemsCmdsContext.spawnPositions, position, bAllowToSpawn);
        }

        Dod::DataUtils::remove(this->spidersContext.position, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->spidersContext.angle, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->spidersContext.health, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));

        const auto toSpawnPosition{ Dod::SharedContext::get(this->toSpawnContext).position };
        const auto toSpawnAngle{ Dod::SharedContext::get(this->toSpawnContext).angle };
        [[maybe_unused]] const auto toSpawnType{ Dod::SharedContext::get(this->toSpawnContext).type };
        const auto toSpawnHealth{ Dod::SharedContext::get(this->toSpawnContext).health };

        Dod::DataUtils::append(this->spidersContext.position, Dod::DataUtils::createImFromBuffer(toSpawnPosition));
        Dod::DataUtils::append(this->spidersContext.angle, Dod::DataUtils::createImFromBuffer(toSpawnAngle));
        Dod::DataUtils::append(this->spidersContext.health, Dod::DataUtils::createImFromBuffer(toSpawnHealth));

        const auto playerX{ Dod::SharedContext::get(this->playerWorldStateContext).x };
        const auto playerY{ Dod::SharedContext::get(this->playerWorldStateContext).y };

        for (int32_t enemyLeftId{}; enemyLeftId < Dod::DataUtils::getNumFilledElements(this->spidersContext.position); ++enemyLeftId)
        {

            const auto leftPosition{ Dod::DataUtils::get(this->spidersContext.position, enemyLeftId) };
            auto leftTotalPushX{ 0.f };
            auto leftTotalPushY{ 0.f };

            for (int32_t enemyRightId{ enemyLeftId + 1 }; enemyRightId < Dod::DataUtils::getNumFilledElements(this->spidersContext.position); ++enemyRightId)
            {

                const auto rightPosition{ Dod::DataUtils::get(this->spidersContext.position, enemyRightId) };

                const auto vecX{ leftPosition.x - rightPosition.x };
                const auto vecY{ leftPosition.y - rightPosition.y };

                const auto dist{ std::sqrtf(vecX * vecX + vecY * vecY) + 0.01f };
                const auto dirX{ vecX / dist };
                const auto dirY{ vecY / dist };

                const auto overlap{ std::max(64.f - dist, 0.f) };

                leftTotalPushX += dirX * overlap;
                leftTotalPushY += dirY * overlap;

                Dod::DataUtils::get(this->spidersContext.position, enemyRightId).x -= dirX * overlap;
                Dod::DataUtils::get(this->spidersContext.position, enemyRightId).y -= dirY * overlap;

            }

            Dod::DataUtils::get(this->spidersContext.position, enemyLeftId).x += leftTotalPushX;
            Dod::DataUtils::get(this->spidersContext.position, enemyLeftId).y += leftTotalPushX;

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

        for (int32_t enemyId{}; enemyId < Dod::DataUtils::getNumFilledElements(this->spidersContext.position); ++enemyId)
        {

            const auto enemyPosition{ Dod::DataUtils::get(this->spidersContext.position, enemyId) };

            const auto vecX{ playerX - enemyPosition.x };
            const auto vecY{ playerY - enemyPosition.y };

            const auto dist{ std::sqrtf(vecX * vecX + vecY * vecY) + 0.01f };
            const auto needDirX{ vecX / dist };
            const auto needDirY{ vecY / dist };

            const auto currentAngle{ Dod::DataUtils::get(this->spidersContext.angle, enemyId) };
            const auto currentDirX{ sinf(currentAngle) };
            const auto currentDirY{ -cosf(currentAngle) };

            const auto control{ cross({needDirX, needDirY}, {-currentDirX, -currentDirY}) };

            const auto moveX{ currentDirX * velocity * dt };
            const auto moveY{ currentDirY * velocity * dt };

            Dod::DataUtils::get(this->spidersContext.position, enemyId).x += moveX;
            Dod::DataUtils::get(this->spidersContext.position, enemyId).y += moveY;
            Dod::DataUtils::get(this->spidersContext.angle, enemyId) += control * this->parametersContext.rotationSpeed * dt * (velocity > 0.f);

        }

        const auto textureKey{ std::hash<std::string_view>{}("spider.png") };
        Types::Render::Cmd cmd;
        const auto totalSpiders{ Dod::DataUtils::getNumFilledElements(this->spidersContext.position) };
        for (int32_t enemyId{}; enemyId < totalSpiders; ++enemyId)
        {

            cmd.transform = ProtoRenderer::transform_t();
            const auto coord{ Dod::DataUtils::get(this->spidersContext.position, enemyId) };
            cmd.transform.translate({ coord.x, coord.y });
            cmd.transform.scale({ 32.f, 32.f });
            cmd.transform.rotate(Dod::DataUtils::get(this->spidersContext.angle, enemyId) * 180.f / pi);
            Dod::DataUtils::populate(this->renderCmdsContext.commands, cmd, true);

        }
        Dod::DataUtils::populate(this->renderCmdsContext.batchMaterial, textureKey, totalSpiders > 0);
        Dod::DataUtils::populate(this->renderCmdsContext.batchDepth, 10, totalSpiders > 0);
        Dod::DataUtils::populate(this->renderCmdsContext.batches, { totalSpiders }, totalSpiders > 0);

        Types::Collision::Circle collision;
        collision.r = 32.f;
        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(this->spidersContext.position); ++bulletId)
        {

            collision.x = Dod::DataUtils::get(this->spidersContext.position, bulletId).x;
            collision.y = Dod::DataUtils::get(this->spidersContext.position, bulletId).y;
            Dod::DataUtils::populate(this->collisionsOutputContext.enemies, collision, true);

        }

    }

}
