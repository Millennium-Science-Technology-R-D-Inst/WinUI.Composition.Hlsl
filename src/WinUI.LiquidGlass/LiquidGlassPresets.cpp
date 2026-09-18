#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassPresets.h"

#if __has_include("LiquidGlassPresets.g.cpp")
#include "LiquidGlassPresets.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    WinUI::Composition::Hlsl::LiquidGlassBrush LiquidGlassPresets::CreateBrush(WinUI::LiquidGlass::LiquidGlassPreset preset)
    {
        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;
        using Profile = WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile;

        Brush b;
        b.SurfaceProfile(Profile::ConvexSquircle);
        b.BlurRadius(1.0);
        b.Saturation(1.0);
        b.Contrast(1.0);
        b.Exposure(0.0);
        b.MaterialOpacity(1.0);
        b.EdgeSoftness(1.0);

        switch (preset)
        {
        case LiquidGlassPreset::Panel:
            b.CornerRadius(31); b.RefractionStrength(24); b.DispersionStrength(.45);
            b.BezelWidth(29); b.GlassThickness(90); b.RefractiveIndex(1.3);
            b.HighlightStrength(.4); b.HighlightSharpness(1.6); b.SpecularSaturation(6);
            b.SpecularWidth(1); b.TintOpacity(.12); b.InnerShadowStrength(.05);
            b.FallbackColor({ 0x55, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::Button:
        case LiquidGlassPreset::Choice:
            b.CornerRadius(preset == LiquidGlassPreset::Choice ? 10 : 8);
            b.RefractionStrength(18); b.DispersionStrength(.55);
            b.BezelWidth(preset == LiquidGlassPreset::Choice ? 9 : 12);
            b.GlassThickness(preset == LiquidGlassPreset::Choice ? 32 : 48);
            b.RefractiveIndex(1.45); b.HighlightStrength(.4); b.HighlightSharpness(1.8);
            b.SpecularSaturation(5); b.SpecularWidth(1); b.TintOpacity(.12);
            b.InnerShadowStrength(.06); b.FallbackColor({ 0x44, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::SearchBox:
            b.CornerRadius(28); b.RefractionStrength(16.8); b.DispersionStrength(.35);
            b.BezelWidth(27); b.GlassThickness(70); b.RefractiveIndex(1.5);
            b.HighlightStrength(.2); b.HighlightSharpness(1.7); b.SpecularSaturation(4);
            b.SpecularWidth(1); b.TintOpacity(.05); b.InnerShadowStrength(.04);
            b.FallbackColor({ 0x30, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::Input:
            b.CornerRadius(8); b.RefractionStrength(16); b.DispersionStrength(.4);
            b.BezelWidth(14); b.GlassThickness(60); b.RefractiveIndex(1.45);
            b.HighlightStrength(.3); b.HighlightSharpness(1.8); b.SpecularSaturation(4);
            b.SpecularWidth(1); b.TintOpacity(.10); b.InnerShadowStrength(.05);
            b.FallbackColor({ 0x36, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::SliderThumb:
            // Match Kube Slider.tsx / virtual:refractionFilter exactly:
            // width=90, height=60, radius=30, bezelWidth=16,
            // glassThickness=80, refractiveIndex=1.45, blur=0,
            // specularOpacity=.4 and specularSaturation=7.
            b.CornerRadius(30); b.BlurRadius(0);
            // LiquidGlass.hlsl maps RefractionStrength / 24 to Kube's displacement
            // scaleRatio, so 9.6 is rest .4 and pressed 2.25x reaches .9.
            b.RefractionStrength(9.6);
            b.BezelWidth(16); b.GlassThickness(80); b.RefractiveIndex(1.45);

            // Kube's slider displacement map has no separate chromatic-dispersion
            // pass and no generic border tint. Those two defaults are mostly hidden
            // by the opaque white rest body, but become the dominant "wrong material"
            // look after backgroundOpacity falls to .1.
            b.DispersionStrength(0.0);
            b.BorderThickness(0.0);

            // specular.ts uses abs(dot(normal, light)) directly (power 1) and a
            // one-pixel edge profile.
            b.HighlightStrength(.4); b.HighlightSharpness(1.0);
            b.SpecularSaturation(7); b.SpecularWidth(1);

            // Kube paints the white body as CSS background after backdrop-filter and
            // specular composition. Keep the shader itself untinted; the slider visual
            // model adds a separate white overlay at opacity 1.0 -> 0.1.
            // specular.ts defaults to +pi/3 in screen coordinates; our SDF normal Y axis
            // is inverted relative to that raster map, so the matching shader angle is -pi/3.
            b.TintOpacity(0.0); b.InnerShadowStrength(0.0);
            b.LightAngle(-1.0471975512);
            b.FallbackColor({ 0xff, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::ToggleSwitchKnob:
            b.SurfaceProfile(Profile::Lip); b.CornerRadius(46); b.BlurRadius(.2);
            // Kube's lip bezel uses the same 0.4 -> 0.9 refraction scale as the slider.
            b.RefractionStrength(9.6); b.DispersionStrength(.70); b.BezelWidth(19);
            b.GlassThickness(47); b.RefractiveIndex(1.5); b.HighlightStrength(.5);
            b.HighlightSharpness(1.6); b.SpecularSaturation(6); b.SpecularWidth(1);
            // Kube's white body is fully opaque at rest and fades to 0.1 while active.
            // Kube adds its inset black/white pair only while the switch is pressed.
            b.TintOpacity(1.0); b.InnerShadowStrength(0.0);
            b.FallbackColor({ 0xff, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::Magnifier:
            b.CornerRadius(75); b.BlurRadius(0); b.RefractionStrength(19.2);
            b.DispersionStrength(.55); b.BezelWidth(25); b.GlassThickness(110);
            b.RefractiveIndex(1.5); b.MagnificationStrength(24); b.HighlightStrength(.5);
            b.HighlightSharpness(1.6); b.SpecularSaturation(9); b.SpecularWidth(1);
            b.TintOpacity(.02); b.InnerShadowStrength(.20);
            b.FallbackColor({ 0x28, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::FloatingPanel:
            b.CornerRadius(28); b.BlurRadius(8); b.RefractionStrength(18);
            b.DispersionStrength(.35); b.BezelWidth(24); b.GlassThickness(80);
            b.RefractiveIndex(1.38); b.HighlightStrength(.32); b.HighlightSharpness(1.8);
            b.SpecularSaturation(5); b.SpecularWidth(1); b.TintOpacity(.14);
            b.InnerShadowStrength(.05); b.FallbackColor({ 0x58, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::TabBar:
            b.CornerRadius(30); b.BlurRadius(3); b.RefractionStrength(20);
            b.DispersionStrength(.45); b.BezelWidth(26); b.GlassThickness(82);
            b.RefractiveIndex(1.42); b.HighlightStrength(.38); b.HighlightSharpness(1.7);
            b.SpecularSaturation(6); b.SpecularWidth(1); b.TintOpacity(.12);
            b.InnerShadowStrength(.05); b.FallbackColor({ 0x50, 0xff, 0xff, 0xff }); break;
        case LiquidGlassPreset::TabBarItem:
            b.CornerRadius(22); b.BlurRadius(1); b.RefractionStrength(16);
            b.DispersionStrength(.4); b.BezelWidth(14); b.GlassThickness(52);
            b.RefractiveIndex(1.42); b.HighlightStrength(.38); b.HighlightSharpness(1.8);
            b.SpecularSaturation(5); b.SpecularWidth(1); b.TintOpacity(.08);
            b.InnerShadowStrength(.05); b.FallbackColor({ 0x42, 0xff, 0xff, 0xff }); break;
        default:
            throw hresult_invalid_argument(L"Unknown LiquidGlassPreset value.");
        }

        return b;
    }
}
