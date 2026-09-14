# Runtime safety and lifetime contract

`WinUI.Composition.Hlsl` keeps its public WinRT objects separate from the private Composition adapter. Private ABI work is activated only when a custom effect must actually participate in native Composition compilation.

## Lazy activation and capability reporting

`HlslComposition.GetRuntimeCapabilities()` is side-effect free. It does not load/scan/patch the private runtime just to report the package contract. Resolver installation remains lazy and fails closed if the required private mechanism cannot be resolved safely.

x86, x64, and ARM64 are public supported architectures. The NuGet package requires Windows App SDK 1.6 or later. x86/x64 resolver logic is version-independent rather than selected from a hard-coded Windows App SDK version table, so normal use does not perform a version-allowlist check before enabling the adapter.

## Process-lifetime registry

Synthetic `EffectType` objects and every byte/string/table they reference live in a process-level registry once registered because native Composition can outlive the public wrapper that created them. The registry is intentionally append-only for the process lifetime; deterministic descriptor keys prevent duplicate registrations and reject GUID reuse with a different definition.

Per-thread `Compositor`/`CompositionEffectFactory` cache entries are weak references. The cache must not keep an otherwise unreachable Window or Compositor alive.

## Validation placement

The package avoids repeating checks in the render path. Validation belongs at the earliest boundary that has the information required to make the decision:

1. `<HlslCompositionShader>` performs strict build-time FXC compilation for packaged source.
2. `HlslCompiler` performs the equivalent work only for genuinely runtime-provided source.
3. `HlslShaderLibrary` performs one-time reflection when arbitrary/cached DXBC is loaded.
4. Typed precompiled effects validate `UserConstants` against the property schema during setup.
5. Native graph lowering validates private topology/input/property assumptions while the factory graph is being compiled.
6. Brush APIs validate object/count/type facts that are only knowable from runtime objects.

There is no package-level DXBC reflection, private-runtime scan, or shader compilation loop on every frame/property update.

For samplers, source `i` is explicitly bound to `texture{i}:t{i}` and `sampler{i}:s{i}`. Build integration reflects generated DXBC to verify those slots. This is a build/test invariant, not a per-render production check.

## Typed property contract

`HlslProperty` supports scalar, vector, and matrix types. The native property blob and constant buffer are produced by one shared layout implementation. Matrix storage is a contiguous float backing representation reconstructed by generated HLSL; this avoids a mismatch between private updater memcpy semantics and HLSL matrix row/column stride.

`HlslEffectBrush` type-checks explicit setter calls. High-frequency animation should use the returned Composition property path so updates stay on the native Composition animation path.

## Public feature boundary

| Feature | Public status |
| --- | --- |
| x86/x64/ARM64 native runtime | Supported |
| linked `Color`/`Sampler`, 1-16 sources | Supported |
| explicit `textureN:tN` / `samplerN:sN` mapping | Supported and DXBC-reflection tested |
| single-source `MaterializedSampler` | Supported |
| scalar/vector/matrix properties | Supported |
| multi-source `MaterializedSampler` | Not supported |
| multiple custom nodes in one general lowered graph | Internal prototype only |
| arbitrary native node after custom materialized pass | Internal prototype only |

Internal implementation experiments are not automatically exposed as public capabilities. The last three graph-planning cases stay fail-closed until the graph planner can represent and validate their topology/bounds/input behavior explicitly.

## Hook installation

Private adapter installation is process-wide and one-time. Required patches/IAT replacements are verified. Failure to locate/patch/restore protection is an immediate HRESULT, never a deferred renderer crash. Graphs containing no registered custom node continue to forward to the original native compiler.

## XAML fallback

Fallback stays inside the normal XAML/Composition brush model whenever possible. `LiquidGlassBrush`, for example, can use `FallbackColor` if an advanced material cannot activate. The fallback is not a second swap chain or overlay visual tree.
