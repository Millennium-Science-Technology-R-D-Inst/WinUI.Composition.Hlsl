# HlslRuntimeCapabilities class

Describes the packaged native Composition adapter without activating it.

**Namespace:** `WinUI.Composition.Hlsl`  
**Package:** `WinUI.Composition.Hlsl` v1.0.0

Retrieve it with `HlslComposition.GetRuntimeCapabilities()`.

## Properties

| Property | Meaning |
| --- | --- |
| `Architecture` | `X86`, `X64`, `Arm64`, or `Unknown`. |
| `SupportLevel` | `Validated`, `Experimental`, or `Unsupported`. This qualifies the confidence of the architecture/runtime claim. |
| `NativeAdapterAvailable` | This package contains an architecture-specific private adapter. |
| `SupportsCompositionGraphNodes` | Custom HLSL can participate as a Windows Graphics Effects/Composition node. |
| `SupportsMaterializedGraphs` | The currently supported single-materialized-source lowering is available on this architecture. |
| `SupportsLinkedMultiSource` | Linked `Color`/`Sampler` effects can expose multiple independently bound public sources. |
| `SupportsMaterializedMultiSource` | Multiple independently materialized texture inputs are a supported public contract. |
| `SupportsMultipleCustomNodes` | More than one custom HLSL node in the same effect graph is a supported public contract. |
| `SupportsNativeNodesAfterCustom` | Native Composition effects after a custom HLSL pass are a supported public contract. |
| `SupportsVectorProperties` | `Vector2`/`Vector3`/`Vector4` typed animated properties are implemented. |
| `SupportsMatrixProperties` | `Matrix3x2`/`Matrix4x4` typed animated properties are implemented. |
| `SupportsAsyncCompilation` | CPU/FXC asynchronous compilation is available. |
| `UsesPrivateCompositionAbi` | Custom effect execution relies on private Composition internals. |

Capability booleans describe the public contract currently exposed by the library. Internal prototypes do not become `true` merely because the runtime contains exploratory lowering code. `SupportLevel` still qualifies how broadly that contract has been validated on the current architecture.

## Current baseline

| Architecture | Support | Graph nodes | Linked multi-source | Single materialized graph | Materialized multi-source | Multiple custom nodes | Native after custom | Vector / matrix properties |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| x64 | Validated baseline | Yes | Yes | Yes | No | No | No | Yes / Yes |
| x86 | Experimental | Yes | Yes | No capability claim yet | No | No | No | Yes / Yes |
| ARM64 | Experimental | Yes | Yes | No capability claim yet | No | No | No | Yes / Yes |

The `SupportsMultipleCustomNodes` and `SupportsNativeNodesAfterCustom` flags intentionally remain `false` while the existing multi-pass lowering remains an internal prototype. Likewise, `SupportsMaterializedMultiSource` stays `false` until each materialized input has an independently validated materialization boundary, sampler-data mapping, edge-mode mapping, and runtime test.

Capability retrieval is intentionally side-effect free: it does not scan `wuceffectsi.dll`, patch code/import tables, or create an effect factory. Actual private-ABI resolution remains lazy and fail-closed when an effect is materialized.

Treat `SupportLevel` as the library's tested support claim, not as proof that every future Windows App SDK build is binary-compatible with the private ABI.
