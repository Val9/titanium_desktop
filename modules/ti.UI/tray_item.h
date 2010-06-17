/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TRAY_ITEM_H_
#define _TRAY_ITEM_H_

#include "ui.h"
#include "menu_item.h"
#include "menu.h"

namespace ti
{
	class TrayItem : public KEventObject
	{

	public:
		TrayItem(std::string& iconURL, KMethodRef cb);
		~TrayItem();

		AutoMenu GetMenu() { return this->menu; }
		KMethodRef GetCallback() { return this->callback; }

		// Platform implementations
		void Initialize();
		void Shutdown();
		void SetIcon(std::string& iconPath);
		void SetMenu(AutoMenu menu);
		void SetHint(std::string& hint);
		void Remove();
		void InvokeCallback();

		void _SetIcon(const ValueList& args, KValueRef result);
		void _SetMenu(const ValueList& args, KValueRef result);
		void _SetHint(const ValueList& args, KValueRef result);
		void _GetIcon(const ValueList& args, KValueRef result);
		void _GetMenu(const ValueList& args, KValueRef result);
		void _GetHint(const ValueList& args, KValueRef result);
		void _Remove(const ValueList& args, KValueRef result);

#ifdef OS_LINUX
		GtkStatusIcon* GetWidget();
#endif

#ifdef OS_WIN32
		void ShowTrayMenu();
		void HandleRightClick();
		void HandleLeftClick();
		void HandleDoubleLeftClick();
		UINT GetId();
		static bool MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK DoubleClickTimerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

	private:
#ifdef OS_LINUX
		GtkStatusIcon* item;
		bool active;
#endif

#ifdef OS_OSX
		NSMenu* nativeMenu;
		NSStatusItem* nativeItem;
#endif

#ifdef OS_WIN32
		HMENU oldNativeMenu;
		NOTIFYICONDATA* trayIconData;
		static std::vector<AutoTrayItem> trayItems;
		static UINT trayClickedMessage;
		bool is_double_clicked;
#endif

		AutoMenu menu;
		std::string iconURL;
		std::string iconPath;
		std::string hint;
		bool removed;
		KMethodRef callback;
	};
}

#endif
