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
        for (int32_t elId{}; elId < Dod::BufferUtils::getNumFilledElements(spawnCommands); ++elId)
        {

            const auto cmd{ Dod::BufferUtils::get(spawnCommands, elId) };

            Dod::BufferUtils::populate(this->internalContext.maxRadius, cmd.desc.radius, true);

            Game::Explosions::Desc descToCreate;
            descToCreate.position = cmd.desc.position;
            Dod::BufferUtils::populate(this->activeContext.descs, descToCreate, true);

            Dod::BufferUtils::populate(this->activeContext.magnitudes, cmd.magnitude, true);

        }

        const auto totalActiveExplosions{ Dod::BufferUtils::getNumFilledElements(this->activeContext.descs) };
        for (int32_t elId{}; elId < totalActiveExplosions; ++elId)
        {

            const auto targetRadius{ Dod::BufferUtils::get(this->internalContext.maxRadius, elId) };
            const auto expandDelta{ targetRadius / this->configContext.explosionTime * dt };

            auto& desc{ Dod::BufferUtils::get(this->activeContext.descs, elId) };
            desc.radius += expandDelta;

            const auto bNeedRemove{ desc.radius >= targetRadius };
            Dod::BufferUtils::populate(this->tempContext.toRemove, elId, bNeedRemove);

        }

        const auto bNeedRenderExplosions{ totalActiveExplosions > 0 };

        for (int32_t elId{}; elId < totalActiveExplosions; ++elId)
        {

            const auto desc{ Dod::BufferUtils::get(this->activeContext.descs, elId) };

            ProtoRenderer::transform_t transform;
            transform.translate({ desc.position.x, desc.position.y });
            transform.scale({ desc.radius, desc.radius });

            Dod::BufferUtils::populate(this->renderCmdsContext.commands, { transform }, true);

        }

        Dod::BufferUtils::populate(this->renderCmdsContext.batches, { totalActiveExplosions }, bNeedRenderExplosions);
        Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, 20, bNeedRenderExplosions);
        Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, this->internalContext.materialId, bNeedRenderExplosions);

        Dod::BufferUtils::remove(this->activeContext.descs, Dod::BufferUtils::createImFromBuffer(this->tempContext.toRemove));
        Dod::BufferUtils::remove(this->activeContext.magnitudes, Dod::BufferUtils::createImFromBuffer(this->tempContext.toRemove));

    }

}
