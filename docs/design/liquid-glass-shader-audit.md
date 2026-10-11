# Shader comparison and validation

Reference: [LiquidGlassWinUI shaders](https://github.com/luckyelysia/LiquidGlassWinUI/tree/main/LiquidGlassWinUI/Effects/Shaders).
The reference has four HLSL files; this project has three because horizontal and
vertical blur share one file with separate exports. File count is not stage count.

| Reference file | Corresponding implementation | Checked behavior |
| --- | --- | --- |
| `BlurH.hlsl` | `SeparableGaussianBlur.hlsl::BlurHorizontal` | Horizontal reciprocal texel size, symmetric ten merged pairs, normalized energy, mirrored boundaries, 112-byte constants. |
| `BlurV.hlsl` | `SeparableGaussianBlur.hlsl::BlurVertical` | Vertical reciprocal texel size, identical kernel/weights, independent factory/brush boundary. |
| `LiquidGlass.hlsl` | `LiquidGlass.hlsl` | Materialized content-rect coordinates, DPI-scaled geometry/bands, Snell refraction, tunable corner normals, eight-tap refraction footprint, bounded seven-tap spectral dispersion, Fresnel, directional/opposite glare, magnification, coverage and premultiplication. |
| `PostProcessing.hlsl` | `LiquidGlassPostProcessing.hlsl` | Two genuine inputs (blurred/raw), blur mix, exposure, temperature, brightness, contrast, saturation/vibrance, straight-color adjustment and premultiplied output. |

Important differences are intentional and explicit:

- The blur's Gaussian weights are normalized; **sample offsets are not**. The
  reference CPU setup normalizes both alternating offset/weight values. This
  project keeps offsets in texel units. Bilinear pair merging is exact at unit
  spacing; scaled spacing is a fixed-cost approximation, not an exact arbitrary-
  radius Gaussian. `BlurRadius` is an authored DIP radius, not `BlurAmount` spacing.
- `BloomAmount` follows the reference shader's actual expression, not its reversed
  comment: 0 is raw and 1 is blurred. Our default is 1 to preserve existing blur.
- Color operations unpremultiply/re-premultiply. Applying additive brightness or
  mid-grey contrast directly to premultiplied RGB would corrupt transparent edges.
- `Exposure` remains EV (`exp2`) rather than the reference's multiplicative gain.
- Coverage stays compatible with a WinUI circular `CornerRadius`; `ShapeRoundness`
  adjusts the optical normal. Relative `ShapeRadius` is opt-in, with -1 retaining
  absolute radius. The Kube/pill opt-out and four surface profiles are retained.
- Fresnel/glare are independent response controls, not aliases for narrow Kube
  specular. New factors default to 0; magnification defaults to identity.
- Source indices occupy the low byte of linker arguments. The post shader uses
  `{0x0200, 0x0201}`; generated multi-input effects now encode each source index
  instead of selecting source 0 for every argument.
- Spectral weights sum to one per channel and sampling stays inside the valid
  content rectangle. Extent/zero-alpha/zero-range guards avoid division by zero,
  dark void sampling and unstable optical bands.

The optics buffer is 208 bytes with appended P10/P11/P12 at 160/176/192; existing
offsets are unchanged. The post buffer is 32 bytes with exposure at byte 24.
CI compiles all built-in libraries and runs native DXBC reflection checks on
exports, resource registers, parameter counts and these buffer layouts. Packaged
C++ and C# consumers compile every added Material property and brush DP. The
Demo smoke path also exercises nonzero Fresnel/glare and transmission controls.

These checks prove compilation and ABI consistency. They do not replace visual
evaluation of different backdrops, DPIs, transparency and animation on a desktop.
