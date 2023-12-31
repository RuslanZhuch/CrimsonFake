#include "ItemsExecutor.h"

#include <dod/BufferUtils.h>
#include <dod/Algorithms.h>

namespace Game::ExecutionBlock
{

    void Items::initImpl() noexcept
    {

        this->internalContext.totalTypes = Dod::BufferUtils::getNumFilledElements(this->configContext.descriptions);

    }

    void Items::updateImpl([[maybe_unused]] float dt) noexcept
    {

        const auto toSpawnCoords{ Dod::SharedContext::get(this->commandsContext).spawnPositions };

        for (int32_t elId{}; elId < Dod::BufferUtils::getNumFilledElements(toSpawnCoords); ++elId)
        {

            const int32_t itemTypeId{ rand() % this->internalContext.totalTypes };
            const auto spawnCoord{ Dod::BufferUtils::get(toSpawnCoords, elId) };
            const auto texture{ Dod::BufferUtils::get(this->configContext.descriptions, itemTypeId).itemTexture };
            const auto size{ Dod::BufferUtils::get(this->configContext.descriptions, itemTypeId).size };
            const auto materialId{ std::hash<std::string_view>{}(texture.internalData.data()) };

            const auto bAllowSpawn{
                Dod::BufferUtils::getNumFilledElements(this->internalContext.types) <
                this->configContext.maxIntems 
            };

            Dod::BufferUtils::populate(this->internalContext.positions, spawnCoord, bAllowSpawn);
            Dod::BufferUtils::populate(this->internalContext.types, itemTypeId, bAllowSpawn);
            Dod::BufferUtils::populate(this->internalContext.materialIds, materialId, bAllowSpawn);
            Dod::BufferUtils::populate(this->internalContext.timeLeft, this->configContext.lifetime, bAllowSpawn);
            Dod::BufferUtils::populate(this->internalContext.sizes, size, bAllowSpawn);

        }

        const auto playerX{ Dod::SharedContext::get(this->playerWorldStateContext).x };
        const auto playerY{ Dod::SharedContext::get(this->playerWorldStateContext).y };

        for (int32_t elId{}; elId < Dod::BufferUtils::getNumFilledElements(this->internalContext.positions); ++elId)
        {

            const auto itemPosition{ Dod::BufferUtils::get(this->internalContext.positions, elId) };

            const auto distSqrt{
                (playerX - itemPosition.x) * (playerX - itemPosition.x) +
                (playerY - itemPosition.y) * (playerY - itemPosition.y)
            };

            const auto bCanPickup{ distSqrt <= this->configContext.radius * this->configContext.radius };

            Dod::BufferUtils::populate(this->tempContext.pickupIds, elId, bCanPickup);

        }

        for (int32_t elId{}; elId < Dod::BufferUtils::getNumFilledElements(this->tempContext.pickupIds); ++elId)
        {

            const auto itemId{ Dod::BufferUtils::get(this->tempContext.pickupIds, elId) };

            Game::Perks::Desc perk;
            perk.type = Dod::BufferUtils::get(this->internalContext.types, itemId) + 1;
            perk.coord = Dod::BufferUtils::get(this->internalContext.positions, itemId);

            Dod::BufferUtils::populate(this->perksCmdsContext.perksToActivate, perk, true);

        }

        for (int32_t elId{}; elId < Dod::BufferUtils::getNumFilledElements(this->internalContext.timeLeft); ++elId)
        {
            Dod::BufferUtils::get(this->internalContext.timeLeft, elId) -= dt;
            const auto bExpired{ Dod::BufferUtils::get(this->internalContext.timeLeft, elId) <= 0.f };
            Dod::BufferUtils::populate(this->tempContext.pickupIds, elId, bExpired);
        }

        Dod::Algorithms::leftUniques(this->tempContext.pickupIds);

        Dod::BufferUtils::remove(this->internalContext.materialIds, Dod::BufferUtils::createImFromBuffer(this->tempContext.pickupIds));
        Dod::BufferUtils::remove(this->internalContext.types, Dod::BufferUtils::createImFromBuffer(this->tempContext.pickupIds));
        Dod::BufferUtils::remove(this->internalContext.positions, Dod::BufferUtils::createImFromBuffer(this->tempContext.pickupIds));
        Dod::BufferUtils::remove(this->internalContext.timeLeft, Dod::BufferUtils::createImFromBuffer(this->tempContext.pickupIds));
        Dod::BufferUtils::remove(this->internalContext.sizes, Dod::BufferUtils::createImFromBuffer(this->tempContext.pickupIds));

        Dod::Algorithms::getSortedIndices(this->tempContext.sortedIds, Dod::BufferUtils::createImFromBuffer(this->internalContext.materialIds));
        const auto sortedMaterials{ Dod::BufferUtils::createSortedImBuffer(
            Dod::BufferUtils::createImFromBuffer(this->internalContext.materialIds), 
            Dod::BufferUtils::createImFromBuffer(this->tempContext.sortedIds))
        };

        Dod::Algorithms::countUniques(this->tempContext.elementsPerBatch, sortedMaterials);

        for (int32_t batchElId{}, globalElId{}; batchElId < Dod::BufferUtils::getNumFilledElements(this->tempContext.elementsPerBatch); ++batchElId)
        {

            const auto materialId{ Dod::BufferUtils::get(sortedMaterials, globalElId) };

            const auto totalElements{ Dod::BufferUtils::get(this->tempContext.elementsPerBatch, batchElId) };

            for (int32_t internalElId{}; internalElId < totalElements; ++internalElId, ++globalElId)
            {

                const auto sortedId{ Dod::BufferUtils::get(this->tempContext.sortedIds, globalElId) };

                const auto position{ Dod::BufferUtils::get(this->internalContext.positions, sortedId) };
                const auto size{ Dod::BufferUtils::get(this->internalContext.sizes, sortedId) };

                ProtoRenderer::transform_t transform;
                transform.translate({ position.x, position.y });
                transform.scale({ size, size });

                Dod::BufferUtils::populate(this->renderCmdsContext.commands, { transform }, true);

            }

            Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, 5, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, materialId, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.batches, { totalElements }, true);

        }

    }

}
