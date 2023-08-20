#include "WorldExecutor.h"

#include <dod/BufferUtils.h>

namespace Game::ExecutionBlock
{

    void World::initImpl() noexcept
    {

    }

    void World::updateImpl([[maybe_unused]] float dt) noexcept
    {
        Types::Render::Cmd cmd;

        const auto textureName{ std::string_view(this->worldContext.textureName.internalData.data()) };
        const auto textureNamekey{ std::hash<std::string_view>{}(textureName) };

        const auto numOfTiles{ this->worldContext.numOfTiles };
        for (int32_t tileYId{}; tileYId < numOfTiles; ++tileYId)
        {
            const auto yCoord{ static_cast<float>(tileYId * this->worldContext.size) };
            for (int32_t tileXId{}; tileXId < numOfTiles; ++tileXId)
            {
                const auto xCoord{ static_cast<float>(tileXId * this->worldContext.size) };
                cmd.transform = ProtoRenderer::transform_t();
                cmd.transform.translate({ xCoord, yCoord });
                const auto scale{ static_cast<float>(this->worldContext.size / 2) };
                cmd.transform.scale({ scale, scale });
                Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
            }
        }
        Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, textureNamekey, true);
        Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, -10, true);
        Dod::BufferUtils::populate(this->renderCmdsContext.batches, { numOfTiles * numOfTiles }, true);

    }

}
