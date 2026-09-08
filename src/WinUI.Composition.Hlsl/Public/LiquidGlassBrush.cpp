#include "pch.h"
#include "LiquidGlassBrush.h"
#include "LiquidGlassBrush.g.cpp"
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
import WinUI.Composition.Hlsl.Validation;
namespace winrt::WinUI::Composition::Hlsl::implementation
{
	using namespace Microsoft::UI::Xaml;
	LiquidGlassBrush::LiquidGlassBrush()
	{
		RegisterPropertyChangedCallback(Media::XamlCompositionBrushBase::FallbackColorProperty(), [this](auto const&, auto const&)
										{
											Update();
										});
	}
	void LiquidGlassBrush::Changed(DependencyObject const& object, DependencyPropertyChangedEventArgs const&)
	{
		get_self<LiquidGlassBrush>(object.as<Hlsl::LiquidGlassBrush>())->Update();
	}
	DependencyProperty LiquidGlassBrush::IsEnabledProperty()
	{
		static auto property=DependencyProperty::Register(L"IsEnabled", Windows::UI::Xaml::Interop::TypeName{ L"Boolean",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(true), PropertyChangedCallback{ Changed }));
		return property;
	}
	bool LiquidGlassBrush::IsEnabled() const
	{
		return unbox_value<bool>(GetValue(IsEnabledProperty()));
	}
	void LiquidGlassBrush::IsEnabled(bool value)
	{
		SetValue(IsEnabledProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::BlurRadiusProperty()
	{
		static auto property=DependencyProperty::Register(L"BlurRadius", Windows::UI::Xaml::Interop::TypeName{ L"Single",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(12.0f), PropertyChangedCallback{ Changed }));
		return property;
	}
	float LiquidGlassBrush::BlurRadius() const
	{
		return unbox_value<float>(GetValue(BlurRadiusProperty()));
	}
	void LiquidGlassBrush::BlurRadius(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(BlurRadiusProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::RefractionStrengthProperty()
	{
		static auto property=DependencyProperty::Register(L"RefractionStrength", Windows::UI::Xaml::Interop::TypeName{ L"Single",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(24.0f), PropertyChangedCallback{ Changed }));
		return property;
	}
	float LiquidGlassBrush::RefractionStrength() const
	{
		return unbox_value<float>(GetValue(RefractionStrengthProperty()));
	}
	void LiquidGlassBrush::RefractionStrength(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(RefractionStrengthProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::DispersionStrengthProperty()
	{
		static auto property=DependencyProperty::Register(L"DispersionStrength", Windows::UI::Xaml::Interop::TypeName{ L"Single",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(1.2f), PropertyChangedCallback{ Changed }));
		return property;
	}
	float LiquidGlassBrush::DispersionStrength() const
	{
		return unbox_value<float>(GetValue(DispersionStrengthProperty()));
	}
	void LiquidGlassBrush::DispersionStrength(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(DispersionStrengthProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::CornerRadiusProperty()
	{
		static auto property=DependencyProperty::Register(L"CornerRadius", Windows::UI::Xaml::Interop::TypeName{ L"Single",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(12.0f), PropertyChangedCallback{ Changed }));
		return property;
	}
	float LiquidGlassBrush::CornerRadius() const
	{
		return unbox_value<float>(GetValue(CornerRadiusProperty()));
	}
	void LiquidGlassBrush::CornerRadius(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(CornerRadiusProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::BorderThicknessProperty()
	{
		static auto property=DependencyProperty::Register(L"BorderThickness", Windows::UI::Xaml::Interop::TypeName{ L"Single",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(1.0f), PropertyChangedCallback{ Changed }));
		return property;
	}
	float LiquidGlassBrush::BorderThickness() const
	{
		return unbox_value<float>(GetValue(BorderThicknessProperty()));
	}
	void LiquidGlassBrush::BorderThickness(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(BorderThicknessProperty(), box_value(value));
	}
	DependencyProperty LiquidGlassBrush::HighlightStrengthProperty()
	{
		static auto property=DependencyProperty::Register(L"HighlightStrength", Windows::UI::Xaml::Interop::TypeName{ L"Single",Windows::UI::Xaml::Interop::TypeKind::Primitive }, Windows::UI::Xaml::Interop::TypeName{ L"WinUI.Composition.Hlsl.LiquidGlassBrush",Windows::UI::Xaml::Interop::TypeKind::Metadata }, PropertyMetadata(box_value(0.8f), PropertyChangedCallback{ Changed }));
		return property;
	}
	float LiquidGlassBrush::HighlightStrength() const
	{
		return unbox_value<float>(GetValue(HighlightStrengthProperty()));
	}
	void LiquidGlassBrush::HighlightStrength(float value)
	{
		if (!hlsl::validation::IsFiniteNonNegative(value) || value > 256) throw hresult_invalid_argument(L"Material values must be between 0 and 256.");
		SetValue(HighlightStrengthProperty(), box_value(value));
	}
	void LiquidGlassBrush::OnConnected()
	{
		m_connected=true; m_queue=Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
		auto weak=get_weak();
		auto changed=[weak](auto const&, auto const&)
			{
				if (auto self=weak.get()) self->m_queue.TryEnqueue([weak]
																   {
																	   if (auto owner=weak.get())owner->Update();
																   });
			};
		m_contrast=m_accessibility.HighContrastChanged(auto_revoke, changed);
		m_effects=m_ui.AdvancedEffectsEnabledChanged(auto_revoke, changed);
		Update();
	}
	void LiquidGlassBrush::OnDisconnected()
	{
		m_connected=false; m_contrast.revoke(); m_effects.revoke();
		CompositionBrush(nullptr); m_material=nullptr;
	}
	void LiquidGlassBrush::Update()
	{
		if (!m_connected) return;
		auto compositor=Media::CompositionTarget::GetCompositorForCurrentThread();
		if (!IsEnabled() || m_accessibility.HighContrast() || !m_ui.AdvancedEffectsEnabled())
		{
			CompositionBrush(compositor.CreateColorBrush(FallbackColor())); m_material=nullptr; return;
		}
		try
		{
			if (!m_material)m_material=Hlsl::LiquidGlassMaterial(compositor);
			m_material.BlurRadius(BlurRadius());
			m_material.RefractionStrength(RefractionStrength());
			m_material.DispersionStrength(DispersionStrength());
			m_material.CornerRadius(CornerRadius());
			m_material.BorderThickness(BorderThickness());
			m_material.HighlightStrength(HighlightStrength());
			CompositionBrush(m_material.EffectBrush().Brush());
		}
		catch (hresult_error const& error)
		{
			OutputDebugStringW(error.message().c_str());
			CompositionBrush(compositor.CreateColorBrush(FallbackColor())); m_material=nullptr;
		}
	}
}


