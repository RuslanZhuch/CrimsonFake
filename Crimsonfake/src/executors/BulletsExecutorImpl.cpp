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
        Dod::DataUtils::append(this->toRemoveContext.ids, Dod::DataUtils::createImFromBuffer(collisions));

        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(this->activeContext.timeLeft); ++bulletId)
        {
            Dod::DataUtils::get(this->activeContext.timeLeft, bulletId) -= dt;
        }

        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(this->activeContext.timeLeft); ++bulletId)
        {
            const auto timeLeft{ Dod::DataUtils::get(this->activeContext.timeLeft, bulletId) };
            Dod::DataUtils::populate(this->toRemoveContext.ids, bulletId, timeLeft <= 0.f);
        }

        Dod::Algorithms::leftUniques(this->toRemoveContext.ids);

        Dod::DataUtils::remove(this->activeContext.angle, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->activeContext.position, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->activeContext.textureNames, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->activeContext.timeLeft, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->activeContext.velocity, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::remove(this->activeContext.type, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));

        const auto toCreateTextureIds{ Dod::SharedContext::get(this->toCreateContext).textureNames };
        const auto toCreateVelocitys{ Dod::SharedContext::get(this->toCreateContext).velocity };
        const auto toCreateAngles{ Dod::SharedContext::get(this->toCreateContext).angle };
        const auto toCreateCoords{ Dod::SharedContext::get(this->toCreateContext).position };
        const auto toCreateTimeLeft{ Dod::SharedContext::get(this->toCreateContext).timeLeft };
        const auto toCreateTypes{ Dod::SharedContext::get(this->toCreateContext).type };

        Dod::DataUtils::append(this->activeContext.textureNames, Dod::DataUtils::createImFromBuffer(toCreateTextureIds));
        Dod::DataUtils::append(this->activeContext.velocity, Dod::DataUtils::createImFromBuffer(toCreateVelocitys));
        Dod::DataUtils::append(this->activeContext.angle, Dod::DataUtils::createImFromBuffer(toCreateAngles));
        Dod::DataUtils::append(this->activeContext.position, Dod::DataUtils::createImFromBuffer(toCreateCoords));
        Dod::DataUtils::append(this->activeContext.timeLeft, Dod::DataUtils::createImFromBuffer(toCreateTimeLeft));
        Dod::DataUtils::append(this->activeContext.type, Dod::DataUtils::createImFromBuffer(toCreateTypes));

        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(this->activeContext.textureNames); ++bulletId)
        {

            const auto angle{ Dod::DataUtils::get(this->activeContext.angle, bulletId) };
            const auto velocity{ Dod::DataUtils::get(this->activeContext.velocity, bulletId) };

            const auto dx{ sinf(angle) * velocity * dt };
            const auto dy{ -cosf(angle) * velocity * dt };

            Dod::DataUtils::get(this->activeContext.position, bulletId).x += dx;
            Dod::DataUtils::get(this->activeContext.position, bulletId).y += dy;

        }

        Types::Render::Cmd cmd;

        Dod::Algorithms::getSortedIndices(this->internalContext.sortedByMaterial, Dod::DataUtils::createImFromBuffer(this->activeContext.textureNames));
        
        const auto sortedMaterials{ Dod::DataUtils::createSortedImBuffer(
            Dod::DataUtils::createImFromBuffer(this->activeContext.textureNames),
            Dod::DataUtils::createImFromBuffer(this->internalContext.sortedByMaterial) 
        )};
        Dod::Algorithms::countUniques(this->internalContext.batchTotalElements, sortedMaterials);

        for (int32_t batchElId{}, globalBulletId{}; globalBulletId < Dod::DataUtils::getNumFilledElements(sortedMaterials); ++batchElId)
        {
            const auto textureId{ Dod::DataUtils::get(sortedMaterials, globalBulletId) };
            const auto totalElements{ Dod::DataUtils::get(this->internalContext.batchTotalElements, batchElId) };

            for (int32_t bulletElId{}; bulletElId < totalElements; ++bulletElId, ++globalBulletId)
            {
                const auto sortedBulletId{ Dod::DataUtils::get(this->internalContext.sortedByMaterial, globalBulletId) };
                cmd.transform = ProtoRenderer::transform_t();
                const auto coord{ Dod::DataUtils::get(this->activeContext.position, sortedBulletId) };
                cmd.transform.translate({ coord.x, coord.y });
                cmd.transform.scale({ 32.f, 32.f });
                cmd.transform.rotate(Dod::DataUtils::get(this->activeContext.angle, sortedBulletId) * 180.f / pi);
                Dod::DataUtils::populate(this->renderCmdsContext.commands, cmd, true);
            }
            Dod::DataUtils::populate(this->renderCmdsContext.batchMaterial, textureId, true);
            Dod::DataUtils::populate(this->renderCmdsContext.batchDepth, 2, true);
            Dod::DataUtils::populate(this->renderCmdsContext.batches, { totalElements }, true);
        }

        Types::Collision::Circle collision;
        collision.r = 16.f;
        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(this->activeContext.textureNames); ++bulletId)
        {

            collision.x = Dod::DataUtils::get(this->activeContext.position, bulletId).x;
            collision.y = Dod::DataUtils::get(this->activeContext.position, bulletId).y;
            const auto type{ Dod::DataUtils::get(this->activeContext.type, bulletId) };
            Dod::DataUtils::populate(this->collisionsOutputContext.playerBullets, collision, true);
            Dod::DataUtils::populate(this->collisionsOutputContext.bulletType, type, true);

        }

    }

}
