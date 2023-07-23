#include "EnemiesExecutor.h"

#include <dod/BufferUtils.h>

#include <numbers>

static constexpr auto pi{ static_cast<float>(std::numbers::pi) };
namespace Game::ExecutionBlock
{

    void Enemies::initImpl() noexcept
    {

    }

    void Enemies::updateImpl(float dt) noexcept
    {

        const auto toSpawnPosition{ Dod::SharedContext::get(this->toSpawnContext).position };
        const auto toSpawnAngle{ Dod::SharedContext::get(this->toSpawnContext).angle };
        const auto toSpawnType{ Dod::SharedContext::get(this->toSpawnContext).type };

        Dod::BufferUtils::append(this->spidersContext.position, Dod::BufferUtils::createImFromBuffer(toSpawnPosition));
        Dod::BufferUtils::append(this->spidersContext.angle, Dod::BufferUtils::createImFromBuffer(toSpawnAngle));

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

    }

}
