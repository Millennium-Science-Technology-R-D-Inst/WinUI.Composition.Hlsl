# Runtime safety and lifetime contract

`WinUI.Composition.Hlsl` deliberately separates its public WinRT surface from the private Composition adapter. The public objects are ordinary `IGraphicsEffect`, `CompositionEffectFactory`, `CompositionEffectBrush`, and `XamlCompositionBrushBase` shapes; the private adapter is activated only when a custom effect must actually participate in Composition's native effect pipeline.

This document defines the lifetime and failure rules that the runtime implementation must preserve.

## Lazy private-adapter activation

`HlslComposition.GetRuntimeCapabilities()` is intentionally side-effect free. It reports the package's architecture/support claim without loading, scanning, patching, or otherwise mutating the private Composition runtime.

Private ABI resolution is performed lazily when a custom effect is materialized for Composition. A platform can therefore report that an adapter exists while a particular Windows/Windows App SDK build still fails closed during private entry-point resolution.

Applications that require a fallback should treat capability reporting as a coarse feature gate, not as proof that an undocumented ABI will resolve on every future OS build.

## Process-lifetime effect registry

Native Composition can retain effect factories and compiled effect objects beyond the lifetime of the public wrapper that created them. Synthetic `EffectType` objects, shader payloads, metadata strings, constant-buffer mappings, and linker descriptors therefore live in a process-level registry once registered.

The registry is intentionally append-only for the process lifetime. Removing an entry while a native factory or DWM-side compiled result still holds a pointer into it would create use-after-free risk across an undocumented ABI boundary.

Definitions should therefore use deterministic descriptors and IDs. Creating the same descriptor repeatedly reuses the same registry entry. The runtime rejects an explicit GUID that is reused with a different definition and limits the number of distinct registered definitions to bound accidental unbounded growth.

Descriptor keys contain a SHA-256 fingerprint of HLSL/DXBC payloads rather than a second permanent copy of the full payload. Precompiled DXBC is still owned by the registry exactly as required by native lifetime rules.

## Factory cache ownership

The convenience factory cache must not own application Composition objects. Cached `Compositor` and `CompositionEffectFactory` references are weak references only.

This means creating an HLSL effect cannot keep a closed Window, its `Compositor`, or an otherwise unreachable effect factory alive merely because the current thread once used it. Expired cache entries are removed opportunistically on subsequent lookups.

The native process-lifetime effect registry and the per-thread factory cache solve different problems: the former owns private ABI metadata that native code may retain; the latter is only an optimization and must not extend public object lifetime.

## Validation layers

Production shader errors should be caught as early as possible.

- `<HlslCompositionShader>` performs strict build-time FXC compilation and is the preferred path for source known at build time.
- `HlslCompiler` performs the same public contract generation for runtime-generated shaders on a background thread.
- `HlslShaderLibrary` defensively reflects arbitrary/cached DXBC before it is attached to an effect.
- Native graph lowering validates private topology and property-buffer assumptions again before producing a DWM-facing compiled result.

The later layers are defensive boundaries, not substitutes for build-time shader validation.

## Property validation and animation

`HlslFloatProperty` defines the application's declared default/minimum/maximum contract. `HlslEffectBrush.SetFloat` validates finite values and that declared range before writing the underlying Composition property.

High-frequency animation should use `CompositionAnimation` directly on the returned property path/property set. Those updates execute through Composition's native property updater and intentionally do not round-trip through `SetFloat` every frame. Consequently, application-authored Composition keyframes are responsible for staying inside any semantic range the application declared.

This distinction is intentional: range checking belongs at API/control boundaries, not in the compositor's per-frame animation path.

## Private ABI support levels

The current support contract is based on runtime validation of released Windows App SDK builds:

| Architecture | Support level | Validated WASDK range | Composition graph nodes | Materialized graphs |
| --- | --- | --- | --- | --- |
| x64 | Validated | 1.6-2.4 | Yes | Yes |
| x86 | Validated | 1.6-2.4 | Yes | Yes |
| ARM64 | Experimental | Not yet validated as a release range | Yes | Not claimed |

`Validated` means the repository's documented private layout and smoke/runtime paths have been exercised successfully on the stated release range. It does not turn the private ABI into a Microsoft-supported public contract. `Experimental` means an adapter exists and builds, but the project does not make the same runtime compatibility claim.

The 1.6-2.4 range is deliberately explicit. A future Windows App SDK version, preview build, or materially different OS/runtime combination must be revalidated rather than assumed compatible because earlier releases passed.

Unknown or structurally incompatible native layouts must fail closed. The runtime must not guess RVAs, object layouts, shader-linking arguments, vector/matrix metadata, or unsupported graph shapes merely to continue rendering.

## Hook-installation rule

The private adapter is process-wide and installed once. Critical code/IAT patches must be verified rather than silently skipped. If the runtime cannot make a required page writable, install the replacement, restore the original protection, or locate a supported call site, custom-effect activation must fail immediately with an HRESULT.

A failed installation must never be reported as success and deferred to a later Composition worker crash or an unexplained `CreateEffectFactory` failure. Any partially installed compile detour must remain transparent for built-in effects by forwarding graphs that contain no registered custom node to the original native implementation.

## XAML fallback policy

Fallback behavior should remain inside the normal XAML/Composition brush model whenever possible. For example, `LiquidGlassBrush` can use `FallbackColor` when its advanced material cannot be activated.

The fallback is not an app-owned swap chain, overlay HWND, or second visual tree. The project's core rendering contract is to preserve normal XAML clipping, transforms, scaling, lifetime, and Composition scheduling even when the private custom-shader path is unavailable.

## Current fail-closed boundaries

The runtime currently does not claim general support for multiple custom HLSL nodes in one lowered graph, multiple independently materialized public texture sources, arbitrary native nodes after a terminal materialized custom sampler, or unverified vector/matrix public-property metadata. Ordinary linked `Color`/`Sampler` inputs are a separate validated path and support 1-16 ordered sources.
