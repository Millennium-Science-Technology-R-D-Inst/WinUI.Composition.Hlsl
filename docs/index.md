# WinUI.Composition.Hlsl documentation

`WinUI.Composition.Hlsl` is a native Windows Runtime component for creating HLSL-backed Microsoft.UI.Composition effects and XAML material brushes in WinUI 3 applications.

## Install

```xml
<PackageReference Include="WinUI.Composition.Hlsl" Version="0.1.0-preview.8" />
```

The package contains the native implementation and the build assets required by C++/WinRT and C# WinUI applications. Consumers do not create or configure a projection project.

## Get started

### Use Liquid Glass in XAML

```xml
<Page
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
    xmlns:hlsl="using:WinUI.Composition.Hlsl">
    <Border>
        <Border.Background>
            <hlsl:LiquidGlassBrush
                BlurRadius="10"
                RefractionStrength="18"
                CornerRadius="24"
                FallbackColor="#CC202020" />
        </Border.Background>
    </Border>
</Page>
```

### Create a custom color effect

```csharp
var effect = HlslEffect.CreateColorTransform(
    "export float4 PSBody(float4 color) { return float4(color.a - color.rgb, color.a); }");

var brush = HlslComposition.CreateBackdropBrush(compositor, effect);
var xamlBrush = HlslComposition.CreateXamlBrush(brush);
```

## API reference

- [WinUI.Composition.Hlsl namespace](api/winui-composition-hlsl.md)
- [HlslEffect](api/hlsl-effect.md)
- [HlslEffectKind](api/hlsl-effect-kind.md)
- [HlslFloatProperty](api/hlsl-float-property.md)
- [HlslEffectFactory](api/hlsl-effect-factory.md)
- [HlslEffectBrush](api/hlsl-effect-brush.md)
- [HlslComposition](api/hlsl-composition.md)
- [LiquidGlassMaterial](api/liquid-glass-material.md)
- [LiquidGlassBrush](api/liquid-glass-brush.md)

## Samples

- `tests/Cpp`: complete C++/WinRT Liquid Glass demo.
- `tests/CSharp`: standard C# WinUI 3 application using `LiquidGlassBrush` and theme resources.

## Applies to

| Product | Version |
| --- | --- |
| Windows App SDK | 2.4 |
| WinUI.Composition.Hlsl | 0.1.0-preview.8 |


