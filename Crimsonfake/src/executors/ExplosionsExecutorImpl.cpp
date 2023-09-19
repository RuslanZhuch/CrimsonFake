#include "ExplosionsExecutor.h"

#include <dod/BufferUtils.h>

namespace Game::ExecutionBlock
{

    void Explosions::initImpl() noexcept
    {
        const auto materialId{ std::hash<std::string_view>{}(this->configContext.texture.internalData.data()) };
        this->internalContext.materialId = materialId;
    }

    void Explosions::updateImpl([[maybe_unused]] float dt) noexcept
    {

        const auto spawnCommands{ Dod::SharedContext::get(this->commandsContext).spawn };
        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(spawnCommands); ++elId)
        {

            const auto cmd{ Dod::DataUtils::get(spawnCommands, elId) };

            Dod::DataUtils::populate(this->internalContext.maxRadius, cmd.desc.radius, true);

            Game::Explosions::Desc descToCreate;
            descToCreate.position = cmd.desc.position;
            Dod::DataUtils::populate(this->activeContext.descs, descToCreate, true);

            Dod::DataUtils::populate(this->activeContext.magnitudes, cmd.magnitude, true);

        }

        const auto totalActiveExplosions{ Dod::DataUtils::getNumFilledElements(this->activeContext.descs) };
        for (int32_t elId{}; elId < totalActiveExplosions; ++elId)
        {

            const auto targetRadius{ Dod::DataUtils::get(this->internalContext.maxRadius, elId) };
            const auto expandDelta{ targetRadius / this->configContext.explosionTime * dt };

            auto& desc{ Dod::DataUtils::get(this->activeContext.descs, elId) };
            desc.radius += expandDelta;

            const auto bNeedRemove{ desc.radius >= targetRadius };
            Dod::DataUtils::populate(this->tempContext.toRemove, elId, bNeedRemove);

        }

        const auto bNeedRenderExplosions{ totalActiveExplosions > 0 };

        for (int32_t elId{}; elId < totalActiveExplosions; ++elId)
        {

            const auto desc{ Dod::DataUtils::get(this->activeContext.descs, elId) };

            ProtoRenderer::transform_t transform;
            transform.translate({ desc.position.x, desc.position.y });
            transform.scale({ desc.radius, desc.radius });

            Dod::DataUtils::populate(this->renderCmdsContext.commands, { transform }, true);

        }

        Dod::DataUtils::populate(this->renderCmdsContext.batches, { totalActiveExplosions }, bNeedRenderExplosions);
        Dod::DataUtils::populate(this->renderCmdsContext.batchDepth, 20, bNeedRenderExplosions);
        Dod::DataUtils::populate(this->renderCmdsContext.batchMaterial, this->internalContext.materialId, bNeedRenderExplosions);

        Dod::DataUtils::remove(this->activeContext.descs, Dod::DataUtils::createImFromBuffer(this->tempContext.toRemove));
        Dod::DataUtils::remove(this->activeContext.magnitudes, Dod::DataUtils::createImFromBuffer(this->tempContext.toRemove));

    }

}
