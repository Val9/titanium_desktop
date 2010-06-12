/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _UI_BINDING_H_
#define _UI_BINDING_H_

#include "ui_module.h"
#include "window_config.h"
#include "clipboard.h"
#include "menu.h"
#include "menu_item.h"
#include "tray_item.h"
#include "notification.h"
#include "user_window.h"

namespace ti
{
	class UIBinding : public KAccessorObject
	{

	public:
		UIBinding(Host *host);
		virtual ~UIBinding();
		Host* GetHost();
		AutoMenu GetActiveMenu() { return activeMenu; }

		void CreateMainWindow(AutoPtr<WindowConfig> config);
		AutoUserWindow GetMainWindow();
		std::vector<AutoUserWindow>& GetOpenWindows();
		void AddToOpenWindows(AutoUserWindow);
		void RemoveFromOpenWindows(AutoUserWindow);
		void ClearTray();
		void UnregisterTrayItem(TrayItem*);
		void _GetOpenWindows(const ValueList& args, KValueRef result);
		void _GetMainWindow(const ValueList& args, KValueRef result);
		void _CreateWindow(const ValueList& args, KValueRef result);
		void _CreateNotification(const ValueList& args, KValueRef result);
		void _CreateMenu(const ValueList& args, KValueRef result);
		void _CreateMenuItem(const ValueList& args, KValueRef result);
		void _CreateCheckMenuItem(const ValueList& args, KValueRef result);
		void _CreateSeparatorMenuItem(const ValueList& args, KValueRef result);
		AutoMenuItem __CreateMenuItem(const ValueList& args);
		AutoMenuItem __CreateCheckMenuItem(const ValueList& args);
		AutoMenuItem __CreateSeparatorMenuItem(const ValueList& args);
		void _SetMenu(const ValueList& args, KValueRef result);
		void _GetMenu(const ValueList& args, KValueRef result);
		void _SetContextMenu(const ValueList& args, KValueRef result);
		void _GetContextMenu(const ValueList& args, KValueRef result);
		void _SetIcon(const ValueList& args, KValueRef result);
		void _SetIcon(std::string iconURL);
		void _AddTray(const ValueList& args, KValueRef result);
		void _ClearTray(const ValueList& args, KValueRef result);
		void _GetIdleTime(const ValueList& args, KValueRef result);

		AutoMenu GetContextMenu() { return contextMenu; }

#ifdef OS_OSX
		void _SetDockIcon(const ValueList& args, KValueRef result);
		void _SetDockMenu(const ValueList& args, KValueRef result);
		void _SetBadge(const ValueList& args, KValueRef result);
		void _SetBadgeImage(const ValueList& args, KValueRef result);

		void SetDockIcon(std::string& icon_path);
		void SetDockMenu(AutoMenu);
		AutoMenu GetDockMenu();
		void SetBadge(std::string& badgeLabel);
		void SetBadgeImage(std::string& badgeImagePath);
		void WindowFocused(AutoUserWindow window);
		void WindowUnfocused(AutoUserWindow window);
		void SetupAppMenuParts(NSMenu* mainMenu);

		static NSImage* MakeImage(std::string&);
#endif

		// Platform implementations
		void Initialize();
		void Shutdown();
		void SetupMainMenu(bool force = false);
		void SetIcon(std::string& iconPath);
		long GetIdleTime();

		static void ErrorDialog(std::string);
		static inline UIBinding* GetInstance() { return instance; }

	protected:
#ifdef OS_OSX
		AutoMenu dockMenu;
		NSMenu* defaultMenu;
		NSMenu* nativeMenu;
		NSMenu* nativeDockMenu;
		NSView* savedDockView;
		NSObject* application;
#endif

		static UIBinding* instance;
		Host* host;
		AutoUserWindow mainWindow;
		AutoUserWindow activeWindow;
		std::vector<AutoUserWindow> openWindows;
		std::vector<AutoTrayItem> trayItems;
		std::string iconURL;
		AutoMenu menu;
		AutoMenu contextMenu;
		AutoMenu activeMenu;

		static void Log(Logger::Level level, std::string& message);
	};
}

#endif
