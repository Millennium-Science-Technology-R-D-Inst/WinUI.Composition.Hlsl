#pragma once
import WinUI.Composition.Hlsl.EffectDef;
import winrt.Windows.Graphics.Effects;
import std;

namespace CustomLiquidGlassPostProcessingEffect
{
    inline constexpr wchar_t EffectName[] = L"LiquidGlassPostProcessingEffect";
    std::shared_ptr<hlsl::engine::EffectDefinition const> Description();
    winrt::Windows::Graphics::Effects::IGraphicsEffect CreateEffect();
}
