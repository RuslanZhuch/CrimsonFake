#include "RenderExecutor.h"

#include <memory>

#include <dod/BufferUtils.h>

namespace Game::ExecutionBlock
{

    void Render::initImpl() noexcept
    {
        this->internalContext.renderer = std::make_unique<ProtoRenderer::Renderer>(
            this->windowParametersContext.width, 
            this->windowParametersContext.height, 
            this->windowParametersContext.title.data.data());

        constexpr auto textureSize{ 64.f };

        this->internalContext.spriteMesh.setPrimitiveType(sf::TriangleStrip);
        this->internalContext.spriteMesh.append(sf::Vertex({ -1.f, -1.f }, { 0, 0 }));
        this->internalContext.spriteMesh.append(sf::Vertex({ 1.f, -1.f }, { textureSize, 0 }));
        this->internalContext.spriteMesh.append(sf::Vertex({ -1.f, 1.f }, { 0, textureSize }));
        this->internalContext.spriteMesh.append(sf::Vertex({ 1.f, 1.f }, { textureSize, textureSize }));
    }

    [[nodisard]] auto getTexture(
        Dod::ImBuffer<uint64_t> names,
        uint64_t nameToFind
    )
    {
        for (int32_t id{}; id < Dod::BufferUtils::getNumFilledElements(names); ++id)
        {
            const auto currName{ Dod::BufferUtils::get(names, id) };
            if (nameToFind == currName)
                return id;
        }
        return -1;
    }

    void Render::updateImpl(float dt) noexcept
    {

        ProtoRenderer::event_t event;

        while (this->internalContext.renderer->pollEvent(event))
        {
            Dod::BufferUtils::populate(this->applicationContext.commands, 1, event.type == sf::Event::Closed);
        }

        this->internalContext.renderer->clear();

        const auto commands{ Dod::SharedContext::get(this->cmdsContext).commands };
        const auto materialNames{ Dod::SharedContext::get(this->cmdsContext).materialNames };

        const auto totalCommands{ std::min(
            Dod::BufferUtils::getNumFilledElements(commands),
            Dod::BufferUtils::getNumFilledElements(materialNames)
        ) };

        const auto materialDataNames{ Dod::SharedContext::get(this->materialsContext).names };
        const auto materialDataTextures{ Dod::SharedContext::get(this->materialsContext).textures };
        for (int32_t cmdId{}; cmdId < totalCommands; ++cmdId) 
        {
            const auto textureId{ getTexture(
                Dod::BufferUtils::createImFromBuffer(materialNames),
                Dod::BufferUtils::get(materialDataNames, cmdId)
            ) };
            if (textureId < 0)
                continue;

            this->internalContext.renderer->setTexture(&Dod::BufferUtils::get(materialDataTextures, textureId));
            this->internalContext.renderer->setTransform(Dod::BufferUtils::get(commands, cmdId).transform);

            this->internalContext.renderer->draw(this->internalContext.spriteMesh);
        }

        const auto mouseCoord{ this->internalContext.renderer->getMousePosition() };
        this->mouseContext.x = mouseCoord.x;
        this->mouseContext.y = mouseCoord.y;

        this->internalContext.renderer->display();

    }

}
