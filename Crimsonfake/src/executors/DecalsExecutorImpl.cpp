#include "DecalsExecutor.h"

#include <dod/BufferUtils.h>
#include <engine/StringUtils.h>
#include <dod/Algorithms.h>

#include <format>

namespace Game::ExecutionBlock
{

    void Decals::initImpl() noexcept
    {

        Types::Render::CreateRenderTextureCmd cmd;
        cmd.width = this->worldContext.size;
        cmd.height = this->worldContext.size;
        
        const auto needTextures{ this->worldContext.numOfTiles * this->worldContext.numOfTiles };
        for (int32_t tileId{}; tileId < needTextures; ++tileId)
        {
            char name[64]{};
            std::format_to_n(name, sizeof(name), "Tile{}Decals", tileId + 1);
            Engine::StringUtils::assign(cmd.textureName, name);
            Dod::BufferUtils::populate(this->renderCmdsContext.createRenderTextureCmd, cmd, true);
        }

    }

    void Decals::updateImpl([[maybe_unused]] float dt) noexcept
    {

        const auto cmds{ Dod::SharedContext::get(this->commandsContext).commands };
        const auto textures{ Dod::SharedContext::get(this->commandsContext).texture };
        Dod::Algorithms::getSortedIndices(this->internalContext.sortedIndices, Dod::BufferUtils::createImFromBuffer(textures));
        const auto sortedMaterials{ Dod::BufferUtils::createSortedImBuffer(
            Dod::BufferUtils::createImFromBuffer(textures),
            Dod::BufferUtils::createImFromBuffer(this->internalContext.sortedIndices)
        ) };
        Dod::Algorithms::countUniques(this->internalContext.elementsPerBatch, sortedMaterials);

        const auto tileSize{ this->worldContext.size };
        const auto needTextures{ this->worldContext.numOfTiles * this->worldContext.numOfTiles };

        for (int32_t tileId{}; tileId < needTextures; ++tileId)
        {

            char name[64]{};
            std::format_to_n(name, sizeof(name), "Tile{}Decals", tileId + 1);
            const auto rtkey{ std::hash<std::string_view>{}(name) };
            Dod::BufferUtils::constructBack(this->renderCmdsContext.rtBatchTarget, rtkey, true);

            for (int32_t elId{}, batchId{}; elId < Dod::BufferUtils::getNumFilledElements(this->internalContext.sortedIndices); ++batchId)
            {

                const auto material{ Dod::BufferUtils::get(sortedMaterials, elId) };

                Dod::BufferUtils::constructBack(this->renderCmdsContext.rtbatchDepth, 1, true);
                Dod::BufferUtils::constructBack(this->renderCmdsContext.rtbatchMaterial, material, true);

                const auto numOfElementsPerBatch{ Dod::BufferUtils::get(this->internalContext.elementsPerBatch, batchId) };
                Dod::BufferUtils::constructBack(this->renderCmdsContext.rtbatches, Types::Render::BatchDesc(numOfElementsPerBatch), true);

                for (int32_t localElId{ elId }; localElId < numOfElementsPerBatch; ++localElId)
                {
                    const auto cmdId{ Dod::BufferUtils::get(this->internalContext.sortedIndices, localElId) };
                    const auto cmd{ Dod::BufferUtils::get(cmds, cmdId) };

                    ProtoRenderer::transform_t transform1;

                    const auto offsetX{ static_cast<float>((tileId % this->worldContext.numOfTiles) * tileSize) };
                    const auto offsetY{ static_cast<float>((tileId / this->worldContext.numOfTiles) * tileSize) };

                    transform1.translate({ cmd.position.x - offsetX, cmd.position.y - offsetY });
                    transform1.rotate(cmd.angle);
                    transform1.scale({ cmd.scale, cmd.scale });
                    Dod::BufferUtils::constructBack(this->renderCmdsContext.rtCommands, Types::Render::Cmd(transform1), true);

                }

                elId += numOfElementsPerBatch;

            }

        }

        for (int32_t tileId{}; tileId < needTextures; ++tileId)
        {

            ProtoRenderer::transform_t transform2;
            const auto offsetX{ static_cast<float>((tileId % this->worldContext.numOfTiles) * tileSize) };
            const auto offsetY{ static_cast<float>((tileId / this->worldContext.numOfTiles) * tileSize) };
            transform2.translate({ offsetX, offsetY });
            const auto scale{ static_cast<float>(this->worldContext.size / 2) };
            transform2.scale({ scale, scale });
            Dod::BufferUtils::populate(this->renderCmdsContext.commands, Types::Render::Cmd(transform2), true);

            char name[64]{};
            std::format_to_n(name, sizeof(name), "Tile{}Decals", tileId + 1);
            const auto rtkey{ std::hash<std::string_view>{}(name) };
            Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, rtkey, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, 0, true);
            Dod::BufferUtils::populate(this->renderCmdsContext.batches, { 1 }, true);

        }

    }

}
