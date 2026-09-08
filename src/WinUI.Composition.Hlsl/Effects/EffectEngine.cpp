#include "pch.h"
#include "EffectEngine.h"
#include "CustomEffectRuntime.h"
#include <list>
#include <memory>
#include <string>

namespace hlsl::engine
{
	namespace
	{
		struct Program
		{
			winrt::guid id;
			std::string code;
			std::string original;
			bool samplerMode;
			CustomEffectRuntime::SourceDescriptor source{ L"Backdrop",CustomEffectRuntime::SourceKind::Backdrop,false,false };
			uint16_t arguments[2]{ 0x0200,0 };
			CustomEffectRuntime::CustomEffectDefinition definition{};
			Program(winrt::guid const& key, std::string_view text, bool sampler) :id(key), code(text), original(text), samplerMode(sampler)
			{
				if (samplerMode)
				{
					code="Texture2D texture0; SamplerState sampler0;\n" + code;
					for (auto suffix : { "","CC","CW","CM","WC","WW","WM","MC","MW","MM","C","W","M" })
						code+="\nexport float4 PSBody" + std::string(suffix) + "(float2 uv,float4 info){return Shade(uv,info);}\n";
					arguments[0]=0x0100; arguments[1]=0x0400;
					source.requiresSamplerDataExt=true;
				}
				definition.id=id;
				definition.effectName=L"HlslEffect";
				definition.fragmentName="AppHlslEffect";
				definition.shaderSource=code.c_str();
				definition.shaderSourceSize=code.size();
				definition.shaderFunctionName="PSBody";
				definition.sources=&source;
				definition.sourceCount=1;
				definition.shaderArguments=arguments;
				definition.shaderArgumentCount=samplerMode ? 2 : 1;
				definition.linkingArgType=samplerMode ? 0x0200 : 0;
				definition.shaderProfileVersion=CustomEffectRuntime::kShaderProfileLevel93;
			}
		};
		std::mutex programsMutex;
		// Intentionally process-lived: native effect entries retain definition pointers.
		auto& Programs()
		{
			static auto* values=new std::list<std::unique_ptr<Program>>; return *values;
		}

		struct XamlBrush : winrt::Microsoft::UI::Xaml::Media::XamlCompositionBrushBaseT<XamlBrush>
		{
			winrt::Microsoft::UI::Composition::CompositionBrush m_brush{ nullptr };
			explicit XamlBrush(winrt::Microsoft::UI::Composition::CompositionBrush const& brush) :m_brush(brush)
			{
			}
			void OnConnected()
			{
				CompositionBrush(m_brush);
			}
			void OnDisconnected()
			{
				CompositionBrush(nullptr);
			}
		};
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateProgram(winrt::guid const& id, std::string_view hlsl, bool sampler)
	{
		if (hlsl.empty() || hlsl.find('\0') != std::string_view::npos)
			throw winrt::hresult_invalid_argument(L"HLSL source is empty or contains NUL.");
		std::lock_guard lock(programsMutex);
		for (auto const& program : Programs())
		{
			if (program->id == id)
			{
				if (program->original != hlsl || program->samplerMode != sampler) throw winrt::hresult_invalid_argument(L"This HLSL effect GUID already has different source.");
				return CustomEffectRuntime::CreateEffect(program->definition);
			}
		}
		auto program=std::make_unique<Program>(id, hlsl, sampler);
		// Keep ownership even if installation fails: registration may already retain the definition.
		auto* registered=program.get();
		Programs().push_back(std::move(program));
		return CustomEffectRuntime::CreateEffect(registered->definition);
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateColorEffect(winrt::guid const& id, std::string_view hlsl)
	{
		return CreateProgram(id, hlsl, false);
	}
	winrt::Windows::Graphics::Effects::IGraphicsEffect CreateSamplerEffect(winrt::guid const& id, std::string_view hlsl)
	{
		return CreateProgram(id, hlsl, true);
	}
	winrt::Microsoft::UI::Composition::CompositionEffectBrush CreateBackdropBrush(winrt::Microsoft::UI::Composition::Compositor const& compositor, winrt::Windows::Graphics::Effects::IGraphicsEffect const& effect)
	{
		if (!compositor || !effect) throw winrt::hresult_invalid_argument();
		auto brush=compositor.CreateEffectFactory(effect).CreateBrush();
		brush.SetSourceParameter(L"Backdrop", compositor.CreateBackdropBrush());
		return brush;
	}
	winrt::Microsoft::UI::Xaml::Media::XamlCompositionBrushBase AsXamlBrush(winrt::Microsoft::UI::Composition::CompositionBrush const& brush)
	{
		if (!brush) throw winrt::hresult_invalid_argument();
		return winrt::make<XamlBrush>(brush);
	}
}

