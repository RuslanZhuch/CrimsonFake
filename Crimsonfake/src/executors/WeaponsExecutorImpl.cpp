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

        if (Dod::DataUtils::getNumFilledElements(this->configContext.descriptions) > 0)
        {
            this->weaponStateContext.currentDesc = Dod::DataUtils::get(this->configContext.descriptions, 0);
            this->weaponStateContext.shotsLeft = this->weaponStateContext.currentDesc.totalShots;
        }

    }

    void Weapons::updateImpl([[maybe_unused]] float dt) noexcept
    {

        this->weaponStateContext.fireDelayLeft = std::max(0.f, this->weaponStateContext.fireDelayLeft - dt);
        const auto bCanShoot{
            this->weaponStateContext.fireDelayLeft <= 0.f
        };

        const auto commands{ Dod::SharedContext::get(this->commandsContext).commands };
        for (int32_t commandId{}; commandId < Dod::DataUtils::getNumFilledElements(commands); ++commandId)
        {

            const auto cmd{ Dod::DataUtils::get(commands, commandId) };

            const auto numOfBullets{ weaponStateContext.currentDesc.bulletsPerShot };
            const auto initialAngle{ cmd.angle };

            for (int32_t bulletId{}; bulletId < numOfBullets * bCanShoot; ++bulletId)
            {
                const auto spreadInGrad{ static_cast<int32_t>(this->weaponStateContext.currentDesc.spread) };
                const auto randSpread{ static_cast<float>(rand() % (spreadInGrad + 1)) * pi / 180.f };
                const auto spreadAngle{ initialAngle + randSpread - randSpread / 2.f };

                const auto bulletKey{ std::hash<std::string_view>{}(this->weaponStateContext.currentDesc.bulletTextureName.internalData.data()) };
                Game::Context::Bullets::addStates(this->bulletsToCreateContext,
                    bulletKey,
                    this->weaponStateContext.currentDesc.bulletVelocity,
                    Types::Coord::Vec2f(cmd.spawnCoord.x, cmd.spawnCoord.y),
                    spreadAngle,
                    this->weaponStateContext.currentDesc.bulletLifeTime,
                    this->weaponStateContext.currentDesc.type
                );
            }
            this->weaponStateContext.fireDelayLeft += this->weaponStateContext.currentDesc.fireDelay * bCanShoot;
            this->weaponStateContext.shotsLeft -= bCanShoot;

        }

        if (this->weaponStateContext.shotsLeft <= 0)
        {
            this->weaponStateContext.currentDesc = Dod::DataUtils::get(this->configContext.descriptions, 0);
            this->weaponStateContext.shotsLeft = this->weaponStateContext.currentDesc.totalShots;
        }

        const auto switchCmds{ Dod::SharedContext::get(this->commandsContext).setWeaponType };
        for (int32_t commandId{}; commandId < Dod::DataUtils::getNumFilledElements(switchCmds); ++commandId)
        {
            const auto switchType{ Dod::DataUtils::get(switchCmds, commandId) };
            for (int32_t descId{}; descId < Dod::DataUtils::getNumFilledElements(this->configContext.descriptions); ++descId)
            {
                const auto type{ Dod::DataUtils::get(this->configContext.descriptions, descId).type };
                if (switchType != type)
                    continue;
                this->weaponStateContext.currentDesc = Dod::DataUtils::get(this->configContext.descriptions, descId);
                this->weaponStateContext.shotsLeft = this->weaponStateContext.currentDesc.totalShots;
                break;
            }
            this->weaponStateContext.fireDelayLeft = 0.f;
        }

    }

}
