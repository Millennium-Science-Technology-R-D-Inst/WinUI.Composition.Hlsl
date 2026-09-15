#pragma once

// XAML-generated translation units are ordinary textual C++ sources. Keep
// C++/WinRT named-module imports out of these TUs: the generated code includes
// textual STL headers such as <regex>, and mixing those headers with imported
// IFC/header-unit state can produce duplicate std definitions on newer MSVC.
#include "pch.h"

// windows.h still exports legacy function-like macros. C++/WinRT projections
// contain legitimate methods with the same names (Storyboard::GetCurrentTime),
// so keep those macros out of generated projection/XAML translation units.
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

// The generated XamlTypeInfo implementation constructs authored runtime-class
// implementation types. Use textual WinRT headers in this generated TU, import
// this component's own projection explicitly, then expose the authored
// implementation definitions used by XamlTypeInfo.g.cpp.
#define WINUI_LIQUID_GLASS_TEXTUAL_WINRT 1
#include "winrt_module_imports.h"
#include <winrt/WinUI.LiquidGlass.h>
#include "LiquidGlassControls.h"
#include "LiquidGlassSpecializedControls.h"
#undef WINUI_LIQUID_GLASS_TEXTUAL_WINRT
