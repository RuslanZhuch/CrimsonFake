#include "WeaponsExecutor.h"

#include <dod/BufferUtils.h>

#include <engine/StringUtils.h>
#include <engine/contextUtils.h>
#include <string_view>
#include <format>
#include <numbers>

static constexpr auto pi{ static_cast<float>(std::numbers::pi) };

namespace Game::ExecutionBlock
{

    void Weapons::initImpl() noexcept
    {

        if (Dod::BufferUtils::getNumFilledElements(this->configContext.descriptions) > 0)
            this->weaponStateContext.currentDesc = Dod::BufferUtils::get(this->configContext.descriptions, 0);

    }

    void Weapons::updateImpl([[maybe_unused]] float dt) noexcept
    {

        this->weaponStateContext.fireDelayLeft = std::max(0.f, this->weaponStateContext.fireDelayLeft - dt);
        const auto bCanShoot{
            this->weaponStateContext.fireDelayLeft <= 0.f
        };

        const auto commands{ Dod::SharedContext::get(this->commandsContext).commands };
        for (int32_t commandId{}; commandId < Dod::BufferUtils::getNumFilledElements(commands); ++commandId)
        {

            const auto cmd{ Dod::BufferUtils::get(commands, commandId) };

            const auto numOfBullets{ weaponStateContext.currentDesc.bulletsPerShot };
            const auto initialAngle{ cmd.angle };

            for (int32_t bulletId{}; bulletId < numOfBullets * bCanShoot; ++bulletId)
            {
                const auto spreadInGrad{ static_cast<int32_t>(this->weaponStateContext.currentDesc.spread) };
                const auto randSpread{ static_cast<float>(rand() % (spreadInGrad + 1)) * pi / 180.f };
                const auto spreadAngle{ initialAngle + randSpread - randSpread / 2.f };

                Dod::BufferUtils::populate(this->bulletsToCreateContext.angle, spreadAngle, true);
                Dod::BufferUtils::populate(this->bulletsToCreateContext.position, Types::Coord::Vec2f(
                    cmd.spawnCoord.x,
                    cmd.spawnCoord.y
                ), true);
                const auto bulletKey{ std::hash<std::string_view>{}(this->weaponStateContext.currentDesc.bulletTextureName.internalData.data()) };
                Dod::BufferUtils::populate(this->bulletsToCreateContext.textureNames, bulletKey, true);
                Dod::BufferUtils::populate(this->bulletsToCreateContext.velocity, this->weaponStateContext.currentDesc.bulletVelocity, true);
                Dod::BufferUtils::populate(this->bulletsToCreateContext.timeLeft, this->weaponStateContext.currentDesc.bulletLifeTime, true);
            }
            this->weaponStateContext.fireDelayLeft += this->weaponStateContext.currentDesc.fireDelay * bCanShoot;

        }

        const auto switchCmds{ Dod::SharedContext::get(this->commandsContext).setWeaponType };
        for (int32_t commandId{}; commandId < Dod::BufferUtils::getNumFilledElements(switchCmds); ++commandId)
        {
            const auto switchType{ Dod::BufferUtils::get(switchCmds, commandId) };
            for (int32_t descId{}; descId < Dod::BufferUtils::getNumFilledElements(this->configContext.types); ++descId)
            {
                const auto type{ Dod::BufferUtils::get(this->configContext.types, descId) };
                if (switchType != type)
                    continue;
                this->weaponStateContext.currentDesc = Dod::BufferUtils::get(this->configContext.descriptions, descId);
                break;
            }
            this->weaponStateContext.fireDelayLeft = 0.f;
        }

    }

}
