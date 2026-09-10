#include "pch.h"
#include "App.xaml.h"

#include <fstream>
using namespace winrt;
using namespace Microsoft::UI::Xaml;
namespace winrt::HlslCppConsumer::implementation
{
	App::App()
	{
	}
	void App::OnLaunched(LaunchActivatedEventArgs const&)
	{
		std::ofstream("cpp-results.txt") << "Started\n";
		try
		{
			auto literal=Markup::XamlReader::Load(LR"(<Border xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:hlsl="using:WinUI.Composition.Hlsl"><Border.Background><hlsl:LiquidGlassBrush IsEnabled="False" BlurRadius="10" RefractionStrength="16"/></Border.Background></Border>)").as<Controls::Border>();
			auto brush=literal.Background().as<WinUI::Composition::Hlsl::LiquidGlassBrush>();
			if (brush.BlurRadius() != 10 || brush.RefractionStrength() != 16)throw hresult_error(E_FAIL, L"XAML numeric conversion failed.");
			std::ofstream("cpp-results.txt", std::ios::app) << "PASS: XAML numeric literals\n";
			m_border=Markup::XamlReader::Load(LR"(<Border xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" Width="400" Height="240" Background="{ThemeResource TestBrush}"/>)").as<Controls::Border>();
			m_border.RequestedTheme(ElementTheme::Light);
			m_window=Window(); m_window.Title(L"HLSL C++ consumer tests"); m_window.Content(m_border); m_window.Activate();
			m_timer=Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread().CreateTimer();
			m_timer.Interval(std::chrono::milliseconds(500));
			m_timer.Tick([this, phase=0](auto const&, auto const&) mutable
						 {
							 try
							 {
								 auto current=m_border.Background().as<WinUI::Composition::Hlsl::LiquidGlassBrush>();
								 if (phase == 0)
								 {
									 if (current.BlurRadius() != 10)throw hresult_error(E_FAIL, L"Light resource"); m_border.RequestedTheme(ElementTheme::Dark);
								 }
								 else if (phase == 1)
								 {
									 if (current.BlurRadius() != 16)throw hresult_error(E_FAIL, L"Dark resource"); current.IsEnabled(true);
								 }
								 else if (phase == 2)
								 {
									 Microsoft::UI::Composition::CompositionCapabilities capabilities; if (capabilities.AreEffectsSupported() && !current.as<Media::IXamlCompositionBrushBaseProtected>().CompositionBrush().try_as<Microsoft::UI::Composition::CompositionEffectBrush>())throw hresult_error(E_FAIL, L"Material unexpectedly fell back.");
								 }
								 else if (phase == 3)
								 {
									 current.IsEnabled(false); std::ofstream("cpp-results.txt", std::ios::app) << "PASS: Light/Dark + live material toggle\nALL PASS\n"; m_timer.Stop(); m_window.Close();
								 }
								 ++phase;
							 }
							 catch (hresult_error const& e)
							 {
								 std::ofstream("cpp-results.txt", std::ios::app) << "FAIL " << to_string(e.message()); m_timer.Stop(); m_window.Close();
							 }
						 }); m_timer.Start();
		}
		catch (hresult_error const& e)
		{
			std::ofstream("cpp-results.txt", std::ios::app) << "FAIL " << to_string(e.message()); Exit();
		}
	}
}


int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	winrt::init_apartment(winrt::apartment_type::single_threaded);
	winrt::Microsoft::UI::Xaml::Application::Start([](auto&&)
												   {
													   auto app=winrt::make_self<winrt::HlslCppConsumer::implementation::App>();
													   app->InitializeComponent();
												   });
	return 0;
}
