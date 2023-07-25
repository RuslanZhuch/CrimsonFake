#include "BulletsExecutor.h"

#include <dod/BufferUtils.h>
#include <dod/Algorithms.h>

#include <iostream>
#include <format>
#include <numbers>

static constexpr auto pi{ static_cast<float>(std::numbers::pi) };
namespace Game::ExecutionBlock
{

    void Bullets::initImpl() noexcept
    {

    }

    void Bullets::updateImpl(float dt) noexcept
    {

        const auto enemies{ Dod::SharedContext::get(this->collisionsInputContext).enemies };
        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.position); ++bulletId)
        {
            const auto bulletPosition{ Dod::BufferUtils::get(this->activeContext.position, bulletId) };
            const auto bulletRadius{ 16.f };

            for (int32_t enemyId{}; enemyId < Dod::BufferUtils::getNumFilledElements(enemies); ++enemyId)
            {
                const auto& enemy{ Dod::BufferUtils::get(enemies, enemyId) };
                const auto vecX{ enemy.x - bulletPosition.x };
                const auto vecY{ enemy.y - bulletPosition.y };
                const auto distance{ std::sqrtf(vecX * vecX + vecY * vecY) };

                const auto bCollide{ distance <= bulletRadius + enemy.r };

                Dod::BufferUtils::populate(this->toRemoveContext.ids, bulletId, bCollide);
            }
        }

//        std::cout << std::format("act {}, new {}\n", 
//            Dod::BufferUtils::getNumFilledElements(this->activeContext.position), 
//            Dod::BufferUtils::getNumFilledElements(toCreateCoords)
//        );

        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.timeLeft); ++bulletId)
        {
            Dod::BufferUtils::get(this->activeContext.timeLeft, bulletId) -= dt;
        }

        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.timeLeft); ++bulletId)
        {
            const auto timeLeft{ Dod::BufferUtils::get(this->activeContext.timeLeft, bulletId) };
            Dod::BufferUtils::populate(this->toRemoveContext.ids, bulletId, timeLeft <= 0.f);
        }

        Dod::Algorithms::leftUniques(this->toRemoveContext.ids);

        Dod::BufferUtils::remove(this->activeContext.angle, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::BufferUtils::remove(this->activeContext.position, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::BufferUtils::remove(this->activeContext.textureNames, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::BufferUtils::remove(this->activeContext.timeLeft, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::BufferUtils::remove(this->activeContext.velocity, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));

        const auto toCreateTextureIds{ Dod::SharedContext::get(this->toCreateContext).textureNames };
        const auto toCreateVelocitys{ Dod::SharedContext::get(this->toCreateContext).velocity };
        const auto toCreateAngles{ Dod::SharedContext::get(this->toCreateContext).angle };
        const auto toCreateCoords{ Dod::SharedContext::get(this->toCreateContext).position };
        const auto toCreateTimeLeft{ Dod::SharedContext::get(this->toCreateContext).timeLeft };

        Dod::BufferUtils::append(this->activeContext.textureNames, Dod::BufferUtils::createImFromBuffer(toCreateTextureIds));
        Dod::BufferUtils::append(this->activeContext.velocity, Dod::BufferUtils::createImFromBuffer(toCreateVelocitys));
        Dod::BufferUtils::append(this->activeContext.angle, Dod::BufferUtils::createImFromBuffer(toCreateAngles));
        Dod::BufferUtils::append(this->activeContext.position, Dod::BufferUtils::createImFromBuffer(toCreateCoords));
        Dod::BufferUtils::append(this->activeContext.timeLeft, Dod::BufferUtils::createImFromBuffer(toCreateTimeLeft));


        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.textureNames); ++bulletId)
        {

            const auto angle{ Dod::BufferUtils::get(this->activeContext.angle, bulletId) };
            const auto velocity{ Dod::BufferUtils::get(this->activeContext.velocity, bulletId) };

            const auto dx{ sinf(angle) * velocity * dt };
            const auto dy{ -cosf(angle) * velocity * dt };

            Dod::BufferUtils::get(this->activeContext.position, bulletId).x += dx;
            Dod::BufferUtils::get(this->activeContext.position, bulletId).y += dy;

        }

        Types::Render::Cmd cmd;

        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.textureNames); ++bulletId)
        {

            cmd.transform = ProtoRenderer::transform_t();
            const auto coord{ Dod::BufferUtils::get(this->activeContext.position, bulletId) };
            cmd.transform.translate({ coord.x, coord.y });
            cmd.transform.scale({ 32.f, 32.f });
            cmd.transform.rotate(Dod::BufferUtils::get(this->activeContext.angle, bulletId) * 180.f / pi);
            Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.materialNames, Dod::BufferUtils::get(this->activeContext.textureNames, bulletId), true);
            Dod::BufferUtils::populate(this->renderCmdsContext.depth, 2, true);

        }

        Types::Collision::Circle collision;
        collision.r = 16.f;
        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.textureNames); ++bulletId)
        {

            collision.x = Dod::BufferUtils::get(this->activeContext.position, bulletId).x;
            collision.y = Dod::BufferUtils::get(this->activeContext.position, bulletId).y;
            Dod::BufferUtils::populate(this->collisionsOutputContext.playerBullets, collision, true);

        }

    }

}
