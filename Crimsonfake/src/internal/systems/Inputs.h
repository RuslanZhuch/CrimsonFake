#pragma once

#include <cinttypes>

namespace Game::Inputs
{

	struct MoveState
	{
		float right{};
		float forward{};
	};

	[[nodiscard]] uint32_t gatherInputs() noexcept;

	[[nodiscard]] int32_t computeFireComponent(uint32_t currentInputs, uint32_t prevInputs) noexcept;
	[[nodiscard]] MoveState computeMoveComponent(uint32_t currentInputs, uint32_t prevInputs, MoveState prevMovement) noexcept;

}
