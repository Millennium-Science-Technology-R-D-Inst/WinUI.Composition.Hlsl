#include <algorithm>
#include <cmath>

#include "XamlWorkaround.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
    void MainWindow::VerifyToggleSwitchLayout()
    {
        namespace Controls = Microsoft::UI::Xaml::Controls;
        using Switch = WinUI::LiquidGlass::LiquidGlassToggleSwitch;
        auto control = GalleryToggleSwitch();
        auto require = [](bool condition, wchar_t const* message)
        {
            if (!condition) throw hresult_error(E_FAIL, message);
        };
        auto findPart = [](auto&& findPart, DependencyObject const& root, hstring const& name) -> DependencyObject
        {
            if (auto element = root.try_as<FrameworkElement>(); element && element.Name() == name) return root;
            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int index = 0; index < count; ++index)
            {
                if (auto part = findPart(findPart, Media::VisualTreeHelper::GetChild(root, index), name)) return part;
            }
            return nullptr;
        };
        Controls::ToggleSwitch native;
        require(unbox_value<hstring>(control.OnContent()) == unbox_value<hstring>(native.OnContent()), L"Default OnContent must use WinUI localization.");
        require(unbox_value<hstring>(control.OffContent()) == unbox_value<hstring>(native.OffContent()), L"Default OffContent must use WinUI localization.");
        require(control.ReadLocalValue(Switch::OnContentProperty()) == DependencyProperty::UnsetValue(), L"Localized defaults must remain overridable by styles.");
        require(std::abs(control.SwitchScale() - SwitchScaleSlider().Value()) < 1e-6, L"SwitchScale XAML binding must resolve before an imperative setter registers the DP.");
        require(std::isnan(control.Height()), L"Default outer height must be Auto so Header can participate in layout.");

        control.ApplyTemplate();
        control.UpdateLayout();
        auto viewbox = findPart(findPart, control, L"SwitchGlyphViewbox").as<Controls::Viewbox>();
        auto track = findPart(findPart, control, L"Track").as<FrameworkElement>();
        auto knob = findPart(findPart, control, L"SwitchKnob").as<FrameworkElement>();
        auto header = findPart(findPart, control, L"HeaderContentPresenter").as<Controls::ContentPresenter>();
        auto on = findPart(findPart, control, L"OnContentPresenter").as<Controls::ContentPresenter>();
        auto off = findPart(findPart, control, L"OffContentPresenter").as<Controls::ContentPresenter>();
        auto const originalHeader = control.Header();
        auto const headerHeight = header.ActualHeight();
        auto const headerTop = header.TransformToVisual(control).TransformPoint({ 0.0f, 0.0f }).Y;
        auto const trackTop = track.TransformToVisual(control).TransformPoint({ 0.0f, 0.0f }).Y;
        require(header.Visibility() == Visibility::Visible && headerTop + headerHeight <= trackTop + 1.0, L"Header must be above the glyph.");

        for (double scale : { 0.5, 1.0, 1.5 })
        {
            control.SwitchScale(scale);
            control.UpdateLayout();
            require(std::abs(viewbox.ActualHeight() - 44.0 * scale) <= 1.0, L"Runtime scale must resize the glyph layout box.");
            require(track.Width() == 160.0 && track.Height() == 67.0 && knob.Width() == 146.0 && knob.Height() == 92.0, L"Scaling must preserve authored material geometry.");
            require(std::abs(header.ActualHeight() - headerHeight) <= 1.0, L"SwitchScale must not scale Header text.");
            auto const transform = track.TransformToVisual(control);
            auto const origin = transform.TransformPoint({ 0.0f, 0.0f });
            auto const x = transform.TransformPoint({ 160.0f, 0.0f });
            auto const y = transform.TransformPoint({ 0.0f, 67.0f });
            require(std::abs(std::abs(x.X - origin.X) / 160.0 - std::abs(y.Y - origin.Y) / 67.0) < 0.01, L"Track must scale uniformly rather than stretch its shape.");
        }

        control.Width(480.0);
        control.Height(160.0);
        control.UpdateLayout();
        require(std::abs(viewbox.ActualHeight() - 66.0) <= 1.0, L"Outer Width/Height must not change the glyph size.");
        control.ClearValue(FrameworkElement::WidthProperty());
        control.ClearValue(FrameworkElement::HeightProperty());
        control.Header(nullptr);
        require(header.Visibility() == Visibility::Collapsed, L"A null Header must not reserve a row.");
        control.HeaderTemplate(Markup::XamlReader::Load(LR"(<DataTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"><TextBlock Text="Template-only header" /></DataTemplate>)").as<DataTemplate>());
        require(header.Visibility() == Visibility::Visible, L"HeaderTemplate alone must show the header row.");
        control.HeaderTemplate(nullptr);
        require(header.Visibility() == Visibility::Collapsed, L"Clearing HeaderTemplate must remove an empty header row.");
        control.Header(originalHeader);

        control.OnContent(box_value(L"A longer enabled label"));
        control.OffContent(box_value(L"Off"));
        control.IsOn(false);
        control.UpdateLayout();
        auto const offWidth = control.DesiredSize().Width;
        require(off.Opacity() == 1.0 && on.Opacity() == 0.0, L"Off state must show OffContent.");
        control.IsOn(true);
        control.UpdateLayout();
        require(std::abs(control.DesiredSize().Width - offWidth) <= 1.0, L"Changing state must not change the measured label width.");
        require(off.Opacity() == 0.0 && on.Opacity() == 1.0, L"On state must show OnContent.");
        control.IsEnabled(false);
        require(on.Opacity() == 1.0, L"Disabled checked state must retain OnContent.");
        control.IsEnabled(true);
        control.ClearValue(Switch::OnContentProperty());
        control.ClearValue(Switch::OffContentProperty());
        control.SwitchScale(1.0);
    }

    winrt::fire_and_forget MainWindow::ShowToggleSwitchPopupRegressionAsync()
    {
        auto lifetime = get_strong();

        namespace Controls = Microsoft::UI::Xaml::Controls;
        namespace Liquid = WinUI::LiquidGlass;

        Controls::ContentDialog dialog;
        dialog.XamlRoot(Root().XamlRoot());
        dialog.Title(box_value(L"ContentDialog switch lifecycle regression"));
        dialog.CloseButtonText(L"Close");

        Controls::StackPanel panel;
        panel.Spacing(12.0);

        Controls::TextBlock instruction;
        instruction.TextWrapping(TextWrapping::Wrap);
        instruction.Text(L"Switch between On and Off repeatedly. Compare the live knob/track with the bound IsOn value, then close and reopen this dialog.");
        panel.Children().Append(instruction);

        Controls::TextBlock status;
        status.Text(L"Glass IsOn=true. Watch for a delayed Unloaded event.");
        panel.Children().Append(status);

        Liquid::LiquidGlassToggleSwitch glass;
        glass.Header(box_value(L"Liquid glass"));
        glass.IsOn(true);
        glass.Loaded([status](auto const&, auto const&)
        {
            status.Text(L"Glass Loaded: ready for interaction.");
        });
        glass.Unloaded([status](auto const& sender, auto const&)
        {
            auto element = sender.try_as<FrameworkElement>();
            if (element && element.IsLoaded())
            {
                status.Text(L"Delayed Unloaded while IsLoaded=true: live visual must be preserved.");
            }
        });
        glass.RegisterPropertyChangedCallback(Liquid::LiquidGlassToggleSwitch::IsOnProperty(),
            [status](DependencyObject const& sender, DependencyProperty const&)
            {
                auto control = sender.as<Liquid::LiquidGlassToggleSwitch>();
                status.Text(control.IsOn()
                    ? L"Glass IsOn=true: knob must be right, track green."
                    : L"Glass IsOn=false: knob must be left, track gray.");
            });
        panel.Children().Append(glass);

        Controls::ToggleSwitch native;
        native.Header(box_value(L"Native WinUI comparison"));
        native.IsOn(true);
        panel.Children().Append(native);

        dialog.Content(panel);
        co_await dialog.ShowAsync();
    }

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

        Controls::Button popupSwitchButton;
        popupSwitchButton.HorizontalAlignment(HorizontalAlignment::Stretch);
        popupSwitchButton.Content(box_value(L"Open ContentDialog toggle switch regression"));
        popupSwitchButton.Click([weak = get_weak()](auto const&, auto const&)
        {
            if (auto self = weak.get())
            {
                self->ShowToggleSwitchPopupRegressionAsync();
            }
        });
        host.Children().Append(popupSwitchButton);

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

        // The direct Composition material does not know which XamlRoot owns it. Keep
        // the playground's logical DIP authoring aligned with the custom sampler's
        // raster-pixel coordinate field, including after a per-monitor DPI transition.
        if (auto root = BackdropHost().XamlRoot())
        {
            m_liquidGlassMaterial.RasterizationScale(
                static_cast<float>(std::max(root.RasterizationScale(), 0.01)));
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

        // Keep the direct material in sync if the window crosses monitors with different
        // rasterization scales. The weak owner avoids extending the window lifetime.
        if (auto root = BackdropHost().XamlRoot())
        {
            root.Changed([weak](auto const&, auto const&)
            {
                if (auto self = weak.get())
                {
                    self->ApplyAdvancedLiquidGlassProperties();
                }
            });
        }

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
