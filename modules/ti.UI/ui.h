/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _UI_H_
#define _UI_H_

#include <iostream>

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>

#include <kroll/kroll.h>
#include <kroll/javascript/javascript_module.h>
#include "url/url.h"
#include "app_config.h"
#include "window_config.h"

namespace ti {
	class UIBinding;
	class Menu;
	class MenuItem;
	class TrayItem;
	class UserWindow;

	typedef AutoPtr<UserWindow> AutoUserWindow;
	typedef AutoPtr<UIBinding> AutoUIBinding;
	typedef AutoPtr<Menu> AutoMenu;
	typedef AutoPtr<MenuItem> AutoMenuItem;
	typedef AutoPtr<TrayItem> AutoTrayItem;
}

#ifdef OS_LINUX
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <webkit/webkit.h>
#include <webkit/webkittitanium.h>

// X11 defines this again for some reason
#undef DisplayString
#endif

#ifdef OS_OSX
#include <Carbon/Carbon.h>
#include <WebKit/WebKit.h>
#include "osx/ti_application_delegate.h"
#include "osx/titanium_protocols.h"
#endif

#ifdef OS_WIN32
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <cderr.h>
#include <comutil.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shlobj.h>
#include <winbase.h>

// We need to do some define hacks here to make comdef work.
// Touch this ordering, and things will break!
#define FormatMessage FormatMessageW
#include <comdef.h>
#undef IsMaximized
#undef IsMinimized

#include <WebKit/WebKit.h>
#include <WebKit/WebKitCOMAPI.h>
#include <WebKit/WebKitTitanium.h>
#include <cairo/cairo.h>
#include <cairo/cairo-win32.h>
#endif

#endif
