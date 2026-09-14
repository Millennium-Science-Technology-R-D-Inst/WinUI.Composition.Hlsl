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
        using Xaml::DependencyProperty;
        using Xaml::DependencyPropertyChangedEventArgs;
        using Xaml::FrameworkElement;
        using Xaml::PropertyChangedCallback;
        using Xaml::PropertyMetadata;

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

        void SetElementScale(FrameworkElement const& target, float x, float y)
        {
            if (!target) return;
            auto v = Hosting::ElementCompositionPreview::GetElementVisual(target);
            v.CenterPoint({ float(target.ActualWidth() * .5), float(target.ActualHeight() * .5), 0 });
            v.Scale({ x, y, 1 });
        }

        void AnimateScale(FrameworkElement const& target, float x, float y, std::chrono::milliseconds duration)
        {
            if (!target) return;
            auto v = Hosting::ElementCompositionPreview::GetElementVisual(target);
            v.CenterPoint({ float(target.ActualWidth() * .5), float(target.ActualHeight() * .5), 0 });
            auto a = v.Compositor().CreateVector3KeyFrameAnimation();
            a.InsertKeyFrame(1, { x, y, 1 }); a.Duration(duration); v.StartAnimation(L"Scale", a);
        }

        void AnimateScale(FrameworkElement const& target, float value, std::chrono::milliseconds duration)
        {
            AnimateScale(target, value, value, duration);
        }

        Primitives::Thumb SliderThumb(Controls::Slider const& slider)
        {
            auto name = slider.Orientation() == Controls::Orientation::Horizontal
                ? L"HorizontalThumb" : L"VerticalThumb";
            auto thumb = NamedDescendant(slider, name).try_as<Primitives::Thumb>();
            return thumb ? thumb : Descendant<Primitives::Thumb>(slider);
        }

        void SliderState(Brush const& b, bool active)
        {
            if (!b) return;
            b.RefractionStrength(active ? 21.6 : 9.6);
            b.TintOpacity(active ? .10 : 1.0);
        }

        void SwitchState(Brush const& b, bool active)
        {
            if (!b) return;
            b.RefractionStrength(active ? 21.6 : 9.6);
            b.TintOpacity(active ? .10 : 1.0);
            b.InnerShadowStrength(active ? .27 : .02);
        }
    }

#define GLASS_DP(Type) \
    void Type::EnsureDependencyProperties() { (void)GlassBrushProperty(); } \
    DependencyProperty Type::GlassBrushProperty() { \
        static auto p = DependencyProperty::Register(L"GlassBrush", xaml_typename<Brush>(), \
            xaml_typename<class_type>(), PropertyMetadata{ Windows::Foundation::IInspectable{ nullptr }, \
            PropertyChangedCallback{ OnGlassBrushChanged } }); return p; } \
    Brush Type::GlassBrush() const { return GetValue(GlassBrushProperty()).try_as<Brush>(); } \
    void Type::GlassBrush(Brush const& value) { SetValue(GlassBrushProperty(), value); } \
    void Type::OnGlassBrushChanged(DependencyObject const& o, DependencyPropertyChangedEventArgs const& e) { \
        detail::EnsureDependencyProperty<Type>::GetSelf(o)->ApplyGlassBrush(e.NewValue().try_as<Brush>()); }

#define BACKGROUND_BRUSH(Type) \
    void Type::ApplyGlassBrush(Brush const& value) { m_glassBrush = value; Background(AsBrush(value)); }

#define SIMPLE_CONTROL(Type, PresetValue) \
    GLASS_DP(Type) BACKGROUND_BRUSH(Type) \
    Type::Type() { GlassBrush(CreateBrush(PresetValue)); }

    SIMPLE_CONTROL(LiquidGlassCard, Preset::Panel)
    SIMPLE_CONTROL(LiquidGlassButton, Preset::Button)
    SIMPLE_CONTROL(LiquidGlassToggleButton, Preset::Button)
    SIMPLE_CONTROL(LiquidGlassHyperlinkButton, Preset::Button)
    SIMPLE_CONTROL(LiquidGlassCheckBox, Preset::Choice)
    SIMPLE_CONTROL(LiquidGlassRadioButton, Preset::Choice)
    SIMPLE_CONTROL(LiquidGlassComboBox, Preset::Choice)

    GLASS_DP(LiquidGlassTextBox)
    BACKGROUND_BRUSH(LiquidGlassTextBox)
    LiquidGlassTextBox::LiquidGlassTextBox()
    {
        GlassBrush(CreateBrush(Preset::Search));
        Loaded([](auto const& s, auto const&) { SetElementScale(s.template try_as<FrameworkElement>(), .8f, .8f); });
        GotFocus([](auto const& s, auto const&) {
            AnimateScale(s.template try_as<FrameworkElement>(), 1, 120ms);
            if (auto c = s.template try_as<WinUI::LiquidGlass::LiquidGlassTextBox>(); c && c.GlassBrush())
                c.GlassBrush().TintOpacity(.20);
        });
        LostFocus([](auto const& s, auto const&) {
            AnimateScale(s.template try_as<FrameworkElement>(), .8f, 170ms);
            if (auto c = s.template try_as<WinUI::LiquidGlass::LiquidGlassTextBox>(); c && c.GlassBrush())
                c.GlassBrush().TintOpacity(.05);
        });
    }

    GLASS_DP(LiquidGlassPasswordBox)
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

        auto weak = get_weak();
        m_passwordBox.GotFocus([weak](auto const& s, auto const&) {
            AnimateScale(s.template try_as<FrameworkElement>(), 1, 120ms);
            if (auto self = weak.get(); self && self->GlassBrush()) self->GlassBrush().TintOpacity(.14);
        });
        m_passwordBox.LostFocus([weak](auto const& s, auto const&) {
            AnimateScale(s.template try_as<FrameworkElement>(), .99f, 160ms);
            if (auto self = weak.get(); self && self->GlassBrush()) self->GlassBrush().TintOpacity(.10);
        });
    }
    hstring LiquidGlassPasswordBox::PlaceholderText() const { return m_passwordBox ? m_passwordBox.PlaceholderText() : hstring{}; }
    void LiquidGlassPasswordBox::PlaceholderText(hstring const& value) { if (m_passwordBox) m_passwordBox.PlaceholderText(value); }
    hstring LiquidGlassPasswordBox::Password() const { return m_passwordBox ? m_passwordBox.Password() : hstring{}; }
    void LiquidGlassPasswordBox::Password(hstring const& value) { if (m_passwordBox) m_passwordBox.Password(value); }

    GLASS_DP(LiquidGlassMagnifier)
    BACKGROUND_BRUSH(LiquidGlassMagnifier)
    LiquidGlassMagnifier::LiquidGlassMagnifier()
    {
        GlassBrush(CreateBrush(Preset::Magnifier));
        RenderTransformOrigin({ .5f, .5f });
        m_dragTransform = Media::CompositeTransform{};
        RenderTransform(m_dragTransform);
        Loaded([](auto const& s, auto const&) { SetElementScale(s.template try_as<FrameworkElement>(), .8f, .8f); });

        auto weak = get_weak();
        PointerPressed([weak](auto const& sender, Xaml::Input::PointerRoutedEventArgs const& e) {
            auto self = weak.get();
            auto element = sender.template try_as<Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !element || !frameworkElement) return;
            auto p = e.GetCurrentPoint(element);
            self->m_activePointerId = p.PointerId(); self->m_lastPointer = p.Position();
            self->m_dragging = element.CapturePointer(e.Pointer());
            if (!self->m_dragging) return;
            if (auto b = self->GlassBrush()) { b.RefractionStrength(24); b.MagnificationStrength(48); b.InnerShadowStrength(.27); }
            AnimateScale(frameworkElement, 1, 110ms); e.Handled(true);
        });
        PointerMoved([weak](auto const& sender, Xaml::Input::PointerRoutedEventArgs const& e) {
            auto self = weak.get();
            auto element = sender.template try_as<Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !self->m_dragging || !element || !frameworkElement) return;
            auto p = e.GetCurrentPoint(element); if (p.PointerId() != self->m_activePointerId) return;
            auto pos = p.Position(); auto dx = pos.X - self->m_lastPointer.X; auto dy = pos.Y - self->m_lastPointer.Y;
            self->m_dragTransform.TranslateX(self->m_dragTransform.TranslateX() + dx);
            self->m_dragTransform.TranslateY(self->m_dragTransform.TranslateY() + dy);
            auto sy = std::max(.7f, 1.f - std::abs(float(dx * 60)) / 5000.f);
            SetElementScale(frameworkElement, 2.f - sy, sy); self->m_lastPointer = pos; e.Handled(true);
        });
        PointerReleased([weak](auto const& sender, Xaml::Input::PointerRoutedEventArgs const& e) {
            auto self = weak.get();
            auto element = sender.template try_as<Xaml::UIElement>();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !self->m_dragging || !element || !frameworkElement) return;
            element.ReleasePointerCapture(e.Pointer()); self->m_dragging = false; self->m_activePointerId = 0;
            if (auto b = self->GlassBrush()) { b.RefractionStrength(19.2); b.MagnificationStrength(24); b.InnerShadowStrength(.20); }
            AnimateScale(frameworkElement, .8f, 180ms); e.Handled(true);
        });
        PointerCaptureLost([weak](auto const& sender, auto const&) {
            auto self = weak.get();
            auto frameworkElement = sender.template try_as<FrameworkElement>();
            if (!self || !frameworkElement) return;
            self->m_dragging = false; self->m_activePointerId = 0;
            if (auto b = self->GlassBrush()) { b.RefractionStrength(19.2); b.MagnificationStrength(24); b.InnerShadowStrength(.20); }
            AnimateScale(frameworkElement, .8f, 180ms);
        });
    }

    GLASS_DP(LiquidGlassSlider)
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
        auto weak = get_weak();
        Loaded([weak](auto const& sender, auto const&) {
            auto self = weak.get(); if (!self || self->m_interactionsWired) return;
            auto slider = sender.template try_as<Controls::Slider>(); if (!slider) return;
            auto thumb = SliderThumb(slider); if (!thumb) return;
            self->m_thumb = thumb;
            if (auto b = self->GlassBrush())
            {
                auto radius = std::max(1.0, std::min(thumb.ActualWidth(), thumb.ActualHeight()) * .5);
                b.CornerRadius(radius); b.BezelWidth(std::max(1.0, std::min(16.0, radius - .5)));
                if (auto surface = BrushSurface(thumb)) SetSurface(surface, AsBrush(b));
            }
            SetElementScale(thumb, .6f, .6f);
            thumb.DragStarted([weak](auto const& s, auto const&) {
                AnimateScale(s.template try_as<FrameworkElement>(), 1, 90ms);
                if (auto self = weak.get()) SliderState(self->GlassBrush(), true);
            });
            thumb.DragCompleted([weak](auto const& s, auto const&) {
                AnimateScale(s.template try_as<FrameworkElement>(), .6f, 180ms);
                if (auto self = weak.get()) SliderState(self->GlassBrush(), false);
            });
            self->m_interactionsWired = true;
        });
    }

    GLASS_DP(LiquidGlassToggleSwitch)
    BACKGROUND_BRUSH(LiquidGlassToggleSwitch)
    LiquidGlassToggleSwitch::LiquidGlassToggleSwitch()
    {
        GlassBrush(CreateBrush(Preset::Switch));
        auto weak = get_weak();
        Loaded([weak](auto const& sender, auto const&) {
            auto self = weak.get(); if (!self || self->m_interactionsWired) return;
            if (auto knob = NamedDescendant(sender.template try_as<DependencyObject>(), L"SwitchKnob").try_as<FrameworkElement>())
                SetElementScale(knob, .65f, .65f);
            self->m_interactionsWired = true;
        });
        PointerPressed([weak](auto const& sender, auto const&) {
            if (auto self = weak.get()) {
                auto knob = NamedDescendant(sender.template try_as<DependencyObject>(), L"SwitchKnob").try_as<FrameworkElement>();
                AnimateScale(knob, .9f, 90ms); SwitchState(self->GlassBrush(), true);
            }
        });
        auto release = [weak](auto const& sender, auto const&) {
            if (auto self = weak.get()) {
                auto knob = NamedDescendant(sender.template try_as<DependencyObject>(), L"SwitchKnob").try_as<FrameworkElement>();
                AnimateScale(knob, .65f, 190ms); SwitchState(self->GlassBrush(), false);
            }
        };
        PointerReleased(release); PointerCaptureLost(release);
        auto pulse = [weak](auto const& sender, auto const&) {
            if (auto self = weak.get()) {
                auto knob = NamedDescendant(sender.template try_as<DependencyObject>(), L"SwitchKnob").try_as<FrameworkElement>();
                AnimateScale(knob, .75f, 110ms);
            }
        };
        Checked(pulse); Unchecked(pulse);
    }
    Windows::Foundation::IInspectable LiquidGlassToggleSwitch::Header() const { return m_header; }
    void LiquidGlassToggleSwitch::Header(Windows::Foundation::IInspectable const& value) { m_header = value; Content(value); }
    bool LiquidGlassToggleSwitch::IsOn() const { auto v = IsChecked(); return v && v.Value(); }
    void LiquidGlassToggleSwitch::IsOn(bool value) { IsChecked(box_value(value).as<Windows::Foundation::IReference<bool>>()); }

#undef SIMPLE_CONTROL
#undef BACKGROUND_BRUSH
#undef GLASS_DP
}
