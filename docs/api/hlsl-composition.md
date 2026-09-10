# HlslComposition class

Provides static entry points for creating Composition and XAML objects.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v0.1.0-preview.7  
**Assembly:** `WinUI.Composition.Hlsl.dll`


## Methods

### CreateEffectFactory

```csharp
public static HlslEffectFactory CreateEffectFactory(Compositor compositor, HlslEffect effect);
```

Compiles and registers the effect for the supplied compositor, then returns a reusable factory.

### CreateBackdropBrush

```csharp
public static HlslEffectBrush CreateBackdropBrush(Compositor compositor, HlslEffect effect);
```

Creates a factory and brush, then assigns `compositor.CreateBackdropBrush()` to the description's source name.

### CreateXamlBrush

```csharp
public static Brush CreateXamlBrush(HlslEffectBrush brush);
```

Wraps the brush in a `XamlCompositionBrushBase` so it can be assigned to `Border.Background`, `Panel.Background`, and other XAML brush properties.

## Example

```cpp
auto compositor = Microsoft::UI::Xaml::Media::CompositionTarget::GetCompositorForCurrentThread();
auto effect = WinUI::Composition::Hlsl::HlslEffect::CreateColorTransform(LR"(
export float4 PSBody(float4 color) { return float4(color.a - color.rgb, color.a); }
)");
auto brush = WinUI::Composition::Hlsl::HlslComposition::CreateBackdropBrush(compositor, effect);
MyBorder().Background(WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrush(brush));
```


