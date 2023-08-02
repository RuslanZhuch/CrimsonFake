#include "AssetsExecutor.h"

#include <dod/BufferUtils.h>

#include <internal/supportTypes/renderTypes.h>
#include <filesystem>
#include <engine/StringUtils.h>

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
            
            Types::Render::CreateTextureCmd cmd;
            const auto name{ path.filename().string() };
            Engine::StringUtils::assign(cmd.filename, name.c_str());
            Dod::BufferUtils::populate(this->renderCmdsContext.createTextureCmds, cmd, true);
        }

    }

    void Assets::updateImpl(float dt) noexcept
    {

    }

}
