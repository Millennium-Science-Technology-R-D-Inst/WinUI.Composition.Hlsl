# WinUI.LiquidGlass controls

Provides native WinUI 3 controls whose visual surfaces are backed by `WinUI.Composition.Hlsl.LiquidGlassBrush` while retaining WinUI input, focus, keyboard, automation, and command semantics.

## Namespace

```text
WinUI.LiquidGlass
```

## Requirements

- Windows App SDK / WinUI 3 desktop application.
- Add a package or project reference to `WinUI.LiquidGlass`.
- The controls package references `WinUI.Composition.Hlsl` for the Composition material implementation.

## XAML namespace

```xml
xmlns:liquid="using:WinUI.LiquidGlass"
```

## Control types

| Type | Inherits | Description |
| --- | --- | --- |
| `LiquidGlassCard` | `ContentControl` | General-purpose rounded glass container. |
| `LiquidGlassMagnifier` | `ContentControl` | Draggable magnifying lens with 24→48 magnification and velocity deformation. |
| `LiquidGlassButton` | `Button` | Native button semantics with a liquid-glass surface. |
| `LiquidGlassToggleButton` | `ToggleButton` | Toggle button using WinUI's combined checked/pointer states. |
| `LiquidGlassHyperlinkButton` | `HyperlinkButton` | Hyperlink-button semantics with a liquid-glass surface. |
| `LiquidGlassCheckBox` | `CheckBox` | Check box whose 20×20 choice glyph is the glass surface. |
| `LiquidGlassRadioButton` | `RadioButton` | Radio button whose circular choice glyph is the glass surface. |
| `LiquidGlassSlider` | `Slider` | Native Slider mechanics with a kube-compatible 90×60 optical thumb. |
| `LiquidGlassTextBox` | `TextBox` | Text input with focus-driven glass optics. |
| `LiquidGlassPasswordBox` | `ContentControl` | Glass wrapper around a native `PasswordBox`. |
| `LiquidGlassComboBox` | `ComboBox` | Combo box with focus-driven glass optics. |
| `LiquidGlassToggleSwitch` | `ToggleButton` | Switch-style toggle using the kube Lip-profile knob geometry. |
| `LiquidGlassTabBar` | `ListView` | Horizontal selector that generates `LiquidGlassTabBarItem` containers. |
| `LiquidGlassTabBarItem` | `ListViewItem` | Selectable glass tab item. |
| `LiquidGlassFloatingPanel` | `ContentControl` | Floating glass content surface. |
| `LiquidGlassSearchBox` | `ContentControl` | Glass shell around a native `AutoSuggestBox`; forwards the common search APIs. |

## Common `GlassBrush` property

Every control exposes:

```text
WinUI.Composition.Hlsl.LiquidGlassBrush GlassBrush
static DependencyProperty GlassBrushProperty { get; }
```

### Property value

A `LiquidGlassBrush` containing the live `LiquidGlassMaterial` used by the control.

### Remarks

Changing `GlassBrush` replaces the control's optical material. Transient interaction state is migrated to the new material by the interaction helpers instead of permanently modifying the authored brush values.

The brush is a normal WinUI dependency property, but high-frequency pointer state is not. Pointer position, velocity, and the SDF interaction field are written directly to the live Composition effect so pointer motion does not repeatedly invalidate XAML dependency properties.

### Example

```xml
<liquid:LiquidGlassButton Content="Action">
    <liquid:LiquidGlassButton.GlassBrush>
        <hlsl:LiquidGlassBrush
            CornerRadius="8"
            BlurRadius="1"
            RefractionStrength="18"
            BezelWidth="12"
            GlassThickness="48"
            RefractiveIndex="1.45" />
    </liquid:LiquidGlassButton.GlassBrush>
</liquid:LiquidGlassButton>
```

## LiquidGlassCard

### Definition

```text
runtimeclass LiquidGlassCard : Microsoft.UI.Xaml.Controls.ContentControl
```

### Remarks

The default material uses the `Panel` optical preset. The default XAML `CornerRadius` is 31 so the WinUI clip/border geometry and shader SDF use the same authored radius.

Use `Content`, `ContentTemplate`, `Padding`, and the other inherited `ContentControl` properties normally.

## LiquidGlassMagnifier

### Definition

```text
runtimeclass LiquidGlassMagnifier : Microsoft.UI.Xaml.Controls.ContentControl
```

### Default geometry

- Width: 210
- Height: 150
- Corner radius: 75
- Magnification: 24 at rest, 48 while dragging
- Bezel width: 25
- Glass thickness: 110
- Refractive index: 1.5

### Remarks

The lens captures the pointer while dragging. Drag displacement is measured in the stable `XamlRoot.Content` coordinate space; the lens never integrates pointer deltas measured relative to the element being translated. This prevents transform/pointer-coordinate feedback jitter.

Horizontal drag velocity drives the kube-style stretch/compression response independently from drag position.

## LiquidGlassButton

### Definition

```text
runtimeclass LiquidGlassButton : Microsoft.UI.Xaml.Controls.Button
```

### Remarks

`LiquidGlassButton` retains `Button` click, command, keyboard, focus, pointer capture, and UI Automation behavior. Its default style is based on WinUI's `DefaultButtonStyle` for typography, padding, focus metrics, and inherited control behavior. Pointer-over and pressed visuals are overlays over the glass surface; state transitions do not replace `GlassBrush` with an opaque theme brush.

All inherited `Button` events and properties remain available, including `Click`, `Command`, and `CommandParameter`.

## LiquidGlassToggleButton

### Definition

```text
runtimeclass LiquidGlassToggleButton : Microsoft.UI.Xaml.Controls.Primitives.ToggleButton
```

### Remarks

The template follows WinUI `ToggleButton`'s combined state contract: `Normal`, `PointerOver`, `Pressed`, `Disabled`, `Checked`, `CheckedPointerOver`, `CheckedPressed`, `CheckedDisabled`, and the corresponding indeterminate states. Persistent checked optics are recomputed before pointer-over and pressed deltas.

Use inherited `IsChecked`, `Checked`, `Unchecked`, and `Indeterminate` APIs.

## LiquidGlassHyperlinkButton

### Definition

```text
runtimeclass LiquidGlassHyperlinkButton : Microsoft.UI.Xaml.Controls.HyperlinkButton
```

### Remarks

Uses WinUI `HyperlinkButton` semantics and default style metrics while preserving the glass surface through pointer visual states. Inherited navigation/click APIs behave as on `HyperlinkButton`.

## LiquidGlassCheckBox

### Definition

```text
runtimeclass LiquidGlassCheckBox : Microsoft.UI.Xaml.Controls.CheckBox
```

### Remarks

The control keeps the native CheckBox state names and inherited tri-state behavior. Only the 20×20 choice surface is glass; the content label is not painted as a large glass rectangle. This matches WinUI's native two-column choice layout more closely and avoids applying the optical SDF to unrelated text content.

Use inherited `IsChecked`, `IsThreeState`, `Checked`, `Unchecked`, and `Indeterminate` APIs.

## LiquidGlassRadioButton

### Definition

```text
runtimeclass LiquidGlassRadioButton : Microsoft.UI.Xaml.Controls.RadioButton
```

### Remarks

The circular 20×20 selector is the glass surface. Grouping, keyboard navigation, checked state, automation, and the inherited `GroupName` behavior remain native WinUI behavior.

## LiquidGlassSlider

### Definition

```text
runtimeclass LiquidGlassSlider : Microsoft.UI.Xaml.Controls.Slider
```

### Remarks

Slider value calculation, keyboard input, pointer manipulation, tick behavior, focus engagement, and automation remain WinUI `Slider` behavior. The template's actual `Thumb` is resized to the kube optical geometry rather than drawing a second disconnected visual:

- 90×60 for a horizontal slider.
- 60×90 for a vertical slider.
- Corner radius 30.
- Rest scale 0.6; pressed/drag scale 1.0.

The glass material is attached to the real template `Thumb`, so the hit-test/value mechanics and the optical visual move together.

## LiquidGlassTextBox

### Definition

```text
runtimeclass LiquidGlassTextBox : Microsoft.UI.Xaml.Controls.TextBox
```

### Remarks

Exposes the complete inherited `TextBox` API. Focus changes animate optical properties through Composition. Normal text editing, selection, clipboard, IME, accessibility, and keyboard semantics are not reimplemented by the library.

## LiquidGlassPasswordBox

### Definition

```text
runtimeclass LiquidGlassPasswordBox : Microsoft.UI.Xaml.Controls.ContentControl
```

### Properties

| Property | Type | Description |
| --- | --- | --- |
| `PlaceholderText` | `String` | Gets or sets the placeholder text of the inner native `PasswordBox`. |
| `Password` | `String` | Gets or sets the password value of the inner native `PasswordBox`. |

### Remarks

The runtimeclass is a glass wrapper rather than a subclass of WinUI's sealed/internal password implementation. Password editing remains delegated to the native `PasswordBox`.

## LiquidGlassComboBox

### Definition

```text
runtimeclass LiquidGlassComboBox : Microsoft.UI.Xaml.Controls.ComboBox
```

### Remarks

Selection, item containers, flyout behavior, keyboard navigation, and automation are inherited from WinUI `ComboBox`. The material adds focus-driven optical changes without replacing selection semantics.

## LiquidGlassToggleSwitch

### Definition

```text
runtimeclass LiquidGlassToggleSwitch : Microsoft.UI.Xaml.Controls.Primitives.ToggleButton
```

### Properties

| Property | Type | Description |
| --- | --- | --- |
| `Header` | `Object` | Gets or sets the switch label/content. |
| `IsOn` | `Boolean` | Gets or sets the Boolean switch state. Maps to inherited `IsChecked`. |

### Default geometry

The default visual geometry follows the kube reference:

- Track: 160×67.
- Optical knob: 146×92 with radius 46.
- Surface profile: `Lip`.
- Rest scale: 0.65.
- Pressed scale: 0.9.
- Bezel width: 19.
- Glass thickness: 47.
- Refractive index: 1.5.

### Remarks

The runtimeclass derives from `ToggleButton`; consequently its template uses the native ToggleButton combined checked/pointer state names rather than a separate custom state machine. This keeps `Checked`, `Unchecked`, keyboard activation, and automation behavior aligned with WinUI.

## LiquidGlassTabBar and LiquidGlassTabBarItem

### Definitions

```text
runtimeclass LiquidGlassTabBar : Microsoft.UI.Xaml.Controls.ListView
runtimeclass LiquidGlassTabBarItem : Microsoft.UI.Xaml.Controls.ListViewItem
```

### Remarks

`LiquidGlassTabBar` retains ListView selection, keyboard, focus, item source, virtualization/container behavior where applicable, and generates `LiquidGlassTabBarItem` containers for ordinary items. `SelectedIndex`, `SelectedItem`, `ItemsSource`, and selection events are inherited.

## LiquidGlassFloatingPanel

### Definition

```text
runtimeclass LiquidGlassFloatingPanel : Microsoft.UI.Xaml.Controls.ContentControl
```

### Remarks

A rounded content surface using the `FloatingPanel` preset. The default corner radius is 28 and is kept aligned with the material preset.

## LiquidGlassSearchBox

### Definition

```text
runtimeclass LiquidGlassSearchBox : Microsoft.UI.Xaml.Controls.ContentControl
```

WinUI's native `AutoSuggestBox` is sealed, so this control hosts a real native `AutoSuggestBox` instead of subclassing it. The wrapper mirrors the native search-specific dependency-property surface and keeps the outer and inner property systems synchronized in both directions.

### Properties

| Property | Type | Description |
| --- | --- | --- |
| `MaxSuggestionListHeight` | `Double` | Gets or sets the maximum height of the suggestion list. |
| `IsSuggestionListOpen` | `Boolean` | Gets or sets whether the suggestion list is open. |
| `TextMemberPath` | `String` | Gets or sets the member path used to obtain suggestion display text. |
| `Text` | `String` | Gets or sets the current text. This is a dependency property and supports `Binding` and TwoWay `x:Bind`. |
| `UpdateTextOnSelect` | `Boolean` | Gets or sets whether choosing a suggestion updates `Text`. |
| `PlaceholderText` | `String` | Gets or sets placeholder text. |
| `Header` | `Object` | Gets or sets the search header. |
| `AutoMaximizeSuggestionArea` | `Boolean` | Gets or sets whether WinUI may maximize the suggestion area. |
| `TextBoxStyle` | `Style` | Gets or sets the inner text editor style. A null wrapper value keeps the built-in transparent LiquidGlass editor style. |
| `QueryIcon` | `IconElement` | Gets or sets the query icon. |
| `LightDismissOverlayMode` | `LightDismissOverlayMode` | Gets or sets the light-dismiss overlay behavior for the suggestion popup. |
| `Description` | `Object` | Gets or sets the control description. |
| `HeaderPlacement` | `ControlHeaderPlacement` | Gets or sets the header placement. |
| `ItemsSource` | `Object` | Mirrors inherited `ItemsControl.ItemsSource` on the hosted native control. |
| `ItemTemplate` | `DataTemplate` | Mirrors inherited `ItemsControl.ItemTemplate` on the hosted native control. |
| `InnerAutoSuggestBox` | `AutoSuggestBox` | Gets the hosted native control for APIs that are intentionally not mirrored. Read-only. |

Every mutable property above except `InnerAutoSuggestBox` is backed by a public `<Name>Property` dependency-property identifier on `LiquidGlassSearchBox`.

### Events

| Event | Description |
| --- | --- |
| `SuggestionChosen` | Forwards `AutoSuggestBox.SuggestionChosen` with the native sender and event args. |
| `TextChanged` | Forwards `AutoSuggestBox.TextChanged` with the native sender and event args. |
| `QuerySubmitted` | Forwards `AutoSuggestBox.QuerySubmitted` with the native sender and event args. |

### Remarks

The outer control owns the rounded glass background. The hosted `AutoSuggestBox` remains responsible for text editing, suggestions, keyboard handling, IME, popup behavior, and UI Automation and is kept visually transparent so its stock rectangular background does not cover the glass surface.

Changes made through the wrapper dependency properties are forwarded to the native `AutoSuggestBox`. Changes made by native control behavior, user input, or through `InnerAutoSuggestBox` are mirrored back to the wrapper dependency properties. Reentrancy guards prevent the two property systems from feeding changes back into each other indefinitely.

For example, a TwoWay compiled binding is valid:

```xml
<liquid:LiquidGlassSearchBox
    PlaceholderText="Search"
    Text="{x:Bind ViewModel.Query, Mode=TwoWay}" />
```

## Presets

`LiquidGlassPresets.CreateBrush(LiquidGlassPreset)` creates a new brush configured for one of the built-in control geometries.

```text
Panel
Button
Choice
SearchBox
Input
SliderThumb
ToggleSwitchKnob
Magnifier
FloatingPanel
TabBar
TabBarItem
```

Each call returns an independent brush instance. Modify the returned brush when a control requires a different authored optical surface.

## Example

```xml
<StackPanel Spacing="12"
            xmlns:liquid="using:WinUI.LiquidGlass">
    <liquid:LiquidGlassButton Content="Save" />
    <liquid:LiquidGlassToggleButton Content="Pinned" />
    <liquid:LiquidGlassSearchBox PlaceholderText="Search" />
    <liquid:LiquidGlassSlider Minimum="0" Maximum="100" Value="40" />
</StackPanel>
```

## See also

- [LiquidGlassBrush](liquid-glass-brush.md)
- [LiquidGlassMaterial](liquid-glass-material.md)
- [LiquidGlassInteraction](liquid-glass-interaction.md)
- [API reference](index.md)
