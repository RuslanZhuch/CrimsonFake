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

    void Bullets::updateImpl([[maybe_unused]] float dt) noexcept
    {

        const auto collisions{ Dod::SharedContext::get(this->collisionsInputContext).bulletIds };
        Dod::BufferUtils::append(this->toRemoveContext.ids, Dod::BufferUtils::createImFromBuffer(collisions));

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
        Dod::BufferUtils::remove(this->activeContext.type, Dod::BufferUtils::createImFromBuffer(this->toRemoveContext.ids));

        const auto toCreateTextureIds{ Dod::SharedContext::get(this->toCreateContext).textureNames };
        const auto toCreateVelocitys{ Dod::SharedContext::get(this->toCreateContext).velocity };
        const auto toCreateAngles{ Dod::SharedContext::get(this->toCreateContext).angle };
        const auto toCreateCoords{ Dod::SharedContext::get(this->toCreateContext).position };
        const auto toCreateTimeLeft{ Dod::SharedContext::get(this->toCreateContext).timeLeft };
        const auto toCreateTypes{ Dod::SharedContext::get(this->toCreateContext).type };

        Dod::BufferUtils::append(this->activeContext.textureNames, Dod::BufferUtils::createImFromBuffer(toCreateTextureIds));
        Dod::BufferUtils::append(this->activeContext.velocity, Dod::BufferUtils::createImFromBuffer(toCreateVelocitys));
        Dod::BufferUtils::append(this->activeContext.angle, Dod::BufferUtils::createImFromBuffer(toCreateAngles));
        Dod::BufferUtils::append(this->activeContext.position, Dod::BufferUtils::createImFromBuffer(toCreateCoords));
        Dod::BufferUtils::append(this->activeContext.timeLeft, Dod::BufferUtils::createImFromBuffer(toCreateTimeLeft));
        Dod::BufferUtils::append(this->activeContext.type, Dod::BufferUtils::createImFromBuffer(toCreateTypes));

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

        Dod::Algorithms::getSortedIndices(this->internalContext.sortedByMaterial, Dod::BufferUtils::createImFromBuffer(this->activeContext.textureNames));
        
        const auto sortedMaterials{ Dod::BufferUtils::createSortedImBuffer(
            Dod::BufferUtils::createImFromBuffer(this->activeContext.textureNames),
            Dod::BufferUtils::createImFromBuffer(this->internalContext.sortedByMaterial) 
        )};
        Dod::Algorithms::countUniques(this->internalContext.batchTotalElements, sortedMaterials);

        for (int32_t batchElId{}, globalBulletId{}; globalBulletId < Dod::BufferUtils::getNumFilledElements(sortedMaterials); ++batchElId)
        {
            const auto textureId{ Dod::BufferUtils::get(sortedMaterials, globalBulletId) };
            const auto totalElements{ Dod::BufferUtils::get(this->internalContext.batchTotalElements, batchElId) };

            for (int32_t bulletElId{}; bulletElId < totalElements; ++bulletElId, ++globalBulletId)
            {
                const auto sortedBulletId{ Dod::BufferUtils::get(this->internalContext.sortedByMaterial, globalBulletId) };
                cmd.transform = ProtoRenderer::transform_t();
                const auto coord{ Dod::BufferUtils::get(this->activeContext.position, sortedBulletId) };
                cmd.transform.translate({ coord.x, coord.y });
                cmd.transform.scale({ 32.f, 32.f });
                cmd.transform.rotate(Dod::BufferUtils::get(this->activeContext.angle, sortedBulletId) * 180.f / pi);
                Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
            }
            Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, textureId, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, 2, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.batches, { totalElements }, true);
        }

        Types::Collision::Circle collision;
        collision.r = 16.f;
        for (int32_t bulletId{}; bulletId < Dod::BufferUtils::getNumFilledElements(this->activeContext.textureNames); ++bulletId)
        {

            collision.x = Dod::BufferUtils::get(this->activeContext.position, bulletId).x;
            collision.y = Dod::BufferUtils::get(this->activeContext.position, bulletId).y;
            const auto type{ Dod::BufferUtils::get(this->activeContext.type, bulletId) };
            Dod::BufferUtils::populate(this->collisionsOutputContext.playerBullets, collision, true);
            Dod::BufferUtils::populate(this->collisionsOutputContext.bulletType, type, true);

        }

    }

}
