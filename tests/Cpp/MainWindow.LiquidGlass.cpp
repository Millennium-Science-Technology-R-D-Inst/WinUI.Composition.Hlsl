#include <algorithm>
#include <cmath>

#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
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
