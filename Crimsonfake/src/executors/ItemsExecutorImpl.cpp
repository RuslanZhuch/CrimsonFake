#include "ItemsExecutor.h"

#include <dod/BufferUtils.h>
#include <dod/TableUtils.h>
#include <dod/Algorithms.h>

namespace Game::ExecutionBlock
{

    void Items::initImpl() noexcept
    {

        this->internalContext.totalTypes = Dod::DataUtils::getNumFilledElements(this->configContext.descriptions);

    }

    void Items::updateImpl([[maybe_unused]] float dt) noexcept
    {

        const auto toSpawnCoords{ Dod::SharedContext::get(this->commandsContext).spawnPositions };

        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(toSpawnCoords); ++elId)
        {

            const int32_t itemTypeId{ rand() % this->internalContext.totalTypes };
            const auto spawnCoord{ Dod::DataUtils::get(toSpawnCoords, elId) };
            const auto texture{ Dod::DataUtils::get(this->configContext.descriptions, itemTypeId).itemTexture };
            const auto size{ Dod::DataUtils::get(this->configContext.descriptions, itemTypeId).size };
            const auto materialId{ std::hash<std::string_view>{}(texture.internalData.data()) };

            const auto bAllowSpawn{
                Dod::DataUtils::getNumFilledElements(this->internalContext.items) <
                this->configContext.maxIntems 
            };

            Game::Context::ItemsInternal::addItems(this->internalContext,
                spawnCoord,
                itemTypeId,
                materialId,
                size,
                this->configContext.lifetime,
                bAllowSpawn
            );

        }

        const auto playerX{ Dod::SharedContext::get(this->playerWorldStateContext).x };
        const auto playerY{ Dod::SharedContext::get(this->playerWorldStateContext).y };

        const auto items{ Game::Context::ItemsInternal::getItems(this->internalContext) };

        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(items.positions); ++elId)
        {

            const auto itemPosition{ Dod::DataUtils::get(items.positions, elId) };

            const auto distSqrt{
                (playerX - itemPosition.x) * (playerX - itemPosition.x) +
                (playerY - itemPosition.y) * (playerY - itemPosition.y)
            };

            const auto bCanPickup{ distSqrt <= this->configContext.radius * this->configContext.radius };

            Dod::DataUtils::populate(this->tempContext.pickupIds, elId, bCanPickup);

        }

        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(this->tempContext.pickupIds); ++elId)
        {

            const auto itemId{ Dod::DataUtils::get(this->tempContext.pickupIds, elId) };

            Game::Perks::Desc perk;
            perk.type = Dod::DataUtils::get(items.types, itemId) + 1;
            perk.coord = Dod::DataUtils::get(items.positions, itemId);

            Dod::DataUtils::populate(this->perksCmdsContext.perksToActivate, perk, true);

        }

        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(items.timeLeft); ++elId)
        {
            Dod::DataUtils::get(items.timeLeft, elId) -= dt;
            const auto bExpired{ Dod::DataUtils::get(items.timeLeft, elId) <= 0.f };
            Dod::DataUtils::populate(this->tempContext.pickupIds, elId, bExpired);
        }

        Dod::Algorithms::leftUniques(this->tempContext.pickupIds);

        Dod::DataUtils::remove(this->internalContext.items, Dod::DataUtils::createImFromBuffer(this->tempContext.pickupIds));

        Dod::Algorithms::getSortedIndices(this->tempContext.sortedIds, Dod::DataUtils::createImFromBuffer(items.materialIds));
        const auto sortedMaterials{ Dod::DataUtils::createSortedImBuffer(
            Dod::DataUtils::createImFromBuffer(items.materialIds),
            Dod::DataUtils::createImFromBuffer(this->tempContext.sortedIds))
        };

        Dod::Algorithms::countUniques(this->tempContext.elementsPerBatch, sortedMaterials);

        for (int32_t batchElId{}, globalElId{}; batchElId < Dod::DataUtils::getNumFilledElements(this->tempContext.elementsPerBatch); ++batchElId)
        {

            const auto materialId{ Dod::DataUtils::get(sortedMaterials, globalElId) };

            const auto totalElements{ Dod::DataUtils::get(this->tempContext.elementsPerBatch, batchElId) };

            for (int32_t internalElId{}; internalElId < totalElements; ++internalElId, ++globalElId)
            {

                const auto sortedId{ Dod::DataUtils::get(this->tempContext.sortedIds, globalElId) };

                const auto position{ Dod::DataUtils::get(items.positions, sortedId) };
                const auto size{ Dod::DataUtils::get(items.scales, sortedId) };

                ProtoRenderer::transform_t transform;
                transform.translate({ position.x, position.y });
                transform.scale({ size, size });

                Dod::DataUtils::populate(this->renderCmdsContext.commands, { transform }, true);

            }

            Dod::DataUtils::populate(this->renderCmdsContext.batchDepth, 5, true);
            Dod::DataUtils::populate(this->renderCmdsContext.batchMaterial, materialId, true);
            Dod::DataUtils::populate(this->renderCmdsContext.batches, { totalElements }, true);

        }

    }

}
