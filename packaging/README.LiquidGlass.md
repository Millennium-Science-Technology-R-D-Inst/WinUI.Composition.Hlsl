# WinUI.LiquidGlass

Native C++/WinRT Liquid Glass controls for WinUI 3. The package depends on the matching `WinUI.Composition.Hlsl` runtime and keeps the control layer separate from the lower-level HLSL/Composition engine.

```xml
<PackageReference Include="WinUI.LiquidGlass" Version="1.0.x" />
```

Controls: `LiquidGlassCard`, `LiquidGlassMagnifier`, `LiquidGlassButton`, `LiquidGlassToggleButton`, `LiquidGlassHyperlinkButton`, `LiquidGlassCheckBox`, `LiquidGlassRadioButton`, `LiquidGlassSlider`, `LiquidGlassTextBox`, `LiquidGlassPasswordBox`, `LiquidGlassComboBox`, and `LiquidGlassToggleSwitch`.

Each control exposes `GlassBrush` as a dependency property. The default value is a per-instance `WinUI.Composition.Hlsl.LiquidGlassBrush`, while applications can replace it from XAML or C++/WinRT:

```xml
<glass:LiquidGlassButton Content="Run">
    <glass:LiquidGlassButton.GlassBrush>
        <hlsl:LiquidGlassBrush
            BlurRadius="2"
            RefractionStrength="24"
            DispersionStrength="0.7"
            CornerRadius="12"
            Contrast="1.05"
            Exposure="0.1" />
    </glass:LiquidGlassButton.GlassBrush>
</glass:LiquidGlassButton>
```

## Reusable optical presets

`LiquidGlassPresets.CreateBrush(...)` creates an independent brush with the library's reference optical profile. Available presets are `Panel`, `Button`, `Choice`, `SearchBox`, `Input`, `SliderThumb`, `ToggleSwitchKnob`, `Magnifier`, `FloatingPanel`, and `TabBar`.

```cpp
auto glass = WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
    WinUI::LiquidGlass::LiquidGlassPreset::TabBar);
glass.TintOpacity(0.16);
glass.Contrast(1.08);
```

The first eight profiles mirror the built-in control baselines. `FloatingPanel` provides the softer media/player-panel treatment and `TabBar` provides a compact bottom-navigation baseline that can be reused by higher-level segmented/tab controls.

## Interaction tuning

`LiquidGlassInteraction` exposes WinUI attached dependency properties rather than baking interaction ratios into every control class. They can be set from XAML, styles, bindings, or C++/WinRT and are shared by the native control presets.

```xml
<glass:LiquidGlassButton
    Content="Elastic action"
    glass:LiquidGlassInteraction.PointerLightingEnabled="True"
    glass:LiquidGlassInteraction.PointerLightInfluence="0.75"
    glass:LiquidGlassInteraction.RestScale="1"
    glass:LiquidGlassInteraction.PointerOverScale="1.04"
    glass:LiquidGlassInteraction.PressedScale="0.95"
    glass:LiquidGlassInteraction.MotionDuration="135"
    glass:LiquidGlassInteraction.Elasticity="0.55"
    glass:LiquidGlassInteraction.PressedRefractionMultiplier="1.3"
    glass:LiquidGlassInteraction.PressedRefractionBoost="2"
    glass:LiquidGlassInteraction.PressedTintBoost="0.06"
    glass:LiquidGlassInteraction.PressedHighlightMultiplier="1.18"
    glass:LiquidGlassInteraction.PressedHighlightBoost="0.04"
    glass:LiquidGlassInteraction.PressedInnerShadowBoost="0.06" />
```

The same settings drive the specialized interactions: Slider copies the press/motion profile to its native Thumb, ToggleSwitch applies it to the glass knob, and Magnifier combines it with `ActiveMagnificationMultiplier` and directional elasticity. `FocusedScale` plus `FocusedTintBoost` drive the focus response of text/password/combo input controls.

The material itself remains independently configurable through `LiquidGlassBrush`: blur/frost, saturation, contrast, exposure, tint RGB/opacity, four surface profiles, bezel geometry, glass thickness, IOR/Snell refraction, magnification, chromatic dispersion, directional lighting, highlight sharpness, specular saturation/width, inner shadow, edge softness, material opacity and fallback color.

Templated controls load `ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml`. Complex input controls intentionally retain the WinUI base templates so keyboard, gamepad, focus, and UI Automation behavior remains platform-native; the glass material is applied to the appropriate native surface at runtime.

The implementation uses C++/WinRT optimized namespace modules. Generated XAML translation units receive the same module-import preamble as the authored control implementation, while the class-named headers required by `module.g.cpp` are thin forwarding headers into the centralized `LiquidGlassControls` module.
