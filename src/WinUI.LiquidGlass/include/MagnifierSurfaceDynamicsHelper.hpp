#pragma once

#include <cmath>
#include <memory>
#include <utility>

#include "ChildSurfaceInteraction.hpp"
#include "MotionAnimation.hpp"

namespace winrt::WinUI::LiquidGlass::detail
{
    template<typename Self>
    struct MagnifierSurfaceDynamicsState
    {
        using Clock = std::chrono::steady_clock;

        static constexpr double kVelocityTimeConstantSeconds = 0.040;
        static constexpr double kMaximumStretch = 0.105;
        static constexpr double kStretchHalfSpeed = 1050.0;
        static constexpr double kCrossAxisCompression = 0.42;
        static constexpr double kPickupScale = 1.035;
        static constexpr double kMaximumTiltDegrees = 5.5;

        Microsoft::UI::Xaml::Controls::Border surface{ nullptr };
        Microsoft::UI::Xaml::UIElement coordinateRoot{ nullptr };
        Windows::Foundation::Point lastPoint{};
        Clock::time_point lastTime{};
        double filteredVelocityX{};
        double filteredVelocityY{};
        std::uint32_t pointerId{};
        bool loaded{};
        bool active{};

        static double SmoothingAlpha(double elapsedSeconds)
        {
            if (elapsedSeconds <= 0.0) return 0.0;
            return 1.0 - std::exp(-elapsedSeconds / kVelocityTimeConstantSeconds);
        }

        void ResolveSurface(Self* owner)
        {
            auto root = owner->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            if (!root)
            {
                surface = nullptr;
                return;
            }
            surface = FindNamedDescendant(root, L"MagnifierSurface")
                .try_as<Microsoft::UI::Xaml::Controls::Border>();
        }

        static Microsoft::UI::Xaml::DependencyObject DependencyOwner(Self* owner)
        {
            return owner->template try_as<Microsoft::UI::Xaml::DependencyObject>();
        }

        void SetRotation(double degrees)
        {
            if (!surface) return;
            auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(surface);
            visual.CenterPoint({
                static_cast<float>(surface.ActualWidth() * .5),
                static_cast<float>(surface.ActualHeight() * .5),
                0.0f });
            visual.StopAnimation(L"RotationAngleInDegrees");
            visual.RotationAngleInDegrees(static_cast<float>(degrees));
        }

        void AnimateRotationToRest(Self* owner)
        {
            if (!surface) return;
            auto dependencyOwner = DependencyOwner(owner);
            if (!dependencyOwner) return;
            auto visual = Microsoft::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(surface);
            visual.CenterPoint({
                static_cast<float>(surface.ActualWidth() * .5),
                static_cast<float>(surface.ActualHeight() * .5),
                0.0f });
            if (!MotionAnimationsEnabled(dependencyOwner))
            {
                visual.StopAnimation(L"RotationAngleInDegrees");
                visual.RotationAngleInDegrees(0.0f);
                return;
            }
            auto animation = visual.Compositor().CreateSpringScalarAnimation();
            animation.FinalValue(box_value(0.0f).as<Windows::Foundation::IReference<float>>());
            animation.DampingRatio(.58f);
            animation.Period(std::chrono::milliseconds{ 185 });
            visual.StartAnimation(L"RotationAngleInDegrees", animation);
        }

        void Begin(Self* owner, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!loaded || active) return;
            if (!surface) ResolveSurface(owner);
            if (!surface) return;

            auto element = owner->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!element) return;
            auto xamlRoot = element.XamlRoot();
            coordinateRoot = xamlRoot
                ? xamlRoot.Content()
                : owner->template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!coordinateRoot) return;

            auto const point = args.GetCurrentPoint(coordinateRoot);
            pointerId = point.PointerId();
            lastPoint = point.Position();
            lastTime = Clock::now();
            filteredVelocityX = 0.0;
            filteredVelocityY = 0.0;
            active = true;

            auto dependencyOwner = DependencyOwner(owner);
            if (dependencyOwner)
            {
                AnimateElementScaleSpring(
                    dependencyOwner,
                    surface,
                    kPickupScale,
                    kPickupScale,
                    .68,
                    90.0);
            }
            SetRotation(0.0);
        }

        void Move(Self*, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            if (!loaded || !active || !surface || !coordinateRoot) return;
            auto const point = args.GetCurrentPoint(coordinateRoot);
            if (point.PointerId() != pointerId) return;

            auto const position = point.Position();
            auto const now = Clock::now();
            auto const elapsed = std::chrono::duration<double>(now - lastTime).count();
            if (elapsed > 1e-4 && elapsed < 0.16)
            {
                auto const instantaneousX = static_cast<double>(position.X - lastPoint.X) / elapsed;
                auto const instantaneousY = static_cast<double>(position.Y - lastPoint.Y) / elapsed;
                auto const alpha = SmoothingAlpha(elapsed);
                filteredVelocityX += (instantaneousX - filteredVelocityX) * alpha;
                filteredVelocityY += (instantaneousY - filteredVelocityY) * alpha;
            }
            else if (elapsed >= 0.16)
            {
                filteredVelocityX *= .35;
                filteredVelocityY *= .35;
            }

            auto const speed = std::hypot(filteredVelocityX, filteredVelocityY);
            auto const stretch = kMaximumStretch * speed / (speed + kStretchHalfSpeed);
            auto axisX = 1.0;
            auto axisY = 0.0;
            if (speed > 1.0)
            {
                axisX = std::abs(filteredVelocityX) / speed;
                axisY = std::abs(filteredVelocityY) / speed;
            }
            auto const axisX2 = axisX * axisX;
            auto const axisY2 = axisY * axisY;
            auto const scaleX = kPickupScale * (
                1.0 + stretch * axisX2 - stretch * kCrossAxisCompression * axisY2);
            auto const scaleY = kPickupScale * (
                1.0 + stretch * axisY2 - stretch * kCrossAxisCompression * axisX2);
            SetElementScale(surface, scaleX, scaleY);

            auto const horizontalVelocity = std::clamp(filteredVelocityX / 1500.0, -1.0, 1.0);
            SetRotation(-horizontalVelocity * kMaximumTiltDegrees);

            lastPoint = position;
            lastTime = now;
        }

        void End(Self* owner, bool animate)
        {
            if (!active)
            {
                coordinateRoot = nullptr;
                pointerId = 0;
                return;
            }

            active = false;
            auto dependencyOwner = DependencyOwner(owner);
            if (surface && dependencyOwner)
            {
                if (animate)
                {
                    AnimateElementScaleSpring(
                        dependencyOwner,
                        surface,
                        1.0,
                        1.0,
                        .58,
                        185.0);
                    AnimateRotationToRest(owner);
                }
                else
                {
                    SetElementScale(surface, 1.0, 1.0);
                    SetRotation(0.0);
                }
            }

            coordinateRoot = nullptr;
            pointerId = 0;
            filteredVelocityX = 0.0;
            filteredVelocityY = 0.0;
        }

        void ClearForTeardown() noexcept
        {
            // Unloaded can race compositor shutdown; drop references without visual writes.
            loaded = false;
            active = false;
            coordinateRoot = nullptr;
            surface = nullptr;
            pointerId = 0;
            filteredVelocityX = 0.0;
            filteredVelocityY = 0.0;
        }
    };

    template<typename Self>
    void InstallMagnifierSurfaceDynamics(Self* self)
    {
        using State = MagnifierSurfaceDynamicsState<Self>;
        using Handler = Microsoft::UI::Xaml::Input::PointerEventHandler;

        auto state = std::make_shared<State>();
        auto weak = self->get_weak();

        self->Loaded([state, weak](auto const&, auto const&)
        {
            state->loaded = true;
            if (auto owner = weak.get()) state->ResolveSurface(owner.get());
        });
        self->Unloaded([state](auto const&, auto const&)
        {
            state->ClearForTeardown();
        });

        auto addPointerHandler = [self](auto routedEvent, auto&& callback)
        {
            auto handler = winrt::box_value<Handler>({
                std::forward<decltype(callback)>(callback) });
            self->AddHandler(routedEvent, handler, true);
        };

        addPointerHandler(
            Microsoft::UI::Xaml::UIElement::PointerPressedEvent(),
            [state, weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                if (auto owner = weak.get()) state->Begin(owner.get(), args);
            });
        addPointerHandler(
            Microsoft::UI::Xaml::UIElement::PointerMovedEvent(),
            [state, weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
            {
                if (auto owner = weak.get()) state->Move(owner.get(), args);
            });
        addPointerHandler(
            Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(),
            [state, weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
            {
                if (auto owner = weak.get()) state->End(owner.get(), true);
            });
        addPointerHandler(
            Microsoft::UI::Xaml::UIElement::PointerCaptureLostEvent(),
            [state, weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
            {
                if (auto owner = weak.get()) state->End(owner.get(), true);
            });
        addPointerHandler(
            Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(),
            [state, weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
            {
                if (auto owner = weak.get()) state->End(owner.get(), true);
            });
    }
}
