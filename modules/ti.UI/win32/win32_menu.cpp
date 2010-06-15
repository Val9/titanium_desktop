/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../menu.h"
#include "../user_window.h"

namespace ti
{
	using std::vector;
	using std::map;
	using std::string;

	void Menu::Initialize()
	{
	}

	void Menu::Shutdown()
	{
		std::vector<HMENU>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end()) {
			this->ClearNativeMenu((*i++));
		}
		nativeMenus.clear();
	}

	void Menu::AppendItemImpl(AutoMenuItem item)
	{
		AutoMenuItem win32Item = item;

		std::vector<HMENU>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end()) {
			HMENU nativeMenu = (*i++);
			Menu::InsertItemIntoNativeMenu(win32Item, nativeMenu, true);
		}

		this->oldChildren = this->children;

		if (this->nativeMenus.size() > 0) {
			UserWindow::RedrawAllMenus();
		}
	}

	void Menu::InsertItemAtImpl(AutoMenuItem item, unsigned int index)
	{
		AutoMenuItem win32Item = item;
		
		std::vector<HMENU>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			HMENU nativeMenu = (*i++);
			Menu::InsertItemIntoNativeMenu(win32Item, nativeMenu, true, index);
		}

		this->oldChildren = this->children;

		if (this->nativeMenus.size() > 0) {
			UserWindow::RedrawAllMenus();
		}
	}

	void Menu::RemoveItemAtImpl(unsigned int index)
	{
		AutoMenuItem win32Item = oldChildren.at(index);

		std::vector<HMENU>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			HMENU nativeMenu = (*i++);
			Menu::RemoveItemAtFromNativeMenu(win32Item, nativeMenu, index);
		}

		this->oldChildren = this->children;

		if (this->nativeMenus.size() > 0) {
			UserWindow::RedrawAllMenus();
		}
	}

	void Menu::ClearImpl()
	{
		std::vector<HMENU>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			HMENU nativeMenu = (*i++);
			this->ClearNativeMenu(nativeMenu);
		}

		this->oldChildren = this->children;

		if (this->nativeMenus.size() > 0) {
			UserWindow::RedrawAllMenus();
		}
	}

	/*static*/
	void Menu::InsertItemIntoNativeMenu(
		MenuItem* item, HMENU nativeMenu, bool registerNative, int position)
	{
		
		// A -1 value means to stick the new item at the end
		if (position == -1) {
			position = GetMenuItemCount(nativeMenu);
			if (position == 0) position = 1;
		}

		MENUITEMINFO itemInfo;
		item->CreateNative(&itemInfo, nativeMenu, registerNative);
		BOOL success = InsertMenuItem(nativeMenu, position, TRUE, &itemInfo);

		if (!success) {
			std::string error = Win32Utils::QuickFormatMessage(GetLastError());
			throw ValueException::FromString("Could not insert native item: " + error);
		}

	}

	/*static*/
	void Menu::RemoveItemAtFromNativeMenu(
		MenuItem* item, HMENU nativeMenu, int position)
	{
		item->DestroyNative(nativeMenu, position);

		BOOL success = DeleteMenu(nativeMenu, position, MF_BYPOSITION);
		if (!success) {
			std::string error = Win32Utils::QuickFormatMessage(GetLastError());
			throw ValueException::FromString("Could not remove native item: " + error);
		}
	}

	void Menu::ClearNativeMenu(HMENU nativeMenu)
	{
		for (int i = GetMenuItemCount(nativeMenu) - 1; i >= 0; i--) {
			AutoMenuItem win32Item =	this->oldChildren.at(i);
			RemoveItemAtFromNativeMenu(win32Item.get(), nativeMenu, i);
		}
	}

	void Menu::DestroyNative(HMENU nativeMenu)
	{
		if (!nativeMenu) {
			return;
		}

		// Remove the native menu from our list of known native menus
		std::vector<HMENU>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end()) {
			if (*i == nativeMenu)
				i = this->nativeMenus.erase(i);
			else
				i++;
		}

		// Clear the native menu and release, so that children will be freed
		this->ClearNativeMenu(nativeMenu);
	}

	HMENU Menu::CreateNative(bool registerNative)
	{
		HMENU nativeMenu = CreatePopupMenu();
		ApplyNotifyByPositionStyleToNativeMenu(nativeMenu);
		this->AddChildrenToNativeMenu(nativeMenu, registerNative);

		if (registerNative)
			this->nativeMenus.push_back(nativeMenu);

		return nativeMenu;
	}

	HMENU Menu::CreateNativeTopLevel(bool registerNative)
	{
		HMENU nativeMenu = CreateMenu();
		ApplyNotifyByPositionStyleToNativeMenu(nativeMenu);
		this->AddChildrenToNativeMenu(nativeMenu, registerNative);

		if (registerNative)
			this->nativeMenus.push_back(nativeMenu);

		return nativeMenu;
	}

	void Menu::AddChildrenToNativeMenu(HMENU nativeMenu, bool registerNative)
	{
		vector<AutoMenuItem>::iterator i = this->children.begin();
		while (i != this->children.end()) {
			AutoMenuItem item = *i++;
			AutoMenuItem win32Item = item;
			Menu::InsertItemIntoNativeMenu(win32Item.get(), nativeMenu, registerNative);
		}
	}

	/*static*/
	void Menu::ApplyNotifyByPositionStyleToNativeMenu(HMENU nativeMenu)
	{
		MENUINFO menuInfo;
		ZeroMemory(&menuInfo, sizeof(MENUINFO)); 
		menuInfo.cbSize = sizeof(MENUINFO);
		menuInfo.fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE;
		menuInfo.dwStyle = MNS_CHECKORBMP | MNS_NOTIFYBYPOS; 
		BOOL success = SetMenuInfo(nativeMenu, &menuInfo);

		if (!success) {
			std::string error = Win32Utils::QuickFormatMessage(GetLastError());
			throw ValueException::FromString("Could not set native menu style: " + error);
		}
	}

}
