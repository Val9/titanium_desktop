/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "../ui_module.h"

@interface OSXMenuDelegate : NSObject
{
	ti::Menu* menu;
	BOOL dirty;
	BOOL registerNative;
}
- (id)initWithMenu:(ti::Menu*)menu willRegister:(BOOL)willRegister;
- (void)menuNeedsUpdate:(NSMenu *)menu;
- (void)markAsDirty;
@end
