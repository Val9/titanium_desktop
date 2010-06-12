/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../menu_item.h"
#include "osx_menu_item_delegate.h"

namespace ti
{
	void MenuItem::SetLabelImpl(std::string newLabel)
	{
		if (this->type == SEPARATOR)
			return;
		this->UpdateNativeMenuItems();
	}

	void MenuItem::SetIconImpl(std::string newIconPath)
	{
		if (this->type == SEPARATOR || this->type == CHECK)
			return;
		this->UpdateNativeMenuItems();
	}

	void MenuItem::SetStateImpl(bool newState)
	{
		if (this->type != CHECK)
			return;
		this->UpdateNativeMenuItems();
	}

	void MenuItem::SetSubmenuImpl(AutoMenu newSubmenu)
	{
		if (this->type == SEPARATOR)
			return;
		this->UpdateNativeMenuItems();
	}

	void MenuItem::SetEnabledImpl(bool enabled)
	{
		if (this->type == SEPARATOR)
			return;
		this->UpdateNativeMenuItems();
	}

	/*static*/
	void MenuItem::SetNSMenuItemTitle(NSMenuItem* item, std::string& title)
	{
		NSString* nstitle = [NSString stringWithUTF8String:title.c_str()];
		[item setTitle:nstitle];

		NSMenu* submenu = [item submenu];
		if (submenu != nil)
		{
			// Need to set the new native menu's title as this item's label. Each
			// native menu will have to use the title of the item it is attached to.
			[submenu setTitle:nstitle];
		}
		if ([item menu] != nil)
		{
			[[item menu] sizeToFit];
		}
	}

	/*static*/
	void MenuItem::SetNSMenuItemIconPath(
		NSMenuItem* item, std::string& iconPath, NSImage* image)
	{
		bool needsRelease = false;

		// If we weren't passed an image, create one for this call. This
		// allows callers to do one image creation in cases where the same
		// image is used over and over again.
		if (image == nil) {
			image = UIBinding::MakeImage(iconPath);
			needsRelease = true;
		}

		if (!iconPath.empty()) {
			[item setImage:image];
		} else {
			[item setImage:nil];
		}

		if ([item menu] != nil) {
			[[item menu] sizeToFit];
		}

		if (needsRelease) {
			[image release];
		}
	}

	/*static*/
	void MenuItem::SetNSMenuItemState(NSMenuItem* item, bool state)
	{
		[item setState:state ? NSOnState : NSOffState];
	}

	/*static*/
	void MenuItem::SetNSMenuItemSubmenu(
		NSMenuItem* item, AutoMenu submenu, bool registerNative)
	{
		if (!submenu.isNull()) {
			AutoPtr<Menu> osxSubmenu = submenu.cast<Menu>();
			NSMenu* nativeMenu = osxSubmenu->CreateNativeLazily(registerNative);
			[nativeMenu setTitle:[item title]];
			[item setSubmenu:nativeMenu];

		} else {
			[item setSubmenu:nil];
		}
	}

	/*static*/
	void MenuItem::SetNSMenuItemEnabled(NSMenuItem* item, bool enabled)
	{
		[item setEnabled:(enabled ? YES : NO)];
	}

	NSMenuItem* MenuItem::CreateNative(bool registerNative)
	{
		if (this->IsSeparator()) {
			return [NSMenuItem separatorItem];
		} else {
			NSMenuItem* item = [[NSMenuItem alloc] 
				initWithTitle:@"Temp" action:@selector(invoke:) keyEquivalent:@""];

			OSXMenuItemDelegate* delegate = [[OSXMenuItemDelegate alloc] initWithMenuItem:this];
			[item setTarget:delegate];

			SetNSMenuItemTitle(item, this->label);
			SetNSMenuItemIconPath(item, this->iconPath);
			SetNSMenuItemState(item, this->state);
			SetNSMenuItemEnabled(item, this->enabled);
			SetNSMenuItemSubmenu(item, this->submenu, registerNative);

			if (registerNative)
			{
				this->nativeItems.push_back(item);
			}

			return item;
		}
	}

	void MenuItem::DestroyNative(NSMenuItem* realization)
	{
		std::vector<NSMenuItem*>::iterator i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			NSMenuItem* item = *i;
			if (item == realization)
			{
				i = this->nativeItems.erase(i);
				if (!this->submenu.isNull() && [item submenu] != nil)
				{
					AutoPtr<Menu> osxSubmenu = this->submenu.cast<Menu>();
					osxSubmenu->DestroyNative([item submenu]);
				}
				[item release];
			}
			else
			{
				i++;
			}
		}
	}

	void MenuItem::UpdateNativeMenuItems()
	{
		std::vector<NSMenuItem*>::iterator i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			NSMenuItem* nativeItem = (*i++);
			if ([nativeItem menu]) {
				Menu::UpdateNativeMenu([nativeItem menu]);
			}
		}

		// Must now iterate through the native menus and fix
		// the main menu -- it will modify this iterator so we
		// must do it in isolation.
		i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			NSMenuItem* nativeItem = (*i++);
			if ([nativeItem menu] == [NSApp mainMenu]) {
				UIBinding* binding = UIBinding::GetInstance();
				binding->SetupMainMenu(true);
				break;
			}
		}
	}
}


