# Navigation lifetime lab

The C++ test executable now starts `NavigationWindow` instead of the original single-surface playground. The window intentionally keeps the shell mounted while replacing `Frame` content:

- **Overview** shows the recommended structural-surface pattern.
- **Shared brush** uses one `LiquidGlassBrush` from the root `Grid.Resources`; the shell `NavigationView` keeps that resource alive during page changes.
- **Isolated brushes** creates one brush per page surface for comparison.
- **Detach / reattach** removes a live subtree and restores it on the next dispatcher turn.
- **Stress 40 cycles** changes the selected page every 120 ms.

The material switch starts disabled. This makes the navigation path testable before enabling the native effect. The test is deliberately a comparison harness, not a claim that every application surface should use isolated brushes.

## Suggested Snap.Hutao migration

1. Keep semantic resources such as `HutaoSurfaceBrush`, but default every liquid brush to `IsEnabled="False"`.
2. Include every liquid resource, including `HutaoGuideLiquidGlassBrush`, in the material switch.
3. First apply glass only to structural surfaces that remain mounted with the shell. Avoid replacing `SettingsCardBackground*` and `CommandBarBackground*` globally until navigation stress is clean.
4. Use per-control brushes only for controls that animate optical properties independently; do not create a new effect graph for every list/card item by default.
5. Run this lab with the shared and isolated pages separately. If isolated page teardown fails while the shared page remains stable, investigate the brush/package teardown path before changing Snap resource ownership.
