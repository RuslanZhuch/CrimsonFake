#include "WorldExecutor.h"

#include <dod/BufferUtils.h>

namespace Game::ExecutionBlock
{

    void World::initImpl() noexcept
    {

    }

    void World::updateImpl(float dt) noexcept
    {
        Types::Render::Cmd cmd;

        const auto textureName{ std::string_view(this->worldContext.textureName.data.data()) };
        const auto textureNamekey{ std::hash<std::string_view>{}(textureName) };

        const auto numOfTiles{ this->worldContext.numOfTiles };
        for (int32_t tileYId{}; tileYId < numOfTiles; ++tileYId)
        {
            const auto yCoord{ tileYId * this->worldContext.size };
            for (int32_t tileXId{}; tileXId < numOfTiles; ++tileXId)
            {
                const auto xCoord{ tileXId * this->worldContext.size };
                cmd.transform = ProtoRenderer::transform_t();
                cmd.transform.translate({ xCoord, yCoord });
                const auto scale{ this->worldContext.size / 2.f };
                cmd.transform.scale({ scale, scale });
                Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
                Dod::BufferUtils::populate(this->renderCmdsContext.materialNames, textureNamekey, true);
                Dod::BufferUtils::populate(this->renderCmdsContext.depth, -10, true);
            }
        }

    }

}
