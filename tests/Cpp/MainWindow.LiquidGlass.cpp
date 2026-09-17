#include <algorithm>
#include <cmath>

#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
    void MainWindow::BuildRegressionLab()
    {
        if (m_regressionLabBuilt)
        {
            return;
        }
        m_regressionLabBuilt = true;

        namespace Controls = Microsoft::UI::Xaml::Controls;
        namespace Liquid = WinUI::LiquidGlass;

        auto host = LiquidGlassControls();
        if (!host)
        {
            return;
        }

        Controls::TextBlock heading;
        heading.Margin({ 0.0, 14.0, 0.0, 0.0 });
        heading.FontSize(14.0);
        heading.Text(L"Regression lab");
        host.Children().Append(heading);

        Controls::TextBlock instructions;
        instructions.Opacity(0.68);
        instructions.TextWrapping(TextWrapping::Wrap);
        instructions.Text(
            L"Reproduce Concave + blur, verify Slider endpoints, then detach/reload an active control subtree. "
            L"After interacting with it, close the window to exercise composition teardown.");
        host.Children().Append(instructions);

        Controls::Button concaveButton;
        concaveButton.HorizontalAlignment(HorizontalAlignment::Stretch);
        concaveButton.Content(box_value(L"Load Concave + blur repro"));
        auto weak = get_weak();
        concaveButton.Click([weak](auto const&, auto const&)
        {
            if (auto self = weak.get())
            {
                // Keep this preset deterministic so screenshots from different runs are
                // comparable. Concave plus non-zero blur was the historical black-edge case.
                self->SurfaceProfileSlider().Value(2.0);
                self->BlurRadiusSlider().Value(3.5);
                self->GlassThicknessSlider().Value(96.0);
                self->RefractionStrengthSlider().Value(42.0);
                self->DispersionStrengthSlider().Value(1.2);
                self->MagnificationStrengthSlider().Value(0.0);
                self->ApplyLiquidGlassProperties();
                self->ApplyAdvancedLiquidGlassProperties();
            }
        });
        host.Children().Append(concaveButton);

        Controls::TextBlock sliderLabel;
        sliderLabel.Margin({ 0.0, 8.0, 0.0, 0.0 });
        sliderLabel.Text(L"LiquidGlassSlider endpoints · current 50");
        host.Children().Append(sliderLabel);

        Liquid::LiquidGlassSlider regressionSlider;
        regressionSlider.Minimum(0.0);
        regressionSlider.Maximum(100.0);
        regressionSlider.Value(50.0);
        regressionSlider.HorizontalAlignment(HorizontalAlignment::Stretch);
        host.Children().Append(regressionSlider);

        Controls::StackPanel endpointButtons;
        endpointButtons.Orientation(Controls::Orientation::Horizontal);
        endpointButtons.Spacing(6.0);

        Controls::Button zeroButton;
        zeroButton.Content(box_value(L"0"));
        zeroButton.Click([regressionSlider, sliderLabel](auto const&, auto const&)
        {
            regressionSlider.Value(0.0);
            sliderLabel.Text(L"LiquidGlassSlider endpoints · current 0");
        });
        endpointButtons.Children().Append(zeroButton);

        Controls::Button fiftyButton;
        fiftyButton.Content(box_value(L"50"));
        fiftyButton.Click([regressionSlider, sliderLabel](auto const&, auto const&)
        {
            regressionSlider.Value(50.0);
            sliderLabel.Text(L"LiquidGlassSlider endpoints · current 50");
        });
        endpointButtons.Children().Append(fiftyButton);

        Controls::Button hundredButton;
        hundredButton.Content(box_value(L"100"));
        hundredButton.Click([regressionSlider, sliderLabel](auto const&, auto const&)
        {
            regressionSlider.Value(100.0);
            sliderLabel.Text(L"LiquidGlassSlider endpoints · current 100");
        });
        endpointButtons.Children().Append(hundredButton);
        host.Children().Append(endpointButtons);

        Controls::TextBlock teardownLabel;
        teardownLabel.Margin({ 0.0, 8.0, 0.0, 0.0 });
        teardownLabel.Text(L"Unload / reload stress");
        host.Children().Append(teardownLabel);

        m_regressionStressMount = Controls::ContentControl{};
        Controls::StackPanel stressContent;
        stressContent.Spacing(6.0);

        Liquid::LiquidGlassButton detachButton;
        detachButton.Content(box_value(L"Detach this subtree, then reattach"));
        detachButton.Click([weak](auto const&, auto const&)
        {
            if (auto self = weak.get())
            {
                auto mount = self->m_regressionStressMount;
                if (!mount)
                {
                    return;
                }

                auto content = mount.Content();
                if (!content)
                {
                    return;
                }

                // Removing Content performs a real visual-tree detach and raises Unloaded on
                // the glass controls. Reattach on the next dispatcher turn so the test also
                // covers the matching Loaded path and restoration of retained baselines.
                mount.Content(nullptr);
                auto queue = mount.DispatcherQueue();
                if (!queue || !queue.TryEnqueue([weak, content]
                    {
                        if (auto owner = weak.get())
                        {
                            if (owner->m_regressionStressMount)
                            {
                                owner->m_regressionStressMount.Content(content);
                            }
                        }
                    }))
                {
                    mount.Content(content);
                }
            }
        });
        stressContent.Children().Append(detachButton);

        Liquid::LiquidGlassTextBox focusBox;
        focusBox.PlaceholderText(L"Focus me before detach");
        focusBox.Text(L"Focused optics baseline");
        stressContent.Children().Append(focusBox);

        Liquid::LiquidGlassSlider stressSlider;
        stressSlider.Minimum(0.0);
        stressSlider.Maximum(100.0);
        stressSlider.Value(67.0);
        stressContent.Children().Append(stressSlider);

        Liquid::LiquidGlassToggleSwitch stressSwitch;
        stressSwitch.Header(box_value(L"Switch teardown"));
        stressSwitch.IsOn(true);
        stressContent.Children().Append(stressSwitch);

        m_regressionStressMount.Content(stressContent);
        host.Children().Append(m_regressionStressMount);
    }

    void MainWindow::ApplyAdvancedLiquidGlassProperties()
    {
        if (m_backdropEffect != BackdropEffectKind::LiquidGlass || !m_liquidGlassMaterial)
        {
            return;
        }

        auto const profileIndex = std::clamp(
            static_cast<int32_t>(std::lround(SurfaceProfileSlider().Value())),
            0,
            3);

        m_liquidGlassMaterial.SurfaceProfile(
            static_cast<WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile>(profileIndex));
        m_liquidGlassMaterial.MagnificationStrength(static_cast<float>(MagnificationStrengthSlider().Value()));
        m_liquidGlassMaterial.HighlightSharpness(static_cast<float>(HighlightSharpnessSlider().Value()));
        m_liquidGlassMaterial.SpecularSaturation(static_cast<float>(SpecularSaturationSlider().Value()));
        m_liquidGlassMaterial.SpecularWidth(static_cast<float>(SpecularWidthSlider().Value()));
        m_liquidGlassMaterial.EdgeSoftness(static_cast<float>(EdgeSoftnessSlider().Value()));
        m_liquidGlassMaterial.MaterialOpacity(static_cast<float>(MaterialOpacitySlider().Value()));
        m_liquidGlassMaterial.InnerShadowStrength(static_cast<float>(InnerShadowStrengthSlider().Value()));
        m_liquidGlassMaterial.Contrast(static_cast<float>(ContrastSlider().Value()));
        m_liquidGlassMaterial.Exposure(static_cast<float>(ExposureSlider().Value()));
        m_liquidGlassMaterial.TintRed(static_cast<float>(TintRedSlider().Value()));
        m_liquidGlassMaterial.TintGreen(static_cast<float>(TintGreenSlider().Value()));
        m_liquidGlassMaterial.TintBlue(static_cast<float>(TintBlueSlider().Value()));
    }

    void MainWindow::OnAdvancedLiquidGlassLoaded(
        Windows::Foundation::IInspectable const&,
        RoutedEventArgs const&)
    {
        BuildRegressionLab();

        if (m_advancedLiquidGlassWired)
        {
            ApplyAdvancedLiquidGlassProperties();
            return;
        }
        m_advancedLiquidGlassWired = true;

        auto weak = get_weak();
        auto const onRangeChanged = [weak](auto const&, auto const&)
            {
                if (auto self = weak.get())
                {
                    self->ApplyAdvancedLiquidGlassProperties();
                }
            };

        SurfaceProfileSlider().ValueChanged(onRangeChanged);
        MagnificationStrengthSlider().ValueChanged(onRangeChanged);
        HighlightSharpnessSlider().ValueChanged(onRangeChanged);
        SpecularSaturationSlider().ValueChanged(onRangeChanged);
        SpecularWidthSlider().ValueChanged(onRangeChanged);
        EdgeSoftnessSlider().ValueChanged(onRangeChanged);
        MaterialOpacitySlider().ValueChanged(onRangeChanged);
        InnerShadowStrengthSlider().ValueChanged(onRangeChanged);
        ContrastSlider().ValueChanged(onRangeChanged);
        ExposureSlider().ValueChanged(onRangeChanged);
        TintRedSlider().ValueChanged(onRangeChanged);
        TintGreenSlider().ValueChanged(onRangeChanged);
        TintBlueSlider().ValueChanged(onRangeChanged);

        // XAML registered OnEffectSelectionChanged first. This second handler therefore runs
        // after ApplyBackdropEffect recreated the material and restores every advanced value.
        EffectSelector().SelectionChanged([weak](auto const&, auto const&)
            {
                if (auto self = weak.get())
                {
                    self->ApplyAdvancedLiquidGlassProperties();
                }
            });

        ApplyAdvancedLiquidGlassProperties();
    }
}
