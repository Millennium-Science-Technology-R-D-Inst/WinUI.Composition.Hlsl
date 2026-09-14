#pragma once
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif
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
		double BezelWidth() const;
		void BezelWidth(double value);
		static Microsoft::UI::Xaml::DependencyProperty BezelWidthProperty();
		double GlassThickness() const;
		void GlassThickness(double value);
		static Microsoft::UI::Xaml::DependencyProperty GlassThicknessProperty();
		double RefractiveIndex() const;
		void RefractiveIndex(double value);
		static Microsoft::UI::Xaml::DependencyProperty RefractiveIndexProperty();
		double TintOpacity() const;
		void TintOpacity(double value);
		static Microsoft::UI::Xaml::DependencyProperty TintOpacityProperty();
		double Saturation() const;
		void Saturation(double value);
		static Microsoft::UI::Xaml::DependencyProperty SaturationProperty();
		double LightAngle() const;
		void LightAngle(double value);
		static Microsoft::UI::Xaml::DependencyProperty LightAngleProperty();
		Hlsl::LiquidGlassSurfaceProfile SurfaceProfile() const;
		void SurfaceProfile(Hlsl::LiquidGlassSurfaceProfile value);
		static Microsoft::UI::Xaml::DependencyProperty SurfaceProfileProperty();
		double MagnificationStrength() const;
		void MagnificationStrength(double value);
		static Microsoft::UI::Xaml::DependencyProperty MagnificationStrengthProperty();
		double HighlightSharpness() const;
		void HighlightSharpness(double value);
		static Microsoft::UI::Xaml::DependencyProperty HighlightSharpnessProperty();
		double TintRed() const;
		void TintRed(double value);
		static Microsoft::UI::Xaml::DependencyProperty TintRedProperty();
		double TintGreen() const;
		void TintGreen(double value);
		static Microsoft::UI::Xaml::DependencyProperty TintGreenProperty();
		double TintBlue() const;
		void TintBlue(double value);
		static Microsoft::UI::Xaml::DependencyProperty TintBlueProperty();
		double InnerShadowStrength() const;
		void InnerShadowStrength(double value);
		static Microsoft::UI::Xaml::DependencyProperty InnerShadowStrengthProperty();
		double SpecularSaturation() const;
		void SpecularSaturation(double value);
		static Microsoft::UI::Xaml::DependencyProperty SpecularSaturationProperty();
		double SpecularWidth() const;
		void SpecularWidth(double value);
		static Microsoft::UI::Xaml::DependencyProperty SpecularWidthProperty();

	private:
		void Update();
		friend class hlsl::xaml::XamlHlslBrushBase<LiquidGlassBrush>;
		Microsoft::UI::Composition::CompositionBrush BuildPipeline(Microsoft::UI::Composition::Compositor const&);
		void ReleasePipeline() noexcept { m_material = nullptr; }
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
