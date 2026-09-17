# WinUI.Composition.Hlsl API reference

The `WinUI.Composition.Hlsl` namespace exposes WinRT APIs for compiling HLSL, describing custom effects, creating Composition factories/brushes, updating typed properties, and bridging Composition brushes into XAML. The `WinUI.LiquidGlass` namespace adds native WinUI 3 controls built on the liquid-glass material.

## Namespace

```text
WinUI.Composition.Hlsl
```

The same WinRT surface is available to C++/WinRT and C# through the package's CsWinRT projection.

## Core types

| Type | Description |
| --- | --- |
| [`HlslComposition`](hlsl-composition.md) | Static helpers for capability reporting, factory/brush creation, source binding, and Composition-to-XAML bridging. |
| [`HlslEffect`](hlsl-effect.md) | Immutable description of a custom HLSL effect, including its source schema and typed property schema. |
| [`HlslCompiler`](hlsl-compiler.md) | Asynchronously compiles runtime-provided HLSL into an immutable shader library. |
| [`HlslShaderLibrary`](hlsl-shader-library.md) | Holds compiled DXBC and its resolved profile, effect kind, and source count. |
| [`HlslProperty`](hlsl-property.md) | Declares a typed animatable shader property. |
| [`HlslEffectFactory`](hlsl-effect-factory.md) | Wraps a `CompositionEffectFactory` created for an `HlslEffect`. |
| [`HlslEffectBrush`](hlsl-effect-brush.md) | Wraps the created `CompositionEffectBrush` and exposes source/property operations. |
| [`HlslRuntimeCapabilities`](hlsl-runtime-capabilities.md) | Reports the package capability contract for the current architecture without activating the private adapter. |

## Material types

| Type | Description |
| --- | --- |
| [`LiquidGlassMaterial`](liquid-glass-material.md) | Composition-level reusable liquid-glass material. |
| [`LiquidGlassBrush`](liquid-glass-brush.md) | XAML `XamlCompositionBrushBase` wrapper for the liquid-glass material. |

## WinUI.LiquidGlass controls

Namespace:

```text
WinUI.LiquidGlass
```

| API | Description |
| --- | --- |
| [`LiquidGlassCard` and control library](liquid-glass-controls.md) | Native Button, ToggleButton, CheckBox, RadioButton, Slider, input, TabBar, SearchBox, Magnifier, and panel controls. |
| [`LiquidGlassInteraction`](liquid-glass-interaction.md) | Attached properties for deterministic motion and optical interaction states. |
| `LiquidGlassPresets` | Creates independent `LiquidGlassBrush` instances configured for the built-in control geometries. |
| `LiquidGlassPreset` | Identifies Panel, Button, Choice, SearchBox, Input, SliderThumb, ToggleSwitchKnob, Magnifier, FloatingPanel, TabBar, or TabBarItem. |

The controls inherit their semantic behavior from WinUI controls whenever possible. The glass package changes visual surfaces and Composition interaction state; it does not replace WinUI click, selection, keyboard, focus, pointer-capture, command, or UI Automation contracts.

## Enumerations

| Enumeration | Purpose |
| --- | --- |
| [`HlslEffectKind`](hlsl-effect-kind.md) | Selects the HLSL calling contract: `Color`, `Sampler`, `MaterializedSampler`, or compiler-only `Auto`. |
| `HlslPropertyType` | Selects `Scalar`, `Vector2`, `Vector3`, `Vector4`, `Matrix3x2`, or `Matrix4x4`. |
| `HlslShaderProfile` | Selects the FXC linkable-library profile: `Level91`, `Level93`, or `Pixel40`. |
| `HlslNativeArchitecture` | Identifies `X86`, `X64`, `Arm64`, or `Unknown`. |
| `HlslRuntimeSupportLevel` | Reports `Validated`, `Experimental`, or `Unsupported`. Current shipped x86/x64/ARM64 targets report `Validated`. |

## Recommended API path

For packaged shaders known at build time:

```text
<HlslCompositionShader>
        -> generated DXBC/.g.h
        -> HlslShaderLibrary generated loader
        -> HlslEffect.CreateCompiled(...)
        -> HlslComposition.CreateEffectFactory/CreateBackdropBrush
        -> HlslEffectBrush
```

For runtime-provided HLSL:

```text
HlslCompiler.Compile*Async(...)
        -> HlslShaderLibrary
        -> HlslEffect.CreateCompiled(...)
        -> Composition
```

Prefer the build-time path for normal application assets. Use `HlslCompiler` when source text is genuinely dynamic.

## API contract conventions

### Null values

Required WinRT objects must be non-null. Methods that need a `Compositor`, effect, source collection, shader library, or file reject missing objects with an invalid-argument error.

### Source order

For advanced multi-source effects, collection order is significant. `SourceNames[i]`, source brush `i`, and linked sampler resources `texture{i}/t{i}` and `sampler{i}/s{i}` refer to the same logical input.

### Property types

A property must be updated with the setter that matches its `HlslPropertyType`. Property metadata is fixed when the effect definition is created; changing a property value does not change the shader schema.

### Threading and performance

`HlslCompiler` is asynchronous. Library reflection and effect validation are setup-time operations. Brush property updates and Composition rendering do not repeat HLSL compilation/reflection.

Liquid-glass pointer position and velocity are transient Composition effect state rather than dependency properties. This keeps high-frequency spatial interaction out of XAML property invalidation while preserving ordinary WinUI semantic input handling on the CPU.

### Private runtime adapter

Public APIs are WinRT, but custom shader execution uses an internal Composition adapter. The package supports x86, x64, and ARM64 and has a Windows App SDK minimum of 1.6. `GetRuntimeCapabilities()` does not scan or install the adapter merely to report static package capabilities.

## See also

- [Get started](../get-started.md)
- [Concepts](../concepts.md)
- [Architecture](../architecture.md)
- [WinUI.LiquidGlass controls](liquid-glass-controls.md)
