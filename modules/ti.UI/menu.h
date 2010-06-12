/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _MENU_H_
#define _MENU_H_

#include "ui_module.h"
#include "menu_item.h"

namespace ti
{
	class Menu : public KAccessorObject
	{
	public:
		Menu();
		~Menu();

		void _AppendItem(const ValueList& args, KValueRef result);
		void _GetItemAt(const ValueList& args, KValueRef result);
		void _InsertItemAt(const ValueList& args, KValueRef result);
		void _RemoveItemAt(const ValueList& args, KValueRef result);
		void _GetLength(const ValueList& args, KValueRef result);
		void _Clear(const ValueList& args, KValueRef result);

		void _AddItem(const ValueList& args, KValueRef result);
		void _AddSeparatorItem(const ValueList& args, KValueRef result);
		void _AddCheckItem(const ValueList& args, KValueRef result);

		void AppendItem(AutoMenuItem item);
		AutoMenuItem GetItemAt(int index);
		void InsertItemAt(AutoMenuItem item, size_t index);
		void RemoveItemAt(size_t index);
		bool ContainsItem(MenuItem* item);
		bool ContainsSubmenu(Menu* submenu);

		// Platform-specific implementation
		void Initialize();
		void Shutdown();
		void AppendItemImpl(AutoMenuItem item);
		void InsertItemAtImpl(AutoMenuItem item, unsigned int index);
		void RemoveItemAtImpl(unsigned int index);
		void ClearImpl();

#ifdef OS_OSX
		NSMenu* CreateNativeNow(bool registerMenu=true);
		NSMenu* CreateNativeLazily(bool registerMenu=true);
		void FillNativeMainMenu(NSMenu* defaultMenu, NSMenu* nativeMainMenu);
		static void CopyMenu(NSMenu* from, NSMenu* to);
		static NSMenuItem* CopyMenuItem(NSMenuItem* item);

		void AddChildrenToNativeMenu(
			NSMenu* nativeMenu, bool registerMenu=true, bool isMainMenu=false);
		void AddChildrenToNSArray(NSMutableArray* array);
		void DestroyNative(NSMenu* nativeMenu);
		void UpdateNativeMenus();
		static void ClearNativeMenu(NSMenu* nativeMenu);
		static void UpdateNativeMenu(NSMenu* nativeMenu);
		static NSMenu* GetWindowMenu(NSMenu* nsMenu);
		static NSMenu* GetAppleMenu(NSMenu* nsMenu);
		static NSMenu* GetServicesMenu(NSMenu* nsMenu);
		static void EnsureAllItemsHaveSubmenus(NSMenu* menu);
		static void FixWindowMenu(NSMenu* menu);
		static bool IsNativeMenuAMainMenu(NSMenu* menu);
		static void SetupInspectorItem(NSMenu* menu);
		static void ReplaceAppNameStandinInMenu(NSMenu* menu, NSString* appName);
#endif

	protected:
		std::vector<AutoMenuItem> children;

	private:
#ifdef OS_OSX
		void Clear();
		NSMenu* CreateNative(bool lazy, bool registerMenu);
		std::vector<NSMenu*> nativeMenus;
#endif
	};
}
#endif
