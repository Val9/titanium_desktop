/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../tray_item.h"
#include "osx_tray_item_delegate.h"

namespace ti
{
	void TrayItem::Initialize()
	{
		nativeMenu = 0;
		nativeItem = 0;

		OSXTrayItemDelegate* delegate = [[OSXTrayItemDelegate alloc] initWithTray:this];
		NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
		nativeItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
		[nativeItem retain];
		[nativeItem setTarget:delegate];
		[nativeItem setAction:@selector(invoke:)];
		[nativeItem setHighlightMode:YES];

		this->SetIcon(this->iconPath);
	}

	void TrayItem::Shutdown()
	{
		if (!this->menu.isNull() && this->nativeMenu) {
			this->menu->DestroyNative(this->nativeMenu);
		}
	}

	void TrayItem::SetIcon(std::string& iconPath)
	{
		NSImage* image = ti::UIBinding::MakeImage(iconPath);
		[nativeItem setImage:image];
	}

	void TrayItem::SetMenu(AutoMenu menu)
	{
		if (menu.get() == this->menu.get()) {
			return;
		}

		AutoMenu newMenu = menu;
		NSMenu* newNativeMenu = nil;
		if (!newMenu.isNull()) {
			newNativeMenu = newMenu->CreateNativeNow(true);
		}

		if (!this->menu.isNull() && this->nativeMenu) {
			this->menu->DestroyNative(this->nativeMenu);
		}

		this->menu = newMenu;
		this->nativeMenu = newNativeMenu;
	}

	void TrayItem::SetHint(std::string& hint)
	{
		if (hint.empty()) {
			[nativeItem setToolTip:@""];
		} else {
			[nativeItem setToolTip:[NSString stringWithUTF8String:hint.c_str()]];
		}
	}

	void TrayItem::Remove()
	{
		[[NSStatusBar systemStatusBar] removeStatusItem:nativeItem];
		[[nativeItem target] release];
		[nativeItem release];
	}

	void TrayItem::InvokeCallback()
	{
		if (nativeMenu != nil)
		{
			[nativeItem popUpStatusItemMenu:nativeMenu];
		}

		if (callback.isNull())
			return;

		try {
			callback->Call(ValueList());
		} catch (ValueException& e) {
			Logger* logger = Logger::Get("UI.TrayItem");
			SharedString ss = e.DisplayString();
			logger->Error("Tray icon callback failed: %s", ss->c_str());
		}
	}
}
