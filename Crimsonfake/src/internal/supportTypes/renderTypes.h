#pragma once

#include <renderer.h>

#include <engine/String.h>
#include <variant>

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

	struct CreateTextureCmd
	{
		Engine::String filename;
	};

	struct CreateRenderTextureCmd
	{
		int32_t width{};
		int32_t height{};
		Engine::String textureName;
	};

}
