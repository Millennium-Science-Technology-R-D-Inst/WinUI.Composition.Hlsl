# Liquid Glass optics and control-layer design

## Goal

The built-in Liquid Glass material should be more than a fixed artistic distortion. It should expose the same useful conceptual controls as browser/WebGL implementations while remaining native to WinUI Composition:

- backdrop blur;
- rounded optical geometry;
- refractive displacement;
- chromatic dispersion;
- directional specular response;
- tint and saturation;
- pointer-driven interaction without rebuilding the effect graph;
- reusable XAML control styling that can evolve independently from the private HLSL runtime.

## Reference mapping

The implementation borrows two mechanisms from the reference projects rather than copying their rendering stacks.

The `liunnn1994` implementation separates a one-dimensional bezel profile from the two-dimensional rounded-rectangle displacement field. Its important idea is that displacement is derived from surface geometry and refractive index, then directed along the rounded-rectangle edge normal.

The `archisvaze/liquid-glass` WebGL shader expresses the same chain directly in a fragment shader:

1. evaluate rounded-rectangle signed distance;
2. turn distance from the edge into normalized bezel coordinate `t`;
3. evaluate a convex surface-height function;
4. numerically differentiate that function to obtain the surface slope;
5. apply Snell refraction using IOR;
6. convert the incident/refracted angular difference and glass thickness into a screen-space displacement;
7. offset sampling along the rounded-rectangle normal;
8. add directional rim/specular, tint and inner-edge response.

`LiquidGlass.hlsl` follows that chain on the GPU. It retains `RefractionStrength` after the physically-derived displacement as an artistic multiplier because UI material design often needs exaggeration or suppression beyond literal optics.

## Why blur no longer defines the corner shape

Blur and shape coverage solve different problems and therefore must not share the same edge.

The native Gaussian stage first produces the blurred transmission source. The custom sampler can freely refract and disperse that texture. Only after sampling is complete does the shader evaluate the final rounded-rectangle coverage from its SDF and premultiply the result by that alpha.

This ordering is deliberate:

```text
Backdrop -> Gaussian blur -> materialized texture -> refract/dispersion/color -> rounded SDF coverage -> output
```

If the rounded shape were blurred together with the source, increasing `BlurRadius` would soften or visually enlarge the corner. Keeping the final coverage after the blur makes `CornerRadius` independent from `BlurRadius`.

## Constant-buffer layout

The private native-effect metadata and HLSL constant buffer must remain byte-for-byte synchronized.

```text
MaterialParams0: BorderThickness, CornerRadius, RefractionStrength, BezelWidth
MaterialParams1: HighlightStrength, EdgeSoftness, DispersionStrength, MaterialOpacity
MaterialParams2: GlassThickness, RefractiveIndex, TintOpacity, Saturation
MaterialParams3: LightAngle, reserved, reserved, reserved
```

Every public parameter is registered through the custom-effect property table and included in `CompositionEffectFactory`'s animatable-property list. Interaction code therefore updates constants on the existing graph rather than recreating the graph.

## Interaction model

The C++ demo demonstrates two levels of motion.

For the large draggable glass surface, pointer position drives only `LightAngle`. Press/release feedback drives the XAML element's Composition `Visual.Scale`. Both operations stay on the Composition side and leave the expensive material graph intact.

For ordinary controls, `LiquidGlassControls.xaml` uses XAML `VisualStateManager` states. Button and ToggleButton explicitly animate Normal, PointerOver, Pressed, Disabled, Focused and Checked states. This keeps normal WinUI input/accessibility semantics while replacing only the presentation.

## Why controls are not in the core DLL yet

The runtime package has a narrow responsibility: compile/link HLSL through the Composition backend and expose reusable material brushes. XAML control templates evolve with WinUI visual conventions and have a different compatibility surface.

For that reason the first control layer is a standalone ResourceDictionary in the C++ demo. If the templates stabilize, the recommended next step is a sibling package/project:

```text
src/
  WinUI.Composition.Hlsl/           # runtime, effects, brushes
  WinUI.Composition.Hlsl.Controls/  # optional XAML control resources / controls
```

The controls project should depend on the runtime package, never the reverse. That dependency direction prevents control-template changes from destabilizing the HLSL/Composition ABI.
