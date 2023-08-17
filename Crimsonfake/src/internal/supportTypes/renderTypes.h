#pragma once

#include <renderer.h>

#include <engine/String.h>
#include <variant>

#include <engine/types.h>

namespace Types::Render
{

	struct Cmd
	{

		ENGINE_TYPES_DESERIALIZE;

		ProtoRenderer::transform_t transform;
	};

	struct BatchDesc
	{

		ENGINE_TYPES_DESERIALIZE;

		int32_t numOfCmds{};
	};

	struct CreateTextureCmd
	{

		ENGINE_TYPES_DESERIALIZE;

		Engine::String filename;
	};

	struct CreateRenderTextureCmd
	{

		ENGINE_TYPES_DESERIALIZE;

		int32_t width{};
		int32_t height{};
		Engine::String textureName;
	};

}
