#pragma once
#include "LiquidGlassBrush.g.h"
#include "XamlHlslBrushBase.h"

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
		double BlurRadius() const;
		void BlurRadius(double value);
		static Microsoft::UI::Xaml::DependencyProperty BlurRadiusProperty();
		double RefractionStrength() const;
		void RefractionStrength(double value);
		static Microsoft::UI::Xaml::DependencyProperty RefractionStrengthProperty();
		double DispersionStrength() const;
		void DispersionStrength(double value);
		static Microsoft::UI::Xaml::DependencyProperty DispersionStrengthProperty();
		double CornerRadius() const;
		void CornerRadius(double value);
		static Microsoft::UI::Xaml::DependencyProperty CornerRadiusProperty();
		double BorderThickness() const;
		void BorderThickness(double value);
		static Microsoft::UI::Xaml::DependencyProperty BorderThicknessProperty();
		double HighlightStrength() const;
		void HighlightStrength(double value);
		static Microsoft::UI::Xaml::DependencyProperty HighlightStrengthProperty();
	private:
		void Update();
        friend class hlsl::xaml::XamlHlslBrushBase<LiquidGlassBrush>;
        Microsoft::UI::Composition::CompositionBrush BuildPipeline(Microsoft::UI::Composition::Compositor const&);
        void ReleasePipeline() noexcept {m_material=nullptr;}
        hlsl::xaml::XamlHlslBrushBase<LiquidGlassBrush> m_lifecycle;
		static void Changed(Microsoft::UI::Xaml::DependencyObject const& object, Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const&);

		Hlsl::LiquidGlassMaterial m_material{ nullptr };



	};
}
namespace winrt::WinUI::Composition::Hlsl::factory_implementation
{
	struct LiquidGlassBrush : LiquidGlassBrushT<LiquidGlassBrush, implementation::LiquidGlassBrush>
	{
	};
}
