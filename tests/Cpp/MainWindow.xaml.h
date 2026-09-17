#pragma once

#include <microsoft.ui.xaml.window.h>

#include "MainWindow.g.h"
import winrt.WinUI.Composition.Hlsl;
import winrt.WinUI.LiquidGlass;
import winrt.Microsoft.UI.Dispatching;

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		MainWindow();

		void InitializeComponent();

		void OnSetBackgroundImageClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void OnClearBackgroundImageClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void OnRootDragOver(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& args);
		void OnRootDrop(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& args);
		void OnBackdropHostLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void OnBackdropHostSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& args);
		void OnBackdropHostPointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnBackdropHostPointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnBackdropFramePointerWheelChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnBackdropHostPointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnBackdropHostPointerCanceled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnBackdropHostPointerCaptureLost(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnBackdropHostPointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& args);
		void OnEffectSelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
		void OnBorderWidthChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
		void OnLiquidGlassParameterChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
		void OnPointerLightingToggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void OnAdvancedLiquidGlassLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

	private:
		winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer m_smokeTimer{ nullptr };
		enum class BackdropInteraction
		{
			None,
			Drag,
			Resize,
		};

		enum class BackdropEffectKind
		{
			Solid,
			LinearGradient,
			Blur,
			Invert,
			LiquidGlass,
		};

		winrt::Microsoft::UI::Xaml::Media::XamlCompositionBrushBase m_backdropXamlBrush{ nullptr };
		winrt::Microsoft::UI::Xaml::Media::IXamlCompositionBrushBaseProtected m_backdropBrushProtected{ nullptr };
		winrt::WinUI::Composition::Hlsl::HlslEffectBrush m_backdropEffectBrush{ nullptr };
		winrt::WinUI::Composition::Hlsl::LiquidGlassMaterial m_liquidGlassMaterial{ nullptr };
		winrt::Microsoft::UI::Input::InputPointerSource m_pointerSource{ nullptr };
		winrt::Microsoft::UI::Input::InputCursor m_arrowCursor{ nullptr };
		winrt::Microsoft::UI::Input::InputCursor m_moveCursor{ nullptr };
		winrt::Microsoft::UI::Input::InputCursor m_resizeCursor{ nullptr };
		winrt::Microsoft::UI::Xaml::Controls::ContentControl m_regressionStressMount{ nullptr };
		BackdropEffectKind m_backdropEffect{ BackdropEffectKind::Solid };
		BackdropInteraction m_backdropInteraction{ BackdropInteraction::None };
		float m_borderWidth{ 2.0f };
		uint32_t m_activePointerId{};
		winrt::Windows::Foundation::Point m_startPointer{};
		float m_startOffsetX{};
		float m_startOffsetY{};
		float m_startWidth{};
		float m_startHeight{};
		bool m_advancedLiquidGlassWired{};
		bool m_regressionLabBuilt{};

		void StartDynamicScene();
		void InitializeBackdropBrush();
		void ApplyBackdropEffect();
		winrt::fire_and_forget PickBackgroundImageAsync();
		winrt::fire_and_forget SetBackgroundImageFromDropAsync(winrt::Microsoft::UI::Xaml::DragEventArgs args);
		winrt::Windows::Foundation::IAsyncAction SetBackgroundImageAsync(winrt::Windows::Storage::StorageFile file);
		void ClearBackgroundImage();
		void ApplyLiquidGlassProperties();
		void ApplyAdvancedLiquidGlassProperties();
		void BuildRegressionLab();
		void UpdateLiquidGlassControlsState();
		void UpdateLiquidGlassPointerLighting(winrt::Windows::Foundation::Point const& position);
		void AnimateBackdropScale(float targetScale, int durationMilliseconds);
		void UpdateBackdropVisualCenterPoint();
		void ClampBackdropFrameRect();
		bool HitTestBackdropFrame(winrt::Windows::Foundation::Point const& position);
		bool HitTestResizeGrip(winrt::Windows::Foundation::Point const& position);
		void InitializeBackdropCursors();
		void EnsurePointerSource();
		void SetBackdropCursor(winrt::Microsoft::UI::Input::InputCursor const& cursor);
		void UpdateBackdropCursor(winrt::Windows::Foundation::Point const& position);
		void EndBackdropInteraction();
		HWND GetWindowHandle();
	};
}

namespace winrt::WUILiquidGlassDemo_Hlsl::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
