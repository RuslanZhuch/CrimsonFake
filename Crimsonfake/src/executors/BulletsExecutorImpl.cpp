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

        const auto prevActiveBullets{ Game::Context::Bullets::getStates(this->activeContext) };

        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(prevActiveBullets.timeLeft); ++bulletId)
        {
            Dod::DataUtils::get(prevActiveBullets.timeLeft, bulletId) -= dt;
        }

        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(prevActiveBullets.timeLeft); ++bulletId)
        {
            const auto timeLeft{ Dod::DataUtils::get(prevActiveBullets.timeLeft, bulletId) };
            Dod::DataUtils::populate(this->toRemoveContext.ids, bulletId, timeLeft <= 0.f);
        }

        Dod::Algorithms::leftUniques(this->toRemoveContext.ids);

        Dod::DataUtils::remove(this->activeContext.states, Dod::DataUtils::createImFromBuffer(this->toRemoveContext.ids));
        Dod::DataUtils::append(this->activeContext.states, this->toCreateContext->context.states);

        const auto activeBullets{ Game::Context::Bullets::getStates(this->activeContext) };
        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(activeBullets.textureNames); ++bulletId)
        {

            const auto angle{ Dod::DataUtils::get(activeBullets.angle, bulletId) };
            const auto velocity{ Dod::DataUtils::get(activeBullets.velocity, bulletId) };

            const auto dx{ sinf(angle) * velocity * dt };
            const auto dy{ -cosf(angle) * velocity * dt };

            Dod::DataUtils::get(activeBullets.position, bulletId).x += dx;
            Dod::DataUtils::get(activeBullets.position, bulletId).y += dy;

        }

        Types::Render::Cmd cmd;

        Dod::Algorithms::getSortedIndices(this->internalContext.sortedByMaterial, Dod::DataUtils::createImFromBuffer(activeBullets.textureNames));
        
        const auto sortedMaterials{ Dod::DataUtils::createSortedImBuffer(
            Dod::DataUtils::createImFromBuffer(activeBullets.textureNames),
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
                const auto coord{ Dod::DataUtils::get(activeBullets.position, sortedBulletId) };
                cmd.transform.translate({ coord.x, coord.y });
                cmd.transform.scale({ 32.f, 32.f });
                cmd.transform.rotate(Dod::DataUtils::get(activeBullets.angle, sortedBulletId) * 180.f / pi);
                Dod::DataUtils::populate(this->renderCmdsContext.commands, cmd, true);
            }
            Dod::DataUtils::populate(this->renderCmdsContext.batchMaterial, textureId, true);
            Dod::DataUtils::populate(this->renderCmdsContext.batchDepth, 2, true);
            Dod::DataUtils::populate(this->renderCmdsContext.batches, { totalElements }, true);
        }

        Types::Collision::Circle collision;
        collision.r = 16.f;
        for (int32_t bulletId{}; bulletId < Dod::DataUtils::getNumFilledElements(this->activeContext.states); ++bulletId)
        {

            collision.x = Dod::DataUtils::get(activeBullets.position, bulletId).x;
            collision.y = Dod::DataUtils::get(activeBullets.position, bulletId).y;
            const auto type{ Dod::DataUtils::get(activeBullets.type, bulletId) };
            Dod::DataUtils::populate(this->collisionsOutputContext.playerBullets, collision, true);
            Dod::DataUtils::populate(this->collisionsOutputContext.bulletType, type, true);

        }

    }

}
