#pragma once
#include "LiquidGlassBrush.g.h"
#include <winrt/Windows.UI.ViewManagement.h>
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	struct LiquidGlassBrush : LiquidGlassBrushT<LiquidGlassBrush>
	{
		LiquidGlassBrush();
		void OnConnected();
		void OnDisconnected();
		bool IsEnabled() const;
		void IsEnabled(bool value);
		static Microsoft::UI::Xaml::DependencyProperty IsEnabledProperty();
		float BlurRadius() const;
		void BlurRadius(float value);
		static Microsoft::UI::Xaml::DependencyProperty BlurRadiusProperty();
		float RefractionStrength() const;
		void RefractionStrength(float value);
		static Microsoft::UI::Xaml::DependencyProperty RefractionStrengthProperty();
		float DispersionStrength() const;
		void DispersionStrength(float value);
		static Microsoft::UI::Xaml::DependencyProperty DispersionStrengthProperty();
		float CornerRadius() const;
		void CornerRadius(float value);
		static Microsoft::UI::Xaml::DependencyProperty CornerRadiusProperty();
		float BorderThickness() const;
		void BorderThickness(float value);
		static Microsoft::UI::Xaml::DependencyProperty BorderThicknessProperty();
		float HighlightStrength() const;
		void HighlightStrength(float value);
		static Microsoft::UI::Xaml::DependencyProperty HighlightStrengthProperty();
	private:
		void Update();
		static void Changed(Microsoft::UI::Xaml::DependencyObject const& object, Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const&);
		bool m_connected{};
		Hlsl::LiquidGlassMaterial m_material{ nullptr };
		Microsoft::UI::Dispatching::DispatcherQueue m_queue{ nullptr };
		Windows::UI::ViewManagement::AccessibilitySettings m_accessibility;
		Windows::UI::ViewManagement::UISettings m_ui;
		Windows::UI::ViewManagement::AccessibilitySettings::HighContrastChanged_revoker m_contrast;
		Windows::UI::ViewManagement::UISettings::AdvancedEffectsEnabledChanged_revoker m_effects;

	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct LiquidGlassBrush : LiquidGlassBrushT<LiquidGlassBrush, implementation::LiquidGlassBrush>
	{
	};
}
