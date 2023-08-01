#pragma once

#include <renderer.h>

namespace Types::Render
{
	struct Cmd
	{
		ProtoRenderer::transform_t transform;
	};

	struct BatchDesc
	{
		int32_t numOfCmds{};
	};
}
