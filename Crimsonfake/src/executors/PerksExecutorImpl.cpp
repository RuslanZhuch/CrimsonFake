#include "PerksExecutor.h"

#include <dod/BufferUtils.h>

namespace Game::ExecutionBlock
{

    void Perks::initImpl() noexcept
    {

    }

    void Perks::updateImpl([[maybe_unused]] float dt) noexcept
    {
        
        const auto commands{ Dod::SharedContext::get(this->commandsContext).perksToActivate };

        for (int32_t elId{}; elId < Dod::DataUtils::getNumFilledElements(commands); ++elId)
        {

            const auto activateCmd{ Dod::DataUtils::get(commands, elId) };

            const auto perkType{ activateCmd.type };

            if (perkType == 1)
            {

                Game::Explosions::Cmd cmd;
                cmd.desc.position = activateCmd.coord;
                cmd.desc.radius = 128;
                cmd.magnitude = 1.f;

                Dod::DataUtils::populate(this->explosionsCmdsContext.spawn, cmd, true);

            }
            else if (perkType == 2)
            {

                Dod::DataUtils::populate(this->weaponCmdsContext.setWeaponType, 2, true);

            }
            else if (perkType == 3)
            {

                Dod::DataUtils::populate(this->weaponCmdsContext.setWeaponType, 3, true);

            }

        }

    }

}
