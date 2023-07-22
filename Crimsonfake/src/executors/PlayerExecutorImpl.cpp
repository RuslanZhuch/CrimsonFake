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
//        std::cout << std::format("dx: {}, dy: {}, angle: {}", deltaXNorm, deltaYNorm, angle) << "\n";

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
        const auto lookAngle{ computeLookAngle(
            this->windowParametersContext.width / 2.f,
            this->windowParametersContext.height / 2.f,
            mouseX,
            mouseY,
            0
        ) };
        cmd.transform.rotate(lookAngle * 180.f / pi);
        Dod::BufferUtils::populate(this->renderCmdsContext.commands, cmd, true);
        
        const auto textureName{ std::string_view(this->initialContext.textureName.data.data()) };
        const auto key{ std::hash<std::string_view>{}(textureName)};
        Dod::BufferUtils::populate(this->renderCmdsContext.materialNames, key, true);

        Dod::BufferUtils::populate(this->renderCmdsContext.depth, 1, true);

        this->renderCmdsContext.cameraX = this->worldStateContext.x;
        this->renderCmdsContext.cameraY = this->worldStateContext.y;

        this->weaponStateContext.fireDelayLeft = std::max(0.f, this->weaponStateContext.fireDelayLeft - dt);

        const auto bNeedShoot{
            Game::Inputs::computeFireComponent(this->inputsContext.inputs, this->inputsContext.prevInputs) > 0 &&
            this->weaponStateContext.fireDelayLeft <= 0.f
        };
//        std::cout << std::format("need shoot {}\n", bNeedShoot);

        Dod::BufferUtils::populate(this->bulletsToCreateContext.angle, lookAngle, bNeedShoot);
        Dod::BufferUtils::populate(this->bulletsToCreateContext.position, Types::Coord::Vec2f(
            this->worldStateContext.x,
            this->worldStateContext.y
        ), bNeedShoot);
        const auto bulletKey{ std::hash<std::string_view>{}("bullet.png")};
        Dod::BufferUtils::populate(this->bulletsToCreateContext.textureNames, bulletKey, bNeedShoot);
        Dod::BufferUtils::populate(this->bulletsToCreateContext.velocity, 1000.f, bNeedShoot);
        Dod::BufferUtils::populate(this->bulletsToCreateContext.timeLeft, 2.f, bNeedShoot);

        this->weaponStateContext.fireDelayLeft += this->weaponStateContext.fireDelay * bNeedShoot;

    }

}
