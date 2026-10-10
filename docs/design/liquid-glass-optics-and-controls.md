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

The implementation borrows mechanisms from the reference projects rather than copying their rendering stacks.

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

`luckyelysia/LiquidGlassWinUI` additionally demonstrates an important perceptual property of rounded liquid glass: a superellipse-derived corner normal and corner-weighted chromatic separation make the four corners read as curved optical surfaces rather than as a rectangular refraction field that is merely clipped to a radius. The native shader adopts that idea without replacing its existing Snell/bezel model.

## Corner coverage versus corner optics

The visible silhouette and the optical field intentionally have separate responsibilities.

The visible coverage remains the ordinary WinUI rounded-rectangle SDF. This is required because a `LiquidGlassBrush` is commonly hosted by a XAML `Border` whose `CornerRadius` uses the same circular rounded-rectangle geometry. Changing only the shader silhouette to a superellipse would let the XAML border clip the extra superellipse area and would make the two layers disagree.

Inside that compatible coverage shape, ordinary rounded rectangles use a richer optical field:

- the corner normal follows the gradient of an `n = 4` superellipse instead of the circular `n = 2` gradient;
- the virtual optical radius may extend inward to at least the bezel scale, so a thick bezel does not lose its corner curvature after the geometric corner arc collapses;
- the rounded-rectangle medial-axis normal transition is blended instead of changing direction abruptly;
- refraction, dispersion, and specular energy receive modest corner-only weighting that reaches its maximum near the 45-degree part of each corner and is exactly zero at the straight-edge tangency points.

This produces the intended visual chain:

```text
WinUI-compatible rounded coverage
        +
superellipse corner normal
        +
Snell/bezel displacement
        +
corner-weighted dispersion/specular
        =
continuous curved corner refraction
```

The correction is deliberately not applied to exact Kube/pill geometry. Borderless Kube slider optics, Lip switch optics, and circles/capsules keep their authored circular field so the reference geometry is not distorted.

The `n = 4` gradient is evaluated with cubic multiplies rather than additional `pow()` calls. This keeps the corner improvement inexpensive enough for the DWM-linked shader path.

## Why blur no longer defines the corner shape

Blur and shape coverage solve different problems and therefore must not share the same edge.

The native Gaussian stage first produces the blurred transmission source. The custom sampler can freely refract and disperse that texture. Only after sampling is complete does the shader evaluate the final rounded-rectangle coverage from its SDF and premultiply the result by that alpha.

This ordering is deliberate:

```text
Backdrop -> Gaussian blur -> materialized texture -> refract/dispersion/color -> rounded SDF coverage -> output
```

If the rounded shape were blurred together with the source, increasing `BlurRadius` would soften or visually enlarge the corner. Keeping the final coverage after the blur makes `CornerRadius` independent from `BlurRadius`.

## Constant-buffer layout

The private native-effect metadata and HLSL constant buffer must remain byte-for-byte synchronized. The corner-optics correction derives its parameters from the existing radius, bezel, profile, and geometry values and therefore does not add an ABI-visible constant-buffer property.

Every public parameter is registered through the custom-effect property table and included in `CompositionEffectFactory`'s animatable-property list. Interaction code therefore updates constants on the existing graph rather than recreating the graph.

## Interaction model

Pointer-driven refraction and highlights are evaluated against the same rounded coverage geometry. The corner optical field only changes the material response inside the shape; it does not alter hit testing, control geometry, or pointer activation bounds.

For ordinary controls, XAML `VisualStateManager` states retain the WinUI input/accessibility contract while the material properties are animated on the existing Composition graph.

## Layering responsibility

The runtime package owns HLSL compilation/linking, reusable material brushes, and the optical model. `WinUI.LiquidGlass` owns XAML control templates and interaction behavior. Keeping the ordinary rounded coverage silhouette compatible with the XAML layer prevents control-template clipping from silently changing the shader's authored corner shape.
