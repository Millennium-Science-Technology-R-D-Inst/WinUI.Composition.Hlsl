# LiquidGlassInteraction class

Defines attached properties that configure motion and optical state transitions for `WinUI.LiquidGlass` controls.

## Namespace

```text
WinUI.LiquidGlass
```

## Definition

```text
static runtimeclass LiquidGlassInteraction
```

The class is static. Each attached property has the standard WinUI triplet:

```text
DependencyProperty <Name>Property { get; }
<T> Get<Name>(DependencyObject element)
void Set<Name>(DependencyObject element, <T> value)
```

In XAML, set the attached property with `liquid:LiquidGlassInteraction.<Name>`.

## Remarks

Interaction state is resolved in deterministic order:

```text
baseline -> activated -> pointer-over -> pressed
```

Focus optics are composed through the focus helper for input controls. Persistent selected/toggled state is recomputed from the authored baseline; it is not restored from an event-order snapshot.

Scale/motion can use Composition spring animations. Optical scalar properties use bounded key-frame transitions so quantities such as blur, saturation, refraction, contrast, and opacity do not overshoot their valid ranges.

High-frequency pointer position and velocity used by the SDF interaction field are transient Composition state and are intentionally not exposed as dependency properties.

## Pointer lighting properties

| Property | Type | Description |
| --- | --- | --- |
| `PointerLightingEnabled` | `Boolean` | Enables pointer-driven light-angle tracking. |
| `PointerLightInfluence` | `Double` | Blends the authored light direction with the pointer direction. Typical range is 0..1. |

Pointer lighting changes the live material while the pointer is active and restores the current authored `LiquidGlassBrush.LightAngle` when tracking ends or the brush is replaced.

## Motion properties

| Property | Type | Description |
| --- | --- | --- |
| `RestScale` | `Double` | Scale used in the resting state. |
| `PointerOverScale` | `Double` | Scale used while the pointer is over the control. |
| `PressedScale` | `Double` | Scale used while pressed or actively dragged. |
| `FocusedScale` | `Double` | Scale used for focused input controls. |
| `MotionDuration` | `Double` | Motion transition duration in milliseconds when key-frame motion is used. |
| `OpticsTransitionDuration` | `Double` | Duration in milliseconds for optical scalar transitions. |
| `UseSpringMotion` | `Boolean` | Uses Composition spring motion for eligible scale/translation transitions. |
| `SpringDampingRatio` | `Double` | Damping ratio for spring motion. |
| `SpringPeriod` | `Double` | Natural period used by the spring animation. |
| `RespectSystemAnimations` | `Boolean` | Honors the system animation setting when applying motion. |
| `Elasticity` | `Double` | Controls pointer/drag stretch response. |
| `PointerDisplacement` | `Double` | Maximum pointer-driven translation amount. |
| `PressedDisplacementMultiplier` | `Double` | Multiplies pointer displacement while pressed. |

## Activated-state optical properties

These properties describe persistent state such as a selected tab or checked toggle.

| Property | Type | Description |
| --- | --- | --- |
| `ActivatedRefractionMultiplier` | `Double` | Multiplies baseline refraction. |
| `ActivatedDispersionMultiplier` | `Double` | Multiplies chromatic dispersion. |
| `ActivatedSaturationMultiplier` | `Double` | Multiplies saturation. |
| `ActivatedContrastMultiplier` | `Double` | Multiplies contrast. |
| `ActivatedTintBoost` | `Double` | Adds to tint opacity. |
| `ActivatedHighlightMultiplier` | `Double` | Multiplies highlight strength. |
| `ActivatedInnerShadowBoost` | `Double` | Adds to inner-shadow strength. |

## Pointer-over optical properties

| Property | Type | Description |
| --- | --- | --- |
| `PointerOverBlurBoost` | `Double` | Adds to blur radius while pointer-over. |
| `PointerOverRefractionMultiplier` | `Double` | Multiplies refraction. |
| `PointerOverRefractionBoost` | `Double` | Adds to refraction strength after multiplication. |
| `PointerOverDispersionMultiplier` | `Double` | Multiplies dispersion. |
| `PointerOverSaturationMultiplier` | `Double` | Multiplies saturation. |
| `PointerOverContrastMultiplier` | `Double` | Multiplies contrast. |
| `PointerOverExposureBoost` | `Double` | Adds exposure in stops. |
| `PointerOverTintBoost` | `Double` | Adds tint opacity. |
| `PointerOverHighlightMultiplier` | `Double` | Multiplies highlight strength. |
| `PointerOverHighlightBoost` | `Double` | Adds highlight strength after multiplication. |
| `PointerOverInnerShadowBoost` | `Double` | Adds inner-shadow strength. |

## Pressed-state optical properties

| Property | Type | Description |
| --- | --- | --- |
| `PressedBlurBoost` | `Double` | Adds to blur radius while pressed. |
| `PressedRefractionMultiplier` | `Double` | Multiplies refraction. |
| `PressedRefractionBoost` | `Double` | Adds to refraction after multiplication. |
| `PressedDispersionMultiplier` | `Double` | Multiplies dispersion. |
| `PressedSaturationMultiplier` | `Double` | Multiplies saturation. |
| `PressedContrastMultiplier` | `Double` | Multiplies contrast. |
| `PressedExposureBoost` | `Double` | Adds exposure in stops. |
| `PressedTintBoost` | `Double` | Adds tint opacity. |
| `PressedHighlightMultiplier` | `Double` | Multiplies highlight strength. |
| `PressedHighlightBoost` | `Double` | Adds highlight strength after multiplication. |
| `PressedInnerShadowBoost` | `Double` | Adds inner-shadow strength. |

## Focused-state optical properties

| Property | Type | Description |
| --- | --- | --- |
| `FocusedBlurBoost` | `Double` | Adds blur while focused. |
| `FocusedRefractionMultiplier` | `Double` | Multiplies refraction. |
| `FocusedDispersionMultiplier` | `Double` | Multiplies dispersion. |
| `FocusedSaturationMultiplier` | `Double` | Multiplies saturation. |
| `FocusedContrastMultiplier` | `Double` | Multiplies contrast. |
| `FocusedExposureBoost` | `Double` | Adds exposure in stops. |
| `FocusedTintBoost` | `Double` | Adds tint opacity. |
| `FocusedHighlightBoost` | `Double` | Adds highlight strength. |
| `FocusedInnerShadowBoost` | `Double` | Adds inner-shadow strength. |

## Magnification property

| Property | Type | Description |
| --- | --- | --- |
| `ActiveMagnificationMultiplier` | `Double` | Multiplies `LiquidGlassBrush.MagnificationStrength` while the control is active. The default Magnifier uses 2.0, producing 24 -> 48 magnification. |

## Example

```xml
<liquid:LiquidGlassButton
    Content="Render"
    liquid:LiquidGlassInteraction.RestScale="1"
    liquid:LiquidGlassInteraction.PointerOverScale="1.02"
    liquid:LiquidGlassInteraction.PressedScale="0.96"
    liquid:LiquidGlassInteraction.PointerLightInfluence="0.7"
    liquid:LiquidGlassInteraction.PressedRefractionMultiplier="1.18"
    liquid:LiquidGlassInteraction.PressedTintBoost="0.04" />
```

## Attached-property inheritance

The implementation resolves an authored value from the element and, when required by a template-part interaction, walks the visual parent chain. This allows a `Thumb` or other internal template part to consume interaction values authored on its owning liquid-glass control.

## Validation

Floating-point interaction properties reject non-finite values. Material-side values are clamped or validated again against the corresponding `LiquidGlassMaterial` range before being written to Composition.

## See also

- [WinUI.LiquidGlass controls](liquid-glass-controls.md)
- [LiquidGlassBrush](liquid-glass-brush.md)
- [LiquidGlassMaterial](liquid-glass-material.md)
