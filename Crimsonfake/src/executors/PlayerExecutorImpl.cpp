#include "PlayerExecutor.h"

#include <internal/systems/Inputs.h>

#include <dod/BufferUtils.h>

#include <iostream>
#include <format>

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

        const auto angle{ atanf(deltaYNorm / deltaXNorm) * 180.f / 3.1416f + 90.f + 180.f * (deltaXNorm < 0) };
        std::cout << std::format("dx: {}, dy: {}, angle: {}", deltaXNorm, deltaYNorm, angle) << "\n";

        return angle;

    }

    void Player::initImpl() noexcept
    {
        this->worldStateContext.x = this->initialContext.x;
        this->worldStateContext.y = this->initialContext.y;
    }

    void Player::updateImpl(float dt) noexcept
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
        cmd.transform.rotate(computeLookAngle(
            this->worldStateContext.x, 
            this->worldStateContext.y, 
            mouseX,
            mouseY,
            0
        ));
        Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
        
        const auto textureName{ std::string_view(this->initialContext.textureName.data.data()) };
        const auto key{ std::hash<std::string_view>{}(textureName)};
        Dod::BufferUtils::populate(this->renderCmdsContext.materialNames, key, true);
    }

}
