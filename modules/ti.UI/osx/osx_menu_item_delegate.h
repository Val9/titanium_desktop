/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "../ui.h"
#import "../menu_item.h"

@interface OSXMenuItemDelegate : NSObject
{
	ti::MenuItem *menuItem;
}
-(id)initWithMenuItem:(ti::MenuItem*)item;
-(void)invoke:(id)sender;
-(ti::MenuItem*)getMenuItem;
@end
