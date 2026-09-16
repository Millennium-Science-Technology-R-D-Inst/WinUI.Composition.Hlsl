#pragma once

namespace winrt::WinUI::LiquidGlass::detail
{
    class PointerFieldRouter : public std::enable_shared_from_this<PointerFieldRouter>
    {
    public:
        using Clock = std::chrono::steady_clock;
        using UpdateCallback = std::function<void(
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&,
            Clock::time_point)>;
        using DeactivateCallback = std::function<void()>;

        static std::shared_ptr<PointerFieldRouter> For(
            Microsoft::UI::Xaml::XamlRoot const& xamlRoot)
        {
            if (!xamlRoot) return {};

            auto const key = reinterpret_cast<std::uintptr_t>(get_abi(xamlRoot));
            if (auto found = s_routers.find(key); found != s_routers.end())
            {
                if (auto router = found->second.lock()) return router;
                s_routers.erase(found);
            }

            auto router = std::shared_ptr<PointerFieldRouter>(
                new PointerFieldRouter(xamlRoot));
            s_routers.emplace(key, router);
            return router;
        }

        std::uint64_t Register(UpdateCallback update, DeactivateCallback deactivate)
        {
            if (!update || !deactivate) return 0;
            if (m_targets.empty()) Attach();

            auto const id = m_nextId++;
            m_targets.push_back(Target{ id, std::move(update), std::move(deactivate) });
            return id;
        }

        void Unregister(std::uint64_t id)
        {
            if (!id) return;

            auto found = std::find_if(m_targets.begin(), m_targets.end(),
                [id](Target const& target) { return target.id == id; });
            if (found == m_targets.end()) return;

            found->deactivate();
            m_targets.erase(found);
            if (m_targets.empty()) Detach();
        }

    private:
        struct Target
        {
            std::uint64_t id{};
            UpdateCallback update;
            DeactivateCallback deactivate;
        };

        explicit PointerFieldRouter(Microsoft::UI::Xaml::XamlRoot const& xamlRoot) :
            m_xamlRoot(xamlRoot)
        {
        }

        void Attach()
        {
            if (m_attached || !m_xamlRoot) return;
            m_root = m_xamlRoot.Content();
            if (!m_root) return;

            auto weak = weak_from_this();
            m_pointerMovedHandler = winrt::box_value<Microsoft::UI::Xaml::Input::PointerEventHandler>({
                [weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    if (auto router = weak.lock()) router->PointerMoved(args);
                } });
            m_pointerExitedHandler = winrt::box_value<Microsoft::UI::Xaml::Input::PointerEventHandler>({
                [weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
                {
                    if (auto router = weak.lock()) router->PointerExited(args);
                } });
            m_pointerCanceledHandler = winrt::box_value<Microsoft::UI::Xaml::Input::PointerEventHandler>({
                [weak](auto const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&)
                {
                    if (auto router = weak.lock()) router->DeactivateAll();
                } });

            // handledEventsToo keeps the spatial field alive over controls such as
            // Slider/ScrollViewer that may consume PointerMoved themselves.
            m_root.AddHandler(
                Microsoft::UI::Xaml::UIElement::PointerMovedEvent(),
                m_pointerMovedHandler,
                true);
            m_root.AddHandler(
                Microsoft::UI::Xaml::UIElement::PointerExitedEvent(),
                m_pointerExitedHandler,
                true);
            m_root.AddHandler(
                Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(),
                m_pointerCanceledHandler,
                true);
            m_attached = true;
        }

        void Detach()
        {
            if (!m_attached || !m_root) return;
            m_root.RemoveHandler(
                Microsoft::UI::Xaml::UIElement::PointerMovedEvent(),
                m_pointerMovedHandler);
            m_root.RemoveHandler(
                Microsoft::UI::Xaml::UIElement::PointerExitedEvent(),
                m_pointerExitedHandler);
            m_root.RemoveHandler(
                Microsoft::UI::Xaml::UIElement::PointerCanceledEvent(),
                m_pointerCanceledHandler);
            m_pointerMovedHandler = nullptr;
            m_pointerExitedHandler = nullptr;
            m_pointerCanceledHandler = nullptr;
            m_root = nullptr;
            m_attached = false;
        }

        void PointerMoved(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            auto const now = Clock::now();
            // A callback can unload a control and unregister it. Iterate over a snapshot
            // so router storage remains stable during routed-event reentrancy.
            auto const targets = m_targets;
            for (auto const& target : targets)
            {
                target.update(args, now);
            }
        }

        void PointerExited(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args)
        {
            // PointerExited is routed, so child-to-child movement also reaches the root.
            // Only deactivate when the pointer has actually left the XamlRoot content.
            auto rootElement = m_root.try_as<Microsoft::UI::Xaml::FrameworkElement>();
            if (!rootElement)
            {
                DeactivateAll();
                return;
            }

            auto const point = args.GetCurrentPoint(m_root).Position();
            constexpr double epsilon = 0.5;
            if (point.X < -epsilon || point.Y < -epsilon ||
                point.X > rootElement.ActualWidth() + epsilon ||
                point.Y > rootElement.ActualHeight() + epsilon)
            {
                DeactivateAll();
            }
        }

        void DeactivateAll()
        {
            auto const targets = m_targets;
            for (auto const& target : targets)
            {
                target.deactivate();
            }
        }

        inline static thread_local std::unordered_map<
            std::uintptr_t,
            std::weak_ptr<PointerFieldRouter>> s_routers;

        Microsoft::UI::Xaml::XamlRoot m_xamlRoot{ nullptr };
        Microsoft::UI::Xaml::UIElement m_root{ nullptr };
        Windows::Foundation::IInspectable m_pointerMovedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerExitedHandler{ nullptr };
        Windows::Foundation::IInspectable m_pointerCanceledHandler{ nullptr };
        std::vector<Target> m_targets;
        std::uint64_t m_nextId{ 1 };
        bool m_attached{};
    };

    template<typename Self>
    class PointerFieldHelper
    {
    public:
        PointerFieldHelper()
        {
            auto self = static_cast<Self*>(this);
            self->Loaded([this](auto const&, auto const&) { Attach(); });
            self->Unloaded([this](auto const&, auto const&) { Detach(); });
            self->SizeChanged([this](auto const&, auto const&)
            {
                m_configurationDirty = true;
            });
            self->RegisterPropertyChangedCallback(
                Self::GlassBrushProperty(),
                [this](Microsoft::UI::Xaml::DependencyObject const&,
                       Microsoft::UI::Xaml::DependencyProperty const&)
                {
                    DeactivateTrackedMaterial();
                    m_configurationDirty = true;
                });
        }

    private:
        static constexpr double kHoverRangeDips = 28.0;

        static void DeactivateEffect(
            WinUI::Composition::Hlsl::HlslEffectBrush const& effect)
        {
            if (!effect) return;

            try
            {
                effect.SetFloat(L"PointerActive", 0.0f);
                effect.SetFloat(L"PointerVelocityX", 0.0f);
                effect.SetFloat(L"PointerVelocityY", 0.0f);
            }
            catch (winrt::hresult_error const& error)
            {
                // Window teardown can close the underlying CompositionEffectBrush before
                // XAML raises Unloaded. Resetting pointer properties is only cleanup at that
                // point, so RO_E_CLOSED is benign; all other failures remain visible.
                if (error.code() != winrt::hresult{ RO_E_CLOSED }) throw;
            }
        }

        void Attach()
        {
            if (m_router) return;

            auto self = static_cast<Self*>(this);
            auto xamlRoot = self->XamlRoot();
            if (!xamlRoot) return;

            auto router = PointerFieldRouter::For(xamlRoot);
            if (!router) return;

            auto weak = self->get_weak();
            auto const id = router->Register(
                [weak](Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args,
                       PointerFieldRouter::Clock::time_point now)
                {
                    if (auto target = weak.get())
                    {
                        static_cast<PointerFieldHelper<Self>*>(target.get())->Update(args, now);
                    }
                },
                [weak]
                {
                    if (auto target = weak.get())
                    {
                        static_cast<PointerFieldHelper<Self>*>(target.get())->DeactivateTrackedMaterial();
                    }
                });

            if (!id) return;
            m_router = std::move(router);
            m_registrationId = id;
        }

        void Detach()
        {
            if (m_router && m_registrationId)
            {
                m_router->Unregister(m_registrationId);
            }
            m_registrationId = 0;
            m_router.reset();
            DeactivateTrackedMaterial();
        }

        void TrackMaterial(WinUI::Composition::Hlsl::LiquidGlassMaterial const& material)
        {
            if (m_trackingMaterial &&
                (!material || get_abi(m_trackingMaterial) != get_abi(material)))
            {
                DeactivateEffect(m_trackingMaterial.EffectBrush());
            }

            if (!material)
            {
                m_trackingMaterial = nullptr;
                m_lastPointValid = false;
                return;
            }

            if (!m_trackingMaterial || get_abi(m_trackingMaterial) != get_abi(material))
            {
                m_trackingMaterial = material;
                m_configurationDirty = true;
                m_lastPointValid = false;
            }
        }

        void Configure(
            WinUI::Composition::Hlsl::LiquidGlassBrush const& brush,
            WinUI::Composition::Hlsl::HlslEffectBrush const& effect,
            double width,
            double height)
        {
            if (!brush || !effect) return;

            auto const maxExtent = std::max(width, height);
            if (maxExtent <= 1e-4) return;

            auto const interactionRadiusDips = std::clamp(
                maxExtent * 0.65,
                56.0,
                180.0);
            auto const refraction = static_cast<float>(std::clamp(
                brush.RefractionStrength() * 0.24,
                2.0,
                8.0));
            auto const highlight = static_cast<float>(std::clamp(
                brush.HighlightStrength() * 0.55,
                0.12,
                0.40));

            // Spatial lengths are normalized against the largest layout extent.
            // The shader multiplies them by its own raster-space extent, keeping
            // pointer and SDF coordinates aligned under DPI/composition scaling.
            effect.SetFloat(L"PointerInteractionRadius",
                static_cast<float>(interactionRadiusDips / maxExtent));
            effect.SetFloat(L"PointerInteractionStrength", 1.0f);
            effect.SetFloat(L"PointerHoverRange",
                static_cast<float>(kHoverRangeDips / maxExtent));
            effect.SetFloat(L"PointerRefractionStrength", refraction);
            effect.SetFloat(L"PointerHighlightStrength", highlight);
            effect.SetFloat(L"PointerMotionRefractionStrength", 5.0f);
            m_configurationDirty = false;
        }

        void Update(
            Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args,
            PointerFieldRouter::Clock::time_point now)
        {
            auto self = static_cast<Self*>(this);
            auto owner = self->template try_as<Microsoft::UI::Xaml::DependencyObject>();
            auto element = self->template try_as<Microsoft::UI::Xaml::FrameworkElement>();
            auto relativeTo = self->template try_as<Microsoft::UI::Xaml::UIElement>();
            if (!owner || !element || !relativeTo) return;

            auto brush = self->GlassBrush();
            auto material = brush ? brush.Material() : WinUI::Composition::Hlsl::LiquidGlassMaterial{ nullptr };
            TrackMaterial(material);
            if (!brush || !material) return;

            auto effect = material.EffectBrush();
            if (!effect) return;

            auto const width = element.ActualWidth();
            auto const height = element.ActualHeight();
            if (width <= 0.0 || height <= 0.0)
            {
                SetInactive(effect);
                return;
            }

            auto const point = args.GetCurrentPoint(relativeTo).Position();
            auto const active =
                point.X >= -kHoverRangeDips && point.X <= width + kHoverRangeDips &&
                point.Y >= -kHoverRangeDips && point.Y <= height + kHoverRangeDips;
            if (!active)
            {
                SetInactive(effect);
                return;
            }

            if (m_configurationDirty)
            {
                Configure(brush, effect, width, height);
            }

            float velocityX = 0.0f;
            float velocityY = 0.0f;
            if (m_lastPointValid)
            {
                auto const elapsed = std::chrono::duration<double>(now - m_lastTime).count();
                if (elapsed > 1e-4 && elapsed < 0.25)
                {
                    velocityX = static_cast<float>(std::clamp(
                        ((point.X - m_lastPoint.X) / width) / elapsed,
                        -100.0,
                        100.0));
                    velocityY = static_cast<float>(std::clamp(
                        ((point.Y - m_lastPoint.Y) / height) / elapsed,
                        -100.0,
                        100.0));
                }
            }

            effect.SetFloat(L"PointerX", static_cast<float>(point.X / width));
            effect.SetFloat(L"PointerY", static_cast<float>(point.Y / height));
            effect.SetFloat(L"PointerVelocityX", velocityX);
            effect.SetFloat(L"PointerVelocityY", velocityY);
            effect.SetFloat(L"PointerActive", 1.0f);

            m_lastPoint = point;
            m_lastTime = now;
            m_lastPointValid = true;
            m_active = true;
        }

        void SetInactive(WinUI::Composition::Hlsl::HlslEffectBrush const& effect)
        {
            if (m_active)
            {
                DeactivateEffect(effect);
            }
            m_active = false;
            m_lastPointValid = false;
        }

        void DeactivateTrackedMaterial()
        {
            if (m_trackingMaterial)
            {
                DeactivateEffect(m_trackingMaterial.EffectBrush());
            }
            m_trackingMaterial = nullptr;
            m_active = false;
            m_lastPointValid = false;
        }

        std::shared_ptr<PointerFieldRouter> m_router;
        WinUI::Composition::Hlsl::LiquidGlassMaterial m_trackingMaterial{ nullptr };
        Windows::Foundation::Point m_lastPoint{};
        PointerFieldRouter::Clock::time_point m_lastTime{};
        std::uint64_t m_registrationId{};
        bool m_configurationDirty{ true };
        bool m_lastPointValid{};
        bool m_active{};
    };
}
