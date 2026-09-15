# WinUI.LiquidGlass

Native C++/WinRT Liquid Glass controls for WinUI 3. The package depends on the matching `WinUI.Composition.Hlsl` runtime and keeps the control layer separate from the lower-level HLSL/Composition engine.

```xml
<PackageReference Include="WinUI.LiquidGlass" Version="1.0.x" />
```

Controls: `LiquidGlassCard`, `LiquidGlassFloatingPanel`, `LiquidGlassMagnifier`, `LiquidGlassButton`, `LiquidGlassToggleButton`, `LiquidGlassHyperlinkButton`, `LiquidGlassCheckBox`, `LiquidGlassRadioButton`, `LiquidGlassSlider`, `LiquidGlassTextBox`, `LiquidGlassPasswordBox`, `LiquidGlassSearchBox`, `LiquidGlassComboBox`, `LiquidGlassToggleSwitch`, `LiquidGlassTabBar`, and `LiquidGlassTabBarItem`.

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

`LiquidGlassPresets.CreateBrush(...)` creates an independent brush with the library's reference optical profile. Available presets are `Panel`, `Button`, `Choice`, `SearchBox`, `Input`, `SliderThumb`, `ToggleSwitchKnob`, `Magnifier`, `FloatingPanel`, `TabBar`, and `TabBarItem`.

```cpp
auto glass = WinUI::LiquidGlass::LiquidGlassPresets::CreateBrush(
    WinUI::LiquidGlass::LiquidGlassPreset::TabBar);
glass.TintOpacity(0.16);
glass.Contrast(1.08);
```

The core profiles mirror the built-in control baselines. `FloatingPanel` provides the softer media/player-panel treatment; `TabBar` provides the outer navigation surface; `TabBarItem` is a smaller independent selection pill.

## Interaction tuning

`LiquidGlassInteraction` exposes WinUI attached dependency properties rather than baking interaction ratios into every control class. They can be set from XAML, styles, bindings, or C++/WinRT and are shared by the native control presets. Values use local-value-first lookup and then walk the visual tree, so a parent `LiquidGlassSlider` or `LiquidGlassTabBar` can configure generated/internal containers without copying every property.

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
    glass:LiquidGlassInteraction.PointerDisplacement="3"
    glass:LiquidGlassInteraction.PressedDisplacementMultiplier="1.5"
    glass:LiquidGlassInteraction.PointerOverRefractionMultiplier="1.08"
    glass:LiquidGlassInteraction.PointerOverDispersionMultiplier="1.05"
    glass:LiquidGlassInteraction.PointerOverTintBoost="0.04"
    glass:LiquidGlassInteraction.PointerOverHighlightMultiplier="1.10"
    glass:LiquidGlassInteraction.PressedBlurBoost="1"
    glass:LiquidGlassInteraction.PressedRefractionMultiplier="1.3"
    glass:LiquidGlassInteraction.PressedRefractionBoost="2"
    glass:LiquidGlassInteraction.PressedDispersionMultiplier="1.15"
    glass:LiquidGlassInteraction.PressedSaturationMultiplier="1.05"
    glass:LiquidGlassInteraction.PressedContrastMultiplier="1.06"
    glass:LiquidGlassInteraction.PressedExposureBoost="0.08"
    glass:LiquidGlassInteraction.PressedTintBoost="0.06"
    glass:LiquidGlassInteraction.PressedHighlightMultiplier="1.18"
    glass:LiquidGlassInteraction.PressedHighlightBoost="0.04"
    glass:LiquidGlassInteraction.PressedInnerShadowBoost="0.06" />
```

Pointer-over, press and focus optical layers snapshot and restore blur, refraction, dispersion, saturation, contrast, exposure, tint opacity, highlight strength and inner shadow. Pointer-over and press stack in order, so a release returns to the hover optical state before pointer exit returns to the resting brush. Geometry parameters such as corner radius, bezel width, glass thickness and IOR remain material-level settings so generic state changes do not cause shape jumps.

`PointerDisplacement` adds a magnetic pointer-following translation using the WinUI `UIElement.Translation` facade while preserving and restoring the application's existing translation vector. The displacement uses a bounded `tanh` curve; `PressedDisplacementMultiplier` controls the stronger pressed response independently from `Elasticity`, which controls anisotropic liquid stretch. Either behavior can be disabled independently by setting its value to zero.

The same settings drive specialized interactions: Slider resolves the profile for its native Thumb, ToggleSwitch applies it to the glass knob, Magnifier combines it with `ActiveMagnificationMultiplier` and directional elasticity, and TabBar settings cascade to generated `LiquidGlassTabBarItem` containers. TabBar stays a real WinUI `ListView`, preserving native single-selection, keyboard/gamepad focus and UI Automation behavior.

## Search and floating surfaces

`LiquidGlassFloatingPanel` is the concrete media/player-style surface for the `FloatingPanel` preset rather than requiring applications to assemble a generic card manually.

WinUI `AutoSuggestBox` is sealed, so `LiquidGlassSearchBox` is a composable wrapper around a real native `AutoSuggestBox`. It forwards the stable search surface directly: `Text`, `PlaceholderText`, `ItemsSource`, `ItemTemplate`, `TextMemberPath`, `Header`, `QueryIcon`, suggestion-list sizing/state, `UpdateTextOnSelect`, and `AutoMaximizeSuggestionArea`. It also forwards `SuggestionChosen`, `TextChanged`, and `QuerySubmitted` with the original WinUI sender/event-args signatures. `InnerAutoSuggestBox` exposes the hosted control for advanced or future-version APIs without forcing this package to mirror the entire sealed WinUI type.

The material itself remains independently configurable through `LiquidGlassBrush`: blur/frost, saturation, contrast, exposure, tint RGB/opacity, four surface profiles, bezel geometry, glass thickness, IOR/Snell refraction, magnification, chromatic dispersion, directional lighting, highlight sharpness, specular saturation/width, inner shadow, edge softness, material opacity and fallback color.

The main templated controls use `ms-appx:///WinUI.LiquidGlass/Themes/Generic.xaml`; TabBar and specialized controls use focused resource dictionaries. Complex input controls intentionally retain or host the WinUI base controls so keyboard, gamepad, focus and UI Automation behavior stays platform-native.

The implementation keeps C++/WinRT optimized namespace modules for authored/component code and `module.g.cpp`. XAML-generated translation units use a conventional textual C++/WinRT preamble instead of named-module imports because generated code includes textual STL headers such as `<regex>`; keeping those paths separate avoids IFC/textual STL redefinition conflicts on current MSVC while preserving optimized modules for the component itself.
