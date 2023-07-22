#include "AssetsExecutor.h"

#include <dod/BufferUtils.h>

#include <filesystem>

namespace Game::ExecutionBlock
{

    void Assets::initImpl() noexcept
    {

        for (auto const& entry : std::filesystem::directory_iterator{ "resources/textures" })
        {
            const auto path{ entry.path() };
            if (path.extension() != ".png")
                continue;
            
            const auto pathName{ path.string()};
                
            ProtoRenderer::texture_t newTexture;
            const auto bLoaded{ newTexture.loadFromFile(pathName) };
            Dod::BufferUtils::constructBack(this->materialsContext.textures, newTexture, bLoaded);

            const auto name{ path.filename().string() };
            const auto key{ std::hash<std::string>{}(name) };
            Dod::BufferUtils::populate(this->materialsContext.names, key, bLoaded);
        }

    }

    void Assets::updateImpl(float dt) noexcept
    {

    }

}
