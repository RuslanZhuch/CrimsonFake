#pragma once

#include <cinttypes>

#include <engine/types.h>

namespace Game::Inputs
{

	struct MoveState
	{

		ENGINE_TYPES_DESERIALIZE;

		float right{};
		float forward{};
	};

	[[nodiscard]] uint32_t gatherInputs() noexcept;

	[[nodiscard]] int32_t computeFireComponent(uint32_t currentInputs, uint32_t prevInputs) noexcept;
	[[nodiscard]] MoveState computeMoveComponent(uint32_t currentInputs, uint32_t prevInputs, MoveState prevMovement) noexcept;
	[[nodiscard]] int32_t computeWeaponSwitchComponent() noexcept;

}
