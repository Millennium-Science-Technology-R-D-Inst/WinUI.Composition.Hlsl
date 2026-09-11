module;

#include <Windows.h>
#include "LiquidGlassShader.g.h"

export module WinUI.Composition.Hlsl.Shaders.LiquidGlass;

import std;

export namespace WinUI::Composition::Hlsl::Shaders
{
	struct ShaderLibraryView
	{
		void const* data{};
		std::size_t size{};
	};

	ShaderLibraryView LiquidGlassShader() noexcept
	{
		return { g_LiquidGlassShader, sizeof(g_LiquidGlassShader) };
	}
}
