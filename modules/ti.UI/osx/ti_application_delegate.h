/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_APP_H
#define TI_APP_H

#import "../ui.h"
#import "../ui_binding.h"

@interface TiApplicationDelegate : NSObject
{
	ti::UIBinding *binding;
}

-(id)initWithBinding:(ti::UIBinding*)binding;
-(BOOL)application:(NSApplication*)theApplication
	openFile:(NSString*)filename;
-(BOOL)application:(NSApplication*)theApplication
	openFiles:(NSArray*)filenames;
-(NSApplicationTerminateReply) applicationShouldTerminate:(NSApplication*)sender;
@end

#endif
