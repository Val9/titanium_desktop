/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "osx_menu_delegate.h"

@implementation OSXMenuDelegate

- (id)initWithMenu:(ti::Menu*)inMenu willRegister:(BOOL)willRegister
{
	if ([super init]) {
		dirty = YES;
		menu = inMenu;
		registerNative = willRegister;
	}
	return self;
}

- (void)menuNeedsUpdate:(NSMenu *)nativeMenu
{
	if (!dirty) {
		return;
	}

	dirty = NO;
	Menu::ClearNativeMenu(nativeMenu);
	menu->AddChildrenToNativeMenu(nativeMenu, registerNative ? true : false);
}

- (void)markAsDirty
{
	dirty = YES;
}
@end


