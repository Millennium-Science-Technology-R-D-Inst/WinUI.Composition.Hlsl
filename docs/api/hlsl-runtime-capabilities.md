# HlslRuntimeCapabilities class

Describes the packaged native Composition adapter without activating it.

Retrieve it with `HlslComposition.GetRuntimeCapabilities()`.

## Properties

| Property | Meaning |
| --- | --- |
| `Architecture` | `X86`, `X64`, `Arm64`, or `Unknown`. |
| `SupportLevel` | `Validated`, `Experimental`, or `Unsupported`. |
| `NativeAdapterAvailable` | An architecture-specific private adapter is present. |
| `SupportsCompositionGraphNodes` | Custom HLSL can participate as a Windows Graphics Effects/Composition node. |
| `SupportsMaterializedGraphs` | The supported single-materialized-source lowering is available. |
| `SupportsLinkedMultiSource` | Linked `Color`/`Sampler` can expose multiple independently bound sources. |
| `SupportsMaterializedMultiSource` | Multiple independently materialized texture inputs are a supported public contract. |
| `SupportsMultipleCustomNodes` | More than one custom HLSL node in one lowered graph is a supported public contract. |
| `SupportsNativeNodesAfterCustom` | Native effects after a custom materialized pass are a supported public contract. |
| `SupportsVectorProperties` | `Vector2/3/4` typed properties are implemented. |
| `SupportsMatrixProperties` | `Matrix3x2/4x4` typed properties are implemented. |
| `SupportsAsyncCompilation` | Background CPU/FXC compilation is available. |
| `UsesPrivateCompositionAbi` | Custom effect execution relies on private Composition internals. |

Capability retrieval is side-effect free: it does not scan `wuceffectsi.dll`, patch code/IAT entries, or create an effect factory. Actual private-ABI resolution remains lazy and fail-closed.

## Current matrix

| Architecture | Support | Released WASDK runtime-tested | Linked multi-source | Single materialized | Materialized multi-source | Multiple custom | Native after custom | Vector / matrix properties |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| x64 | Validated | 1.6-2.4 | Yes | Yes | No | No | No | Yes / Yes |
| x86 | Validated | 1.6-2.4 | Yes | Yes | No | No | No | Yes / Yes |
| ARM64 | Experimental | no real-device validation claim | Yes | No public claim | No | No | No | Yes / Yes |

`Validated` is an empirical tested-release claim, not a guarantee that a future Windows App SDK build preserves the private ABI. The feature-specific booleans describe the public contract; internal graph-lowering prototypes do not become public merely because code exists for them.
