#include "EnemiesExecutor.h"

#include <dod/BufferUtils.h>
#include <dod/Algorithms.h>

#include <numbers>

static constexpr auto pi{ static_cast<float>(std::numbers::pi) };
namespace Game::ExecutionBlock
{

    void Enemies::initImpl() noexcept
    {

    }

    void Enemies::updateImpl(float dt) noexcept
    {

        constexpr auto offset{ 10.f };
        const auto bullets{ Dod::SharedContext::get(this->collisionsInputContext).playerBullets };
        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(bullets); ++bulletId)
        {
            const auto& bullet{ Dod::BufferUtils::get(bullets, bulletId) };

            for (int32_t enemyId{}; enemyId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++enemyId)
            {
                constexpr auto enemyRadius{ 32.f };
                const auto enemyPosition{ Dod::BufferUtils::get(this->spidersContext.position, enemyId) };
                const auto vecX{ enemyPosition.x - bullet.x };
                const auto vecY{ enemyPosition.y - bullet.y };
                const auto distance{ std::sqrtf(vecX * vecX + vecY * vecY) };

                const auto bCollide{ distance <= bullet.r + enemyRadius };

                const auto dirX{ vecX / (distance + 0.01f) };
                const auto dirY{ vecY / (distance + 0.01f) };

                Dod::BufferUtils::get(this->spidersContext.position, enemyId).x += dirX * offset * bCollide;
                Dod::BufferUtils::get(this->spidersContext.position, enemyId).y += dirY * offset * bCollide;
                Dod::BufferUtils::get(this->spidersContext.health, enemyId) -= bCollide;

            }
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

        for (int32_t enemyId{}; enemyId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++enemyId)
        {

            const auto enemyPosition{ Dod::BufferUtils::get(this->spidersContext.position, enemyId) };

            const auto vecX{ playerX - enemyPosition.x };
            const auto vecY{ playerY - enemyPosition.y };

            const auto dist{ std::sqrtf(vecX * vecX + vecY * vecY) + 0.01f };
            const auto dirX{ vecX / dist };
            const auto dirY{ vecY / dist };

            constexpr auto velocity{ 50.f };

            const auto moveX{ dirX * velocity * dt };
            const auto moveY{ dirY * velocity * dt };

            Dod::BufferUtils::get(this->spidersContext.position, enemyId).x += moveX;
            Dod::BufferUtils::get(this->spidersContext.position, enemyId).y += moveY;

            const auto angle{ atanf(dirY / dirX) + pi / 2.f + pi * (dirX < 0) };
            Dod::BufferUtils::get(this->spidersContext.angle, enemyId) = angle;

        }

        const auto textureKey{ std::hash<std::string_view>{}("spider.png") };
        Types::Render::Cmd cmd;
        for (int32_t enemyId{}; enemyId < Dod::BufferUtils::getNumFilledElements(this->spidersContext.position); ++enemyId)
        {

            cmd.transform = ProtoRenderer::transform_t();
            const auto coord{ Dod::BufferUtils::get(this->spidersContext.position, enemyId) };
            cmd.transform.translate({ coord.x, coord.y });
            cmd.transform.scale({ 32.f, 32.f });
            cmd.transform.rotate(Dod::BufferUtils::get(this->spidersContext.angle, enemyId) * 180.f / pi);
            Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.materialNames, textureKey, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.depth, 10, true);

        }

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
