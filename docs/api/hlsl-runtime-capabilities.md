# HlslRuntimeCapabilities class

Describes the packaged native Composition adapter without activating it.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0

Retrieve it with `HlslComposition.GetRuntimeCapabilities()`.

## Properties

| Property | Meaning |
| --- | --- |
| `Architecture` | `X86`, `X64`, `Arm64`, or `Unknown`. |
| `SupportLevel` | `Validated`, `Experimental`, or `Unsupported`. |
| `NativeAdapterAvailable` | This package contains an architecture-specific private adapter. |
| `SupportsCompositionGraphNodes` | Custom HLSL can participate as a Windows Graphics Effects/Composition node. |
| `SupportsMaterializedGraphs` | The materialized native-upstream -> texture -> terminal custom sampler lowering is currently declared supported. |
| `SupportsAsyncCompilation` | CPU/FXC asynchronous compilation is available. |
| `UsesPrivateCompositionAbi` | Custom effect execution relies on private Composition internals. |

## Current baseline

Released Windows App SDK **1.6 through 2.4** has been runtime-validated on both x86 and x64. That validation includes ordinary linked custom graph nodes and the current single-source `MaterializedSampler` lowering path.

| Architecture | Support | Graph nodes | Materialized graphs |
| --- | --- | --- | --- |
| x64 | Validated (WASDK 1.6-2.4) | Yes | Yes |
| x86 | Validated (WASDK 1.6-2.4) | Yes | Yes |
| ARM64 | Experimental | Yes | No capability claim yet |

Capability retrieval is intentionally side-effect free: it does not scan `wuceffectsi.dll`, patch code/import tables, or create an effect factory. Actual private-ABI resolution remains lazy and fail-closed when an effect is materialized.

Treat `SupportLevel` as the library's tested support claim, not as proof that every future Windows App SDK build is binary-compatible with the private ABI. Versions outside the validated 1.6-2.4 release range, including future preview/experimental builds, must be revalidated rather than inferred from version numbers alone.
