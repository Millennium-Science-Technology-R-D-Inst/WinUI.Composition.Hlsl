# WinUI.LiquidGlass controls

`WinUI.LiquidGlass` is the native C++/WinRT WinUI 3 control package built on top of `WinUI.Composition.Hlsl`. It keeps normal XAML controls, WinUI input semantics, UI Automation, layout, and Composition while replacing the visual material with `LiquidGlassBrush`.

The controls package is intentionally separate from the core shader/effect package:

```text
WinUI.Composition.Hlsl
    custom HLSL effect runtime
    LiquidGlassMaterial
    LiquidGlassBrush

WinUI.LiquidGlass
    native C++/WinRT WinUI controls
    XAML templates and resources
    interaction/motion helpers
    per-control glass presets
```

## Installation

Use matching package versions:

```xml
<ItemGroup>
  <PackageReference Include="WinUI.Composition.Hlsl" Version="1.0.*" />
  <PackageReference Include="WinUI.LiquidGlass" Version="1.0.*" />
</ItemGroup>
```

`WinUI.LiquidGlass` has an exact-version dependency on the core package when packed by this repository.

## Controls

The package currently contains:

- `LiquidGlassCard`
- `LiquidGlassMagnifier`
- `LiquidGlassButton`
- `LiquidGlassToggleButton`
- `LiquidGlassHyperlinkButton`
- `LiquidGlassCheckBox`
- `LiquidGlassRadioButton`
- `LiquidGlassSlider`
- `LiquidGlassTextBox`
- `LiquidGlassPasswordBox`
- `LiquidGlassComboBox`
- `LiquidGlassToggleSwitch`
- `LiquidGlassTabBarItem`
- `LiquidGlassTabBar`

Each control owns its own `LiquidGlassBrush` instance. Replacing `GlassBrush` does not rebuild the shader graph; material parameters remain animatable through Composition property paths.

## Interaction ownership

The control library follows one important rule: one visual property has one interaction owner.

For example, a control should not have XAML VisualState storyboards and a C++ Composition helper both writing the same `Scale`, `Translation`, or optical material property. The current controls split responsibilities between:

- native WinUI state and input semantics;
- Composition scale/translation/elevation;
- `LiquidGlassBrush` optical parameters;
- shared pointer-field data for local highlight/refraction response.

This rule is especially important for Slider, ToggleSwitch, and Magnifier, where multiple independent animation systems quickly produce jumps, delayed response, or teardown-time writes.

## LiquidGlassSlider

`LiquidGlassSlider` deliberately keeps the real WinUI Slider mechanics.

The native semantic Thumb remains `18 x 18` DIPs. WinUI uses the real Thumb `ActualWidth`/`ActualHeight` for track layout and pointer-to-value conversion, so the 90 x 60 glass lens must not replace that semantic footprint.

```text
native Thumb: 18 x 18
    owns value/layout/input/UIA

visual glass lens: 90 x 60
    owns material/scale/elevation/pointer field
```

At rest the glass lens is scaled to `0.6`, producing a visible width of 54 DIPs. While pressed it scales to `1.0`. The scale spring follows the Kube reference values (`stiffness=2000`, `damping=80`).

The progress fill is a separate Composition rounded-rectangle visual over the real WinUI track. Its width represents the full normalized Slider value, so the blue/gray boundary can move through the glass lens instead of always stopping at the semantic Thumb center.

A lightweight `LayoutUpdated` correction is still required because WinUI lays out the native Thumb after value changes. That callback only transforms the native Thumb center into track coordinates and updates lens translation. It must not walk the template tree, allocate brushes, or rewrite resources on every layout pass.

The active optical response mirrors the Kube reference conceptually:

- rest scale: `0.6`
- pressed scale: `1.0`
- refraction target: approximately `0.4 -> 0.9` relative strength (`2.25x`)
- active tint is reduced so the refracted backdrop becomes more visible
- the native Slider remains responsible for keyboard, pointer value changes, RTL/direction behavior, and UI Automation

## LiquidGlassToggleSwitch

`LiquidGlassToggleSwitch` uses a composable `ToggleButton` semantic base because the WinUI `ToggleSwitch` runtime class is sealed.

The visual model uses the Kube geometry:

- track: `160 x 67`
- glass knob: `146 x 92`
- rest scale: `0.65`
- pressed scale: `0.9`
- travel: `57.9` DIPs
- overscroll damping: `/22`
- drag threshold: 4 DIPs

Semantic `IsChecked` remains the source of truth. Drag interaction may preview an intermediate ratio, but release commits the semantic state before the final spring settles. This prevents native `ToggleButton::OnToggle`, pointer capture loss, and the visual ratio from starting conflicting transitions.

## LiquidGlassMagnifier

The magnifier follows the Kube interaction model rather than filtering pointer position itself:

- pointer translation follows input directly (`dragMomentum=false` behavior);
- object scale uses a continuous unit-mass spring (`stiffness=340`, `damping=20`);
- horizontal velocity drives squash/stretch through a second spring (`stiffness=340`, `damping=30`);
- `scaleY = objectScale * max(0.7, 1 - abs(velocityX) / 5000)`;
- `scaleX = objectScale + (1 - scaleY)`;
- magnification and refraction strengthen while dragging;
- external elevation and shader inner-shadow remain separate layers.

The spring state is continuous across pointer updates. Restarting a new Composition spring for every `PointerMoved` event discards accumulated velocity and makes high-polling-rate mice look unnaturally rigid.

## Liquid-glass rendering pipeline

The current material pipeline is:

```text
Backdrop
  -> separable Gaussian horizontal pass
  -> separable Gaussian vertical pass
  -> LiquidGlass HLSL
  -> final SDF coverage
```

The Gaussian passes are materialized before the final custom sampler. Rounded coverage is applied after blur/refraction rather than before the blur, so the material does not blur a pre-cut transparent rounded rectangle.

The primary source sample order remains:

```text
Source(x + Refraction(x) + Magnification(x + Refraction(x)))
```

The shader is built with FXC as SM4/DXBC. The project does not migrate this path to DXC/SM6 because the private Windows Composition effect linker used by this repository consumes the SM4/DXBC form.

## Pointer specular highlight

Pointer highlight is calculated in the HLSL material rather than by placing a generic Fluent Reveal overlay above the control.

Conceptually:

```text
pointer specular
    = edge profile
    * angular response
    * radial pointer light field
```

The shared pointer field supplies local pointer position/velocity to glass surfaces. This allows large control groups to share pointer routing while each surface evaluates its own local specular/refraction response.

## Teardown and lifetime rules

`Unloaded` can race `XamlCompositionBrushBase::OnDisconnected`. Control helpers therefore follow a no-write teardown rule:

- release references and transient pointer state;
- do not restore optics by writing to a closing `CompositionEffectBrush`;
- preserve numeric baselines when required;
- restore/recompute them on the next `Loaded` only after a live brush is available.

This rule avoids close-time `RO_E_CLOSED`/closed-object writes while still preventing pressed/focused values from becoming a new baseline after navigation or template replacement.

## Reference implementations

The interaction and optical work has been compared against several public liquid-glass implementations, including Kube's CSS/SVG examples, `LiquidGlassWinUI`, WebGL liquid-glass experiments, and WinUIEssentials reveal/focus helpers. Reference projects are used to validate interaction structure and optical ideas; the production library keeps native WinUI mechanics and its own C++/WinRT/Composition architecture.

See `THIRD-PARTY-NOTICES.md` in `WinUI.LiquidGlass` for attribution where applicable.
