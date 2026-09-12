#pragma once

// XAML-generated translation units do not understand C++/WinRT named modules yet.
// This preamble is injected with /FI into generated sources and is also included
// before *.g.h from authored XAML translation units.
//
// Important: parse Win32/COM and ordinary C++ headers BEFORE importing WinRT
// modules. Do not fake Windows/RPC include guards: doing so can suppress required
// declarations from rpcndr.h/unknwn.h while making downstream headers believe
// those declarations already exist.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <unknwn.h>
#include <shobjidl.h>
#include <microsoft.ui.xaml.window.h>

// Include the STL headers used by the authored/generated XAML code before any
// module import. MSVC supports include-then-import; the reverse ordering can
// produce duplicate declaration diagnostics when a later header is included.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// Once the projection modules are imported, generated #include <winrt/...>
// directives must become inert so the same WinRT declarations are not parsed
// textually a second time.
#ifndef WINRT_IMPORT_MODULE
#define WINRT_IMPORT_MODULE
#endif

import winrt.Windows.Foundation;
import winrt.Windows.Foundation.Collections;
import winrt.Windows.Foundation.Numerics;
import winrt.Windows.Graphics.Effects;
import winrt.Windows.UI;
import winrt.Windows.ApplicationModel.Activation;
import winrt.Windows.ApplicationModel.DataTransfer;
import winrt.Windows.Storage;
import winrt.Windows.Storage.Pickers;
import winrt.Windows.Storage.Streams;

import winrt.Microsoft.UI.Content;
import winrt.Microsoft.UI.Input;
import winrt.Microsoft.UI.Composition;
import winrt.Microsoft.UI.Dispatching;
import winrt.Microsoft.UI.Xaml;
import winrt.Microsoft.UI.Xaml.Controls;
import winrt.Microsoft.UI.Xaml.Controls.Primitives;
import winrt.Microsoft.UI.Xaml.Data;
import winrt.Microsoft.UI.Xaml.Hosting;
import winrt.Microsoft.UI.Xaml.Input;
import winrt.Microsoft.UI.Xaml.Interop;
import winrt.Microsoft.UI.Xaml.Markup;
import winrt.Microsoft.UI.Xaml.Media;
import winrt.Microsoft.UI.Xaml.Media.Animation;
import winrt.Microsoft.UI.Xaml.Media.Imaging;
import winrt.Microsoft.UI.Xaml.Navigation;
import winrt.Microsoft.UI.Xaml.Shapes;
import winrt.Microsoft.UI.Xaml.XamlTypeInfo;
