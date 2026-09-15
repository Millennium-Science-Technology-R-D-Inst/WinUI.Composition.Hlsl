#include "pch.h"
#include "winrt_module_imports.h"
#include "LiquidGlassControls.h"

#if __has_include("LiquidGlassCard.g.cpp")
#include "LiquidGlassCard.g.cpp"
#include "LiquidGlassMagnifier.g.cpp"
#include "LiquidGlassButton.g.cpp"
#include "LiquidGlassToggleButton.g.cpp"
#include "LiquidGlassHyperlinkButton.g.cpp"
#include "LiquidGlassCheckBox.g.cpp"
#include "LiquidGlassRadioButton.g.cpp"
#include "LiquidGlassSlider.g.cpp"
#include "LiquidGlassTextBox.g.cpp"
#include "LiquidGlassPasswordBox.g.cpp"
#include "LiquidGlassComboBox.g.cpp"
#include "LiquidGlassToggleSwitch.g.cpp"
#endif

namespace winrt::WinUI::LiquidGlass::implementation
{
    using namespace std::chrono_literals;
    namespace
    {
        namespace Xaml = Microsoft::UI::Xaml;
        namespace Controls = Xaml::Controls;
        namespace Primitives = Controls::Primitives;
        namespace Hosting = Xaml::Hosting;
        namespace Media = Xaml::Media;
        namespace Shapes = Xaml::Shapes;

        using Brush = WinUI::Composition::Hlsl::LiquidGlassBrush;
        using Profile = WinUI::Composition::Hlsl::LiquidGlassSurfaceProfile;
        using Xaml::DependencyObject;
        using Xaml::FrameworkElement;

        enum class Preset { Panel, Button, Choice, Search, Input, Slider, Switch, Magnifier };

        Media::Brush AsBrush(Brush const& value)
        {
            return value ? value.as<Media::Brush>() : Media::Brush{ nullptr };
        }

        Brush CreateBrush(Preset preset)
        {
            Brush b;
            b.SurfaceProfile(Profile::ConvexSquircle);
            b.BlurRadius(1.0);
            b.Saturation(1.0);
            b.Contrast(1.0);
            b.Exposure(0.0);
            b.MaterialOpacity(1.0);
            b.EdgeSoftness(1.0);

            switch (preset)
            {
            case Preset::Panel:
                b.CornerRadius(31); b.RefractionStrength(24); b.DispersionStrength(.45);
                b.BezelWidth(29); b.GlassThickness(90); b.RefractiveIndex(1.3);
                b.HighlightStrength(.4); b.HighlightSharpness(1.6); b.SpecularSaturation(6);
                b.SpecularWidth(1); b.TintOpacity(.12); b.InnerShadowStrength(.05);
                b.FallbackColor({ 0x55, 0xff, 0xff, 0xff }); break;
            case Preset::Button:
            case Preset::Choice:
                b.CornerRadius(preset == Preset::Choice ? 10 : 8);
                b.RefractionStrength(18); b.DispersionStrength(.55);
                b.BezelWidth(preset == Preset::Choice ? 9 : 12);
                b.GlassThickness(preset == Preset::Choice ? 32 : 48);
                b.RefractiveIndex(1.45); b.HighlightStrength(.4); b.HighlightSharpness(1.8);
                b.SpecularSaturation(5); b.SpecularWidth(1); b.TintOpacity(.12);
                b.InnerShadowStrength(.06); b.FallbackColor({ 0x44, 0xff, 0xff, 0xff }); break;
            case Preset::Search:
                b.CornerRadius(28); b.RefractionStrength(16.8); b.DispersionStrength(.35);
                b.BezelWidth(27); b.GlassThickness(70); b.RefractiveIndex(1.5);
                b.HighlightStrength(.2); b.HighlightSharpness(1.7); b.SpecularSaturation(4);
                b.SpecularWidth(1); b.TintOpacity(.05); b.InnerShadowStrength(.04);
                b.FallbackColor({ 0x30, 0xff, 0xff, 0xff }); break;
            case Preset::Input:
                b.CornerRadius(8); b.RefractionStrength(16); b.DispersionStrength(.4);
                b.BezelWidth(14); b.GlassThickness(60); b.RefractiveIndex(1.45);
                b.HighlightStrength(.3); b.HighlightSharpness(1.8); b.SpecularSaturation(4);
                b.SpecularWidth(1); b.TintOpacity(.10); b.InnerShadowStrength(.05);
                b.FallbackColor({ 0x36, 0xff, 0xff, 0xff }); break;
            case Preset::Slider:
                b.CornerRadius(30); b.BlurRadius(0); b.RefractionStrength(9.6);
                b.DispersionStrength(.45); b.BezelWidth(16); b.GlassThickness(80);
                b.RefractiveIndex(1.45); b.HighlightStrength(.4); b.HighlightSharpness(1.7);
                b.SpecularSaturation(7); b.SpecularWidth(1); b.TintOpacity(1);
                b.InnerShadowStrength(.05); b.FallbackColor({ 0xff, 0xff, 0xff, 0xff }); break;
            case Preset::Switch:
                b.SurfaceProfile(Profile::Lip); b.CornerRadius(46); b.BlurRadius(.2);
                b.RefractionStrength(9.6); b.DispersionStrength(.45); b.BezelWidth(19);
                b.GlassThickness(47); b.RefractiveIndex(1.5); b.HighlightStrength(.5);
                b.HighlightSharpness(1.6); b.SpecularSaturation(6); b.SpecularWidth(1);
                b.TintOpacity(1); b.InnerShadowStrength(.02);
                b.FallbackColor({ 0xff, 0xff, 0xff, 0xff }); break;
            case Preset::Magnifier:
                b.CornerRadius(75); b.BlurRadius(0); b.RefractionStrength(19.2);
                b.DispersionStrength(.55); b.BezelWidth(25); b.GlassThickness(110);
                b.RefractiveIndex(1.5); b.MagnificationStrength(24); b.HighlightStrength(.5);
                b.HighlightSharpness(1.6); b.SpecularSaturation(9); b.SpecularWidth(1);
                b.TintOpacity(.02); b.InnerShadowStrength(.20);
                b.FallbackColor({ 0x28, 0xff, 0xff, 0xff }); break;
            }
            return b;
        }

        DependencyObject NamedDescendant(DependencyObject const& root, std::wstring_view name)
        {
            if (!root) return nullptr;
            if (auto e = root.try_as<FrameworkElement>(); e && e.Name() == name) return root;
            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int32_t i = 0; i < count; ++i)
                if (auto r = NamedDescendant(Media::VisualTreeHelper::GetChild(root, i), name)) return r;
            return nullptr;
        }

        template<typename T>
        T Descendant(DependencyObject const& root)
        {
            if (!root) return nullptr;
            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int32_t i = 0; i < count; ++i)
            {
                auto child = Media::VisualTreeHelper::GetChild(root, i);
                if (auto r = child.try_as<T>()) return r;
                if (auto r = Descendant<T>(child)) return r;
            }
            return nullptr;
        }

        DependencyObject BrushSurface(DependencyObject const& root)
        {
            if (!root) return nullptr;
            auto const count = Media::VisualTreeHelper::GetChildrenCount(root);
            for (int32_t i = 0; i < count; ++i)
                if (auto r = BrushSurface(Media::VisualTreeHelper::GetChild(root, i))) return r;
            if (root.try_as<Shapes::Shape>() || root.try_as<Controls::Border>() ||
                root.try_as<Primitives::Thumb>() || root.try_as<Controls::Control>()) return root;
            return nullptr;
        }

        void SetSurface(DependencyObject const& target, Media::Brush const& brush)
        {
            if (auto s = target.try_as<Shapes::Shape>()) s.Fill(brush);
            else if (auto b = target.try_as<Controls::Border>()) b.Background(brush);
            else if (auto c = target.try_as<Controls::Control>()) c.Background(brush);
        }

        void SetElementScale(FrameworkElement const& target, double x, double y)
        {
            detail::SetElementScale(target, x, y);
        }

        void AnimateScale(FrameworkElement const& target, double x, double y, double durationMs)
        {
            if (!target) return;
            auto owner = target.try_as<DependencyObject>();
            if (!owner) return;
            detail::AnimateElementScale(owner, target, x, y, durationMs);
        }

        void AnimateScale(FrameworkElement const& target, double value, double durationMs)
        {
            AnimateScale(target, value, value, durationMs);
        }

        Primitives::Thumb SliderThumb(Controls::Slider const& slider)
        {
            auto name = slider.Orientation() == Controls::Orientation::Horizontal
                ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = NamedDescendant(slider, name).try_as<Primitives::Thumb>();
            return thumb ? thumb : Descendant<Primitives::Thumb>(slider);
        }
    }

    LiquidGlassCard::LiquidGlassCard()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Panel));
    }

    LiquidGlassButton::LiquidGlassButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Button));
    }

    LiquidGlassToggleButton::LiquidGlassToggleButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Button));
    }

    LiquidGlassHyperlinkButton::LiquidGlassHyperlinkButton()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Button));
    }

    LiquidGlassCheckBox::LiquidGlassCheckBox()
    {
        GlassBrush(CreateBrush(Preset::Choice));
    }

    LiquidGlassRadioButton::LiquidGlassRadioButton()
    {
        GlassBrush(CreateBrush(Preset::Choice));
    }

    LiquidGlassComboBox::LiquidGlassComboBox()
    {
        GlassBrush(CreateBrush(Preset::Choice));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.01));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.04));
    }

    LiquidGlassTextBox::LiquidGlassTextBox()
    {
        GlassBrush(CreateBrush(Preset::Search));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.98));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.15));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
    }

    void LiquidGlassPasswordBox::ApplyGlassBrush(Brush const& value)
    {
        m_glassBrush = value;
        if (m_passwordBox) m_passwordBox.Background(AsBrush(value));
    }

    LiquidGlassPasswordBox::LiquidGlassPasswordBox()
    {
        m_passwordBox = Controls::PasswordBox{};
        m_passwordBox.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
        HorizontalContentAlignment(Xaml::HorizontalAlignment::Stretch);
        Content(m_passwordBox);
        GlassBrush(CreateBrush(Preset::Input));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.99));
        SetValue(LiquidGlassInteraction::FocusedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::FocusedTintBoostProperty(), box_value(.04));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(140.0));
    }

    hstring LiquidGlassPasswordBox::PlaceholderText() const { return m_passwordBox ? m_passwordBox.PlaceholderText() : hstring{}; }
    void LiquidGlassPasswordBox::PlaceholderText(hstring const& value) { if (m_passwordBox) m_passwordBox.PlaceholderText(value); }
    hstring LiquidGlassPasswordBox::Password() const { return m_passwordBox ? m_passwordBox.Password() : hstring{}; }
    void LiquidGlassPasswordBox::Password(hstring const& value) { if (m_passwordBox) m_passwordBox.Password(value); }

    LiquidGlassMagnifier::LiquidGlassMagnifier()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Magnifier));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.8));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(130.0));
        SetValue(LiquidGlassInteraction::ElasticityProperty(), box_value(.85));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(1.25));
        SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.07));
        SetValue(LiquidGlassInteraction::ActiveMagnificationMultiplierProperty(), box_value(2.0));

        RenderTransformOrigin({ .5f, .5f });
        m_dragTransform = Media::CompositeTransform{};
        RenderTransform(m_dragTransform);
        Loaded([](auto const& sender, auto const&) {
            auto owner = sender.template try_as<DependencyObject>();
            auto element = sender.template try_as<FrameworkElement>();
            if (!owner || !element) return;
            auto const scale = std::clamp(LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            SetElementScale(element, scale, scale);
        });

        auto weak = get_weak();
        PointerPressed([weak](auto const& sender, Xaml::Input::PointerRoutedEventArgs const& e) {
            auto self = weak.get();
            auto owner = sender.template try_as<DependencyObject>();
            auto element = sender.template try_as<Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !owner || !element || !frameworkElement) return;
            auto p = e.GetCurrentPoint(element);
            self->m_activePointerId = p.PointerId(); self->m_lastPointer = p.Position();
            self->m_dragging = element.CapturePointer(e.Pointer());
            if (!self->m_dragging) return;
            if (auto b = self->GlassBrush())
            {
                self->m_dragMagnification = b.MagnificationStrength();
                detail::EnterPressedOptics(owner, b, self->m_dragOptics);
                b.MagnificationStrength(std::clamp(
                    self->m_dragMagnification * LiquidGlassInteraction::GetActiveMagnificationMultiplier(owner), 0.0, 128.0));
            }
            auto const scale = std::clamp(LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            AnimateScale(frameworkElement, scale, LiquidGlassInteraction::GetMotionDuration(owner));
            e.Handled(true);
        });
        PointerMoved([weak](auto const& sender, Xaml::Input::PointerRoutedEventArgs const& e) {
            auto self = weak.get();
            auto owner = sender.template try_as<DependencyObject>();
            auto element = sender.template try_as<Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !self->m_dragging || !owner || !element || !frameworkElement) return;
            auto p = e.GetCurrentPoint(element); if (p.PointerId() != self->m_activePointerId) return;
            auto pos = p.Position(); auto dx = pos.X - self->m_lastPointer.X; auto dy = pos.Y - self->m_lastPointer.Y;
            self->m_dragTransform.TranslateX(self->m_dragTransform.TranslateX() + dx);
            self->m_dragTransform.TranslateY(self->m_dragTransform.TranslateY() + dy);

            auto const distance = std::sqrt(dx * dx + dy * dy);
            auto const elasticity = std::clamp(LiquidGlassInteraction::GetElasticity(owner), 0.0, 1.0);
            auto const amount = std::min(.32, elasticity * distance / 70.0);
            auto const base = std::clamp(LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
            if (distance > 1e-4)
            {
                auto const nx = std::abs(dx) / distance;
                auto const ny = std::abs(dy) / distance;
                auto sx = base * (1.0 + amount * nx);
                auto sy = base * (1.0 + amount * ny);
                if (nx > ny) sy *= 1.0 - amount * .45;
                else sx *= 1.0 - amount * .45;
                SetElementScale(frameworkElement, sx, sy);
            }
            self->m_lastPointer = pos; e.Handled(true);
        });
        PointerReleased([weak](auto const& sender, Xaml::Input::PointerRoutedEventArgs const& e) {
            auto self = weak.get();
            auto owner = sender.template try_as<DependencyObject>();
            auto element = sender.template try_as<Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !self->m_dragging || !owner || !element || !frameworkElement) return;
            element.ReleasePointerCapture(e.Pointer()); self->m_dragging = false; self->m_activePointerId = 0;
            if (self->m_dragOptics.brush) self->m_dragOptics.brush.MagnificationStrength(self->m_dragMagnification);
            detail::LeavePressedOptics(self->m_dragOptics);
            auto const scale = std::clamp(LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            AnimateScale(frameworkElement, scale, LiquidGlassInteraction::GetMotionDuration(owner)); e.Handled(true);
        });
        PointerCaptureLost([weak](auto const& sender, auto const&) {
            auto self = weak.get();
            auto owner = sender.template try_as<DependencyObject>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !owner || !frameworkElement) return;
            self->m_dragging = false; self->m_activePointerId = 0;
            if (self->m_dragOptics.brush) self->m_dragOptics.brush.MagnificationStrength(self->m_dragMagnification);
            detail::LeavePressedOptics(self->m_dragOptics);
            auto const scale = std::clamp(LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
            AnimateScale(frameworkElement, scale, LiquidGlassInteraction::GetMotionDuration(owner));
        });
    }

    void LiquidGlassSlider::ApplyGlassBrush(Brush const& value)
    {
        m_glassBrush = value;
        Background(AsBrush(value));
        if (m_thumb)
            if (auto surface = BrushSurface(m_thumb)) SetSurface(surface, AsBrush(value));
    }

    LiquidGlassSlider::LiquidGlassSlider()
    {
        GlassBrush(CreateBrush(Preset::Slider));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.6));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(120.0));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
        SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.9));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(0.0));

        auto weak = get_weak();
        Loaded([weak](auto const& sender, auto const&) {
            auto self = weak.get(); if (!self || self->m_interactionsWired) return;
            auto slider = sender.template try_as<Controls::Slider>(); if (!slider) return;
            auto owner = slider.template try_as<DependencyObject>();
            auto thumb = SliderThumb(slider); if (!owner || !thumb) return;
            self->m_thumb = thumb;
            if (auto b = self->GlassBrush())
            {
                auto radius = std::max(1.0, std::min(thumb.ActualWidth(), thumb.ActualHeight()) * .5);
                b.CornerRadius(radius); b.BezelWidth(std::max(1.0, std::min(16.0, radius - .5)));
                if (auto surface = BrushSurface(thumb)) SetSurface(surface, AsBrush(b));
            }
            auto const restScale = std::clamp(LiquidGlassInteraction::GetRestScale(thumb), .25, 4.0);
            SetElementScale(thumb, restScale, restScale);
            thumb.DragStarted([weak](auto const& s, auto const&) {
                auto thumbObject = s.template try_as<DependencyObject>();
                auto thumbElement = s.template try_as<FrameworkElement>();
                auto self = weak.get();
                if (!self || !thumbObject || !thumbElement) return;
                auto const scale = std::clamp(LiquidGlassInteraction::GetPressedScale(thumbObject), .25, 4.0);
                AnimateScale(thumbElement, scale, LiquidGlassInteraction::GetMotionDuration(thumbObject));
                detail::EnterPressedOptics(thumbObject, self->GlassBrush(), self->m_dragOptics);
            });
            thumb.DragCompleted([weak](auto const& s, auto const&) {
                auto thumbObject = s.template try_as<DependencyObject>();
                auto thumbElement = s.template try_as<FrameworkElement>();
                auto self = weak.get();
                if (!self || !thumbObject || !thumbElement) return;
                auto const scale = std::clamp(LiquidGlassInteraction::GetRestScale(thumbObject), .25, 4.0);
                AnimateScale(thumbElement, scale, LiquidGlassInteraction::GetMotionDuration(thumbObject));
                detail::LeavePressedOptics(self->m_dragOptics);
            });
            self->m_interactionsWired = true;
        });
    }

    LiquidGlassToggleSwitch::LiquidGlassToggleSwitch()
    {
        DefaultStyleKey(box_value(xaml_typename<class_type>()));
        GlassBrush(CreateBrush(Preset::Switch));
        SetValue(LiquidGlassInteraction::RestScaleProperty(), box_value(.65));
        SetValue(LiquidGlassInteraction::PressedScaleProperty(), box_value(.9));
        SetValue(LiquidGlassInteraction::MotionDurationProperty(), box_value(130.0));
        SetValue(LiquidGlassInteraction::PressedRefractionMultiplierProperty(), box_value(2.25));
        SetValue(LiquidGlassInteraction::PressedRefractionBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedTintBoostProperty(), box_value(-.9));
        SetValue(LiquidGlassInteraction::PressedHighlightMultiplierProperty(), box_value(1.0));
        SetValue(LiquidGlassInteraction::PressedHighlightBoostProperty(), box_value(0.0));
        SetValue(LiquidGlassInteraction::PressedInnerShadowBoostProperty(), box_value(.25));

        auto weak = get_weak();
        Loaded([weak](auto const& sender, auto const&) {
            auto self = weak.get(); if (!self || self->m_interactionsWired) return;
            auto owner = sender.template try_as<DependencyObject>(); if (!owner) return;
            if (auto knob = NamedDescendant(owner, L"SwitchKnob").try_as<FrameworkElement>())
            {
                auto const scale = std::clamp(LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                SetElementScale(knob, scale, scale);
            }
            self->m_interactionsWired = true;
        });
        PointerPressed([weak](auto const& sender, auto const&) {
            if (auto self = weak.get()) {
                auto owner = sender.template try_as<DependencyObject>(); if (!owner) return;
                auto knob = NamedDescendant(owner, L"SwitchKnob").try_as<FrameworkElement>();
                auto const scale = std::clamp(LiquidGlassInteraction::GetPressedScale(owner), .25, 4.0);
                AnimateScale(knob, scale, LiquidGlassInteraction::GetMotionDuration(owner));
            }
        });
        auto release = [weak](auto const& sender, auto const&) {
            if (auto self = weak.get()) {
                auto owner = sender.template try_as<DependencyObject>(); if (!owner) return;
                auto knob = NamedDescendant(owner, L"SwitchKnob").try_as<FrameworkElement>();
                auto const scale = std::clamp(LiquidGlassInteraction::GetRestScale(owner), .25, 4.0);
                AnimateScale(knob, scale, LiquidGlassInteraction::GetMotionDuration(owner));
            }
        };
        PointerReleased(release); PointerCaptureLost(release); PointerCanceled(release);
        auto pulse = [weak](auto const& sender, auto const&) {
            if (auto self = weak.get()) {
                auto owner = sender.template try_as<DependencyObject>(); if (!owner) return;
                auto knob = NamedDescendant(owner, L"SwitchKnob").try_as<FrameworkElement>();
                auto const rest = LiquidGlassInteraction::GetRestScale(owner);
                auto const pressed = LiquidGlassInteraction::GetPressedScale(owner);
                AnimateScale(knob, std::clamp(rest + (pressed - rest) * .4, .25, 4.0),
                    LiquidGlassInteraction::GetMotionDuration(owner));
            }
        };
        Checked(pulse); Unchecked(pulse);
    }

    Windows::Foundation::IInspectable LiquidGlassToggleSwitch::Header() const { return m_header; }
    void LiquidGlassToggleSwitch::Header(Windows::Foundation::IInspectable const& value) { m_header = value; Content(value); }
    bool LiquidGlassToggleSwitch::IsOn() const { auto v = IsChecked(); return v && v.Value(); }
    void LiquidGlassToggleSwitch::IsOn(bool value) { IsChecked(box_value(value).as<Windows::Foundation::IReference<bool>>()); }
}
