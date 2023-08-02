#include "RenderExecutor.h"

#include <memory>

#include <dod/BufferUtils.h>
#include <dod/Algorithms.h>

#include <iostream>
#include <format>

namespace Game::ExecutionBlock
{

    void Render::initImpl() noexcept
    {
        this->internalContext.renderer = std::make_unique<ProtoRenderer::Renderer>(
            this->windowParametersContext.width, 
            this->windowParametersContext.height, 
            this->windowParametersContext.title.internalData.data());

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

        const auto createTextureCmds{ Dod::SharedContext::get(this->cmdsContext).createTextureCmds };
        for (int32_t elId{}; elId < Dod::BufferUtils::getNumFilledElements(createTextureCmds); ++elId)
        {
            ProtoRenderer::texture_t newTexture;
            const auto cmd{ Dod::BufferUtils::get(createTextureCmds, elId) };
            const auto bLoaded{ newTexture.loadFromFile("resources/textures/" + std::string(cmd.filename.internalData.data())) };

            Dod::BufferUtils::emplaceBack(this->assetsContext.textures, std::move(newTexture), bLoaded);
            auto texturePtr{ &Dod::BufferUtils::get(this->assetsContext.textures,
                Dod::BufferUtils::getNumFilledElements(this->assetsContext.textures) - 1) };

            const auto name{ std::string_view(cmd.filename.internalData.data()) };
            const auto key{ std::hash<std::string_view>{}(name) };
            Dod::BufferUtils::populate(this->assetsContext.loadedTextureIds, key, bLoaded);
            Dod::BufferUtils::populate(this->assetsContext.loadedTextures, texturePtr, bLoaded);
        }

        this->internalContext.renderer->clear();

        const auto depths{ Dod::SharedContext::get(this->cmdsContext).batchDepth };


        Dod::Algorithms::getSortedIndices(this->internalContext.priority, Dod::BufferUtils::createImFromBuffer(depths));

        const auto commands{ Dod::SharedContext::get(this->cmdsContext).commands };
        const auto batchMaterials{ Dod::SharedContext::get(this->cmdsContext).batchMaterial };
        const auto batches{ Dod::SharedContext::get(this->cmdsContext).batches };
        for (int32_t globalOffset{}, batchElId{}; batchElId < Dod::BufferUtils::getNumFilledElements(batches); ++batchElId)
        {
            Dod::BufferUtils::populate(this->internalContext.globalCmdOffsets, globalOffset, true);
            globalOffset += Dod::BufferUtils::get(batches, batchElId).numOfCmds;
        }

        const auto totalCommands{ Dod::BufferUtils::getNumFilledElements(commands) };

        for (int32_t batchElId{}; batchElId < Dod::BufferUtils::getNumFilledElements(batches); ++batchElId)
        {
            const auto batchId{ Dod::BufferUtils::get(this->internalContext.priority, batchElId) };

            const auto batch{ Dod::BufferUtils::get(batches, batchId) };
            const auto commandsInBatch{ batch.numOfCmds };

            const auto textureId{ getTexture(
                Dod::BufferUtils::createImFromBuffer(this->assetsContext.loadedTextureIds),
                Dod::BufferUtils::get(batchMaterials, batchId)
            ) };
            if (textureId < 0)
                continue;

            auto texturePtr{ Dod::BufferUtils::get(this->assetsContext.loadedTextures, textureId) };
            this->internalContext.renderer->setTexture(texturePtr);
            const auto textureSize{ texturePtr->getSize() };
            this->internalContext.spriteMesh[0].texCoords = { 0.f, 0.f };
            this->internalContext.spriteMesh[1].texCoords = { static_cast<float>(textureSize.x), 0.f };
            this->internalContext.spriteMesh[2].texCoords = { 0.f, static_cast<float>(textureSize.y) };
            this->internalContext.spriteMesh[3].texCoords = { static_cast<float>(textureSize.x), static_cast<float>(textureSize.y) };

            const auto globalCmdOffset{ Dod::BufferUtils::get(this->internalContext.globalCmdOffsets, batchId) };
            for (int32_t cmdElId{}; cmdElId < commandsInBatch; ++cmdElId)
            {
                this->internalContext.renderer->setTransform(Dod::BufferUtils::get(commands, cmdElId + globalCmdOffset).transform);
                this->internalContext.renderer->draw(this->internalContext.spriteMesh);
            }
        }

//        for (int32_t id{}; id < totalCommands; ++id) 
//        {
//            const auto cmdId{ Dod::BufferUtils::get(this->internalContext.priority, id) };
//            const auto textureId{ getTexture(
//                Dod::BufferUtils::createImFromBuffer(materialDataNames),
//                Dod::BufferUtils::get(materialNames, cmdId)
//            ) };
//            if (textureId < 0)
//                continue;
//
//            const auto texture{ &Dod::BufferUtils::get(materialDataTextures, textureId) };
//            this->internalContext.renderer->setTexture(texture);
//            const auto textureSize{ texture->getSize() };
//            this->internalContext.spriteMesh[0].texCoords = { 0.f, 0.f };
//            this->internalContext.spriteMesh[1].texCoords = { static_cast<float>(textureSize.x), 0.f};
//            this->internalContext.spriteMesh[2].texCoords = { 0.f, static_cast<float>(textureSize.y) };
//            this->internalContext.spriteMesh[3].texCoords = { static_cast<float>(textureSize.x), static_cast<float>(textureSize.y) };
//
//            this->internalContext.renderer->setTransform(Dod::BufferUtils::get(commands, cmdId).transform);
//
//            this->internalContext.renderer->draw(this->internalContext.spriteMesh);
//        }

        const auto mouseCoord{ this->internalContext.renderer->getMousePosition() };
        this->mouseContext.x = mouseCoord.x;
        this->mouseContext.y = mouseCoord.y;

        const auto camX{ Dod::SharedContext::get(this->cmdsContext).cameraX };
        const auto camY{ Dod::SharedContext::get(this->cmdsContext).cameraY };

        this->internalContext.renderer->setCameraCoord(camX, camY);

        this->internalContext.renderer->display();

    }

}
