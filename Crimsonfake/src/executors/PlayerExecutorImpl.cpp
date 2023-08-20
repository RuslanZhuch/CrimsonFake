#include "PlayerExecutor.h"

#include <internal/systems/Inputs.h>

#include <dod/BufferUtils.h>

#include <iostream>
#include <format>
#include <numbers>

static constexpr auto pi{ static_cast<float>(std::numbers::pi) };

namespace Game::ExecutionBlock
{

    [[nodiscard]] static auto computeLookAngle(
        float x, 
        float y, 
        float mouseX, 
        float mouseY, 
        float prevAngle
    ) noexcept
    {

        const auto deltaX{ mouseX - x };
        const auto deltaY{ mouseY - y };

        const auto deltaLen{ std::sqrtf(
            deltaX * deltaX +
            deltaY * deltaY
        ) };


        if (deltaLen <= 0.01f)
            return prevAngle;

        const auto deltaXNorm{ deltaX / deltaLen };
        const auto deltaYNorm{ deltaY / deltaLen };

        const auto angle{ atanf(deltaYNorm / deltaXNorm) + pi / 2.f + pi * (deltaXNorm < 0) };

        return angle;

    }

    void Player::initImpl() noexcept
    {
        this->worldStateContext.x = this->initialContext.x;
        this->worldStateContext.y = this->initialContext.y;
    }

    void Player::updateImpl([[maybe_unused]] float dt) noexcept
    {
        this->inputsContext.prevInputs = this->inputsContext.inputs;
        this->inputsContext.inputs = Game::Inputs::gatherInputs();

        this->inputsContext.moveState = Game::Inputs::computeMoveComponent(
            this->inputsContext.inputs,
            this->inputsContext.prevInputs,
            this->inputsContext.moveState
        );

        const auto moveDist{ std::sqrtf(
            this->inputsContext.moveState.right * this->inputsContext.moveState.right + 
            this->inputsContext.moveState.forward * this->inputsContext.moveState.forward 
        ) };

        if (moveDist != 0.f) 
        {
            const auto moveXNorm{ this->inputsContext.moveState.right / moveDist };
            const auto moveYNorm{ this->inputsContext.moveState.forward / moveDist };

            this->worldStateContext.x += moveXNorm * dt * this->initialContext.velocity;
            this->worldStateContext.y -= moveYNorm * dt * this->initialContext.velocity;
        }

        const auto mouseX{ static_cast<float>(Dod::SharedContext::get(this->mouseContext).x) };
        const auto mouseY{ static_cast<float>(Dod::SharedContext::get(this->mouseContext).y) };

        Types::Render::Cmd cmd;
        cmd.transform.translate({ this->worldStateContext.x, this->worldStateContext.y });
        cmd.transform.scale({ this->initialContext.size, this->initialContext.size });
        const auto lookAngle{ computeLookAngle(
            static_cast<float>(this->windowParametersContext.width / 2),
            static_cast<float>(this->windowParametersContext.height / 2),
            mouseX,
            mouseY,
            0
        ) };
        cmd.transform.rotate(lookAngle * 180.f / pi);
        Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
        
        const auto textureName{ std::string_view(this->initialContext.textureName.internalData.data()) };
        const auto key{ std::hash<std::string_view>{}(textureName)};
        Dod::BufferUtils::populate(this->renderCmdsContext.batchMaterial, key, true);
        Dod::BufferUtils::populate(this->renderCmdsContext.batchDepth, 1, true);
        Dod::BufferUtils::populate(this->renderCmdsContext.batches, { 1 }, true);

        this->renderCmdsContext.cameraX = this->worldStateContext.x;
        this->renderCmdsContext.cameraY = this->worldStateContext.y;

        const auto bNeedShoot{
            Game::Inputs::computeFireComponent(this->inputsContext.inputs, this->inputsContext.prevInputs) > 0
        };

        Game::Weapons::FireCmd fireCmd;
        fireCmd.spawnCoord = { this->worldStateContext.x, this->worldStateContext.y };
        fireCmd.angle = lookAngle;
        Dod::BufferUtils::populate(this->weaponCmdsContext.commands, fireCmd, bNeedShoot);

        const auto switchType{ Game::Inputs::computeWeaponSwitchComponent() };
        const auto bNeedSwitch{ switchType >= 0 };
        Dod::BufferUtils::populate(this->weaponCmdsContext.setWeaponType, switchType, bNeedSwitch);

    }

}
