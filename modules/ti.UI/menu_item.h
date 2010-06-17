/**
* Appcelerator Titanium - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_

#include "ui.h"
#include "menu.h"

namespace ti
{
	class MenuItem : public KEventObject
	{
	public:
		enum MenuItemType
		{
			NORMAL,
			SEPARATOR,
			CHECK
		};

		MenuItem(MenuItemType type);
		~MenuItem();

		void _IsSeparator(const ValueList& args, KValueRef result);
		void _IsCheck(const ValueList& args, KValueRef result);
		void _SetLabel(const ValueList& args, KValueRef result);
		void _GetLabel(const ValueList& args, KValueRef result);
		void _SetIcon(const ValueList& args, KValueRef result);
		void _GetIcon(const ValueList& args, KValueRef result);
		void _SetState(const ValueList& args, KValueRef result);
		void _GetState(const ValueList& args, KValueRef result);
		void _SetAutoCheck(const ValueList& args, KValueRef result);
		void _IsAutoCheck(const ValueList& args, KValueRef result);
		void _SetSubmenu(const ValueList& args, KValueRef result);
		void _GetSubmenu(const ValueList& args, KValueRef result);
		void _Enable(const ValueList& args, KValueRef result);
		void _Disable(const ValueList& args, KValueRef result);
		void _IsEnabled(const ValueList& args, KValueRef result);
		void _Click(const ValueList& args, KValueRef result);

		void _AddItem(const ValueList& args, KValueRef result);
		void _AddSeparatorItem(const ValueList& args, KValueRef result);
		void _AddCheckItem(const ValueList& args, KValueRef result);

		void SetLabel(string& label);
		std::string& GetLabel();
		void SetIcon(string& iconURL);
		bool GetState();
		void SetState(bool);
		bool IsSeparator();
		bool IsCheck();
		bool IsEnabled();
		void HandleClickEvent(KObjectRef source);
		void EnsureHasSubmenu();
		bool ContainsItem(MenuItem* item);
		bool ContainsSubmenu(Menu* submenu);

		// Platform-specific implementation
		void SetLabelImpl(std::string newLabel);
		void SetIconImpl(std::string newIconPath);
		void SetStateImpl(bool newState);
		void SetSubmenuImpl(AutoMenu newSubmenu);
		void SetEnabledImpl(bool enabled);

#ifdef OS_LINUX
		void ReplaceNativeItem(GtkMenuItem* nativeItem, GtkMenuItem* newNativeItem);
		void SetNativeItemIcon(GtkMenuItem* nativeItem, std::string& newIconPath);
		void SetNativeItemSubmenu(GtkMenuItem* nativeItem, AutoMenu newSubmenu);
		GtkMenuItem* CreateNative(bool registerNative);
		void DestroyNative(GtkMenuItem* nativeItem);
#endif

#ifdef OS_OSX
		NSMenuItem* CreateNative(bool registerNative=true);
		void DestroyNative(NSMenuItem* realization);
		void UpdateNativeMenuItems();
#endif

#ifdef OS_WIN32
		// Pieces of information we can use to do a reverse lookup
		// on an item's position in a parent menu.
		struct NativeItemBits
		{
			UINT id;
			HMENU parentMenu;
			HMENU submenu;
		};

		void RecreateAllNativeItems();
		void RecreateMenuItem(NativeItemBits* bits);
		void CreateNative(LPMENUITEMINFO itemInfo,
			HMENU nativeParentMenu, bool registerNative);
		void DestroyNative(NativeItemBits* bits);
		void DestroyNative(HMENU nativeParent, int position);
		static int GetNativeMenuItemPosition(NativeItemBits* bits);
		static bool HandleClickEvent(HMENU nativeMenu, UINT position);
#endif

	private:
#ifdef OS_LINUX
		std::vector<GtkMenuItem*> nativeItems;
#endif

#ifdef OS_OSX
		static void SetNSMenuItemTitle(NSMenuItem* item, std::string& title);
		static void SetNSMenuItemState(NSMenuItem* item, bool state);
		static void SetNSMenuItemIconPath(
			NSMenuItem* item, std::string& iconPath, NSImage* image = nil);
		static void SetNSMenuItemSubmenu(
			NSMenuItem* item, AutoMenu submenu, bool registerNative=true);
		static void SetNSMenuItemEnabled(NSMenuItem* item, bool enabled);

		std::vector<NSMenuItem*> nativeItems;
#endif

#ifdef OS_WIN32
		std::wstring wideOldLabel;
		std::vector<NativeItemBits*> nativeItems;
#endif

		MenuItemType type;
		bool enabled;
		std::string label;
		std::string iconURL;
		std::string iconPath;
		KMethodRef callback;
		AutoMenu submenu;
		AutoMenu oldSubmenu;
		std::vector<KMethodRef> eventListeners;
		bool state;
		bool autoCheck;
	};
}
#endif
