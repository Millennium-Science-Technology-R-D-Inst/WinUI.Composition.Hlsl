#include "XamlWorkaround.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "NavigationWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace
{
	[[maybe_unused]] void CompileEnterpriseApiSurface(
		WinUI::Composition::Hlsl::HlslShaderLibrary const& library,
		Microsoft::UI::Composition::Compositor const& compositor)
	{
		constexpr wchar_t materializedShader[] = LR"(
float4 Shade(float2 uv, float4 samplerDataExt, float4 samplerData)
{
    return texture0.Sample(sampler0, uv + samplerDataExt.zw * 0.0f + samplerData.xy * 0.0f);
}
)";

		auto capabilities = WinUI::Composition::Hlsl::HlslComposition::GetRuntimeCapabilities();
		auto pending = WinUI::Composition::Hlsl::HlslCompiler::CompileAsync(
			materializedShader,
			WinUI::Composition::Hlsl::HlslEffectKind::MaterializedSampler,
			WinUI::Composition::Hlsl::HlslShaderProfile::Pixel40);
		auto effect = WinUI::Composition::Hlsl::HlslEffect::CreateCustomMaterializedSampler(materializedShader);
		auto compiled = WinUI::Composition::Hlsl::HlslEffect::CreateCompiledMaterializedSampler({}, library);
		auto propertyPaths = effect.GetAnimatablePropertyPaths();
		auto source = Microsoft::UI::Composition::CompositionEffectSourceParameter(effect.SourceName())
			.as<Windows::Graphics::Effects::IGraphicsEffectSource>();
		auto graph = effect.CreateGraphicsEffectWithSource(source);
		auto standardFactory = compositor.CreateEffectFactory(graph, propertyPaths);
		auto factory = WinUI::Composition::Hlsl::HlslComposition::CreateEffectFactory(compositor, effect);
		auto brush = factory.CreateBrush();
		auto nativeFactory = factory.Factory();
		auto nativeBrush = brush.EffectBrush();
		auto properties = brush.Properties();
		auto bytes = library.Bytecode();
		auto xamlBrush = WinUI::Composition::Hlsl::HlslComposition::CreateXamlBrushFromCompositionBrush(brush.Brush());
		(void)capabilities;
		(void)pending;
		(void)compiled;
		(void)graph;
		(void)standardFactory;
		(void)nativeFactory;
		(void)nativeBrush;
		(void)properties;
		(void)bytes;
		(void)xamlBrush;
	}
}

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WUILiquidGlassDemo_Hlsl::implementation
{
	/// <summary>
	/// Initializes the singleton application object.  This is the first line of authored code
	/// executed, and as such is the logical equivalent of main() or WinMain().
	/// </summary>
	App::App()
	{
		// Xaml objects should not call InitializeComponent during construction.
		// See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
		UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
						   {
							   if (IsDebuggerPresent())
							   {
								   auto errorMessage = e.Message();
								   __debugbreak();
							   }
						   });
#endif
	}

	/// <summary>
	/// Invoked when the application is launched.
	/// </summary>
	/// <param name="e">Details about the launch request and process.</param>
	void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
	{
		window = make<MainWindow>();
		auto navigationWindow = make<NavigationWindow>();
		navigationWindow.Activate();
		window.Activate();
	}
}

