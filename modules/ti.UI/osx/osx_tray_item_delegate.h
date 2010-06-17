/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "../ui.h"
#import "../tray_item.h"

@interface OSXTrayItemDelegate : NSObject
{
	ti::TrayItem *trayItem;
}
-(id)initWithTray:(ti::TrayItem*)item;
-(void)invoke:(id)sender;
@end
