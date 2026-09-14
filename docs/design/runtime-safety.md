# Runtime safety and lifetime contract

`WinUI.Composition.Hlsl` keeps its public WinRT objects separate from the private Composition adapter. Private ABI work is activated only when a custom effect must actually participate in native Composition compilation.

## Lazy activation and capability reporting

`HlslComposition.GetRuntimeCapabilities()` is side-effect free. It does not load/scan/patch the private runtime just to report the package contract. Resolver installation remains lazy and must fail closed when a loaded Windows/App SDK build cannot be recognized safely.

Released Windows App SDK 1.6-2.4 have been runtime-tested on x86 and x64. ARM64 remains experimental pending real-device validation. The tested range is evidence for those concrete releases, not a promise for future private ABI revisions.

## Process-lifetime registry

Synthetic `EffectType` objects and every byte/string/table they reference live in a process-level registry once registered because native Composition can outlive the public wrapper that created them. The registry is intentionally append-only for the process lifetime; deterministic descriptor keys prevent duplicate registrations and reject GUID reuse with a different definition.

Per-thread `Compositor`/`CompositionEffectFactory` cache entries are weak references. The cache must not keep an otherwise unreachable Window or Compositor alive.

## Validation layers

Errors are rejected as early as possible:

1. `<HlslCompositionShader>` performs strict build-time FXC compilation.
2. `HlslCompiler` emits the same public wrappers and resource binding contract for runtime-generated source.
3. `HlslShaderLibrary` reflects arbitrary/cached DXBC before attaching it to an effect.
4. Typed precompiled effects reflect `UserConstants` and validate name/type/offset/size against the property schema.
5. Native graph lowering validates private topology/input/property assumptions before returning a DWM-facing compiled result.

For samplers, source `i` is explicitly bound to `texture{i}:t{i}` and `sampler{i}:s{i}`. Build integration reflects the compiled DXBC to verify those slots rather than trusting declaration text alone.

## Typed property contract

`HlslProperty` supports scalar, vector, and matrix types. The native property blob and constant buffer are produced by one shared layout implementation. Matrix storage is a contiguous float backing representation reconstructed by generated HLSL; this avoids a mismatch between private updater memcpy semantics and HLSL matrix row/column stride.

`HlslEffectBrush` type-checks low-frequency setter calls. High-frequency animation should use the returned Composition property path so updates remain on the native Composition animation path.

## Public feature boundary

| Feature | Public status |
| --- | --- |
| linked `Color`/`Sampler`, 1-16 sources | Supported |
| explicit `textureN:tN` / `samplerN:sN` mapping | Supported and DXBC-reflection tested |
| single-source `MaterializedSampler` | Supported on validated x86/x64 runtime range |
| scalar/vector/matrix properties | Implemented and reflection-validated |
| multi-source `MaterializedSampler` | Not supported |
| multiple custom nodes in one lowered graph | Internal prototype only |
| arbitrary native node after custom materialized pass | Internal prototype only |

The last three cases remain fail-closed. Existing internal code that can preserve/materialize more than one pass is not itself a public compatibility guarantee.

## Hook installation

Private adapter installation is process-wide and one-time. Required patches/IAT replacements must be verified. Failure to locate/patch/restore protection is an immediate HRESULT, never a deferred renderer crash. Graphs containing no registered custom node must continue to forward transparently to the original native compiler.

## XAML fallback

Fallback should remain inside the normal XAML/Composition brush model whenever possible. `LiquidGlassBrush`, for example, can use `FallbackColor` if the advanced material cannot activate. The fallback is not a second swap chain or overlay visual tree.
