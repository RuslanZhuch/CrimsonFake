#include "DecalsExecutor.h"

#include <dod/BufferUtils.h>

namespace Game::ExecutionBlock
{

    void Decals::initImpl() noexcept
    {
//        const auto needTextures{ this->worldContext.numOfTiles * this->worldContext.numOfTiles };
//        for (int32_t tileId{}; tileId < needTextures; ++tileId)
//        {
//            ProtoRenderer::targetTexture_t newTexture;
//            const auto bCreated{ newTexture.create(this->worldContext.size, this->worldContext.size) };
//            Dod::BufferUtils::constructBack(this->internalContext.targetTextures, newTexture, bCreated);
//            Dod::BufferUtils::constructBack(this->materialsContext.textures, newTexture.getTexture(), bCreated);
//
//            const auto key{ std::hash<int>{}(42 + tileId) };
//            Dod::BufferUtils::populate(this->materialsContext.names, key, bCreated);
//        }
    }

    void Decals::updateImpl(float dt) noexcept
    {

    }

}
