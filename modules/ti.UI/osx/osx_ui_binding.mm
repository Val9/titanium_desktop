/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_binding.h"

@interface NSApplication (LegacyWarningSurpression)
- (id) dockTile;
@end

@interface DockTileStandin: NSObject {
}
- (NSSize)size;
- (void)setContentView:(NSView *)view;
- (NSView *)contentView;
- (void)display;
- (void)setShowsApplicationBadge:(BOOL)flag;
- (BOOL)showsApplicationBadge;
- (void)setBadgeLabel:(NSString *)string;
- (NSString *)badgeLabel;
- (id)owner;
@end

namespace ti
{
	void UIBinding::Initialize()
	{
		application = [[TiApplicationDelegate alloc] initWithBinding:this];
		[application retain];

		NSApplication* nsapp = [NSApplication sharedApplication];
		[nsapp setDelegate:application];
		[NSBundle loadNibNamed:@"MainMenu" owner:nsapp];

		// Create a default menu -- so that keybindings and such work out of the box.
		this->defaultMenu = [NSApp mainMenu];

		NSString* appName = [NSString
			stringWithUTF8String:host->GetApplication()->name.c_str()];
		Menu::ReplaceAppNameStandinInMenu(this->defaultMenu, appName);
		Menu::SetupInspectorItem(this->defaultMenu);
		this->SetupMainMenu(true);

		// Register our custom URL handler
		[NSURLProtocol registerClass:[TitaniumProtocols class]];

		// SECURITY FLAG: this will allow apps to have the same security
		// as local files (like cross-domain XHR requests).  we should 
		// make sure this is part of the upcoming security work
		[WebView registerURLSchemeAsLocal:@"app"];
		[WebView registerURLSchemeAsLocal:@"ti"];
	}

	void UIBinding::Shutdown()
	{
		[application release];
		[savedDockView release];
	}

	void UIBinding::SetDockIcon(std::string& badgePath)
	{
		//TODO: Put Dock Icon support back in for 10.4.
		if (![NSApp respondsToSelector:@selector(dockTile)]) {
			return;
		}

		DockTileStandin *dockTile = (DockTileStandin *)[NSApp dockTile];
		if (!badgePath.empty())
		{
			if (!savedDockView) // remember the old one
			{
				savedDockView = [dockTile contentView];
				[savedDockView retain];
			}

			// setup our image view for the dock tile
			NSRect frame = NSMakeRect(0, 0, dockTile.size.width, dockTile.size.height);
			NSImageView *dockImageView = [[NSImageView alloc] initWithFrame: frame];
			NSImage *image = MakeImage(badgePath);
			[dockImageView setImage:image];
			[image release];

			// by default, add it to the NSDockTile
			[dockTile setContentView: dockImageView];
		}
		else if (savedDockView)
		{
			[dockTile setContentView:savedDockView];
			[savedDockView release];
			savedDockView = nil;
		}
		else
		{
			[dockTile setContentView:nil];
		}
		[dockTile display];
	}

	NSImage* UIBinding::MakeImage(std::string& iconURL)
	{
		NSString* iconString = [NSString stringWithUTF8String:iconURL.c_str()];
		if (FileUtils::IsFile(iconURL)) {
			return [[NSImage alloc] initWithContentsOfFile:iconString];
		} else {
			return [[NSImage alloc] initWithContentsOfURL:[NSURL URLWithString: iconString]];
		}
	}

	void UIBinding::WindowFocused(AutoUserWindow window)
	{
		this->activeWindow = window;
		this->SetupMainMenu();
	}

	void UIBinding::WindowUnfocused(AutoUserWindow window)
	{
		this->activeWindow = NULL;
		this->SetupMainMenu();
	}

	void UIBinding::SetupMainMenu(bool force)
	{
		AutoMenu newActiveMenu = NULL;

		// If there is an active window, search there first for the menu
		if (!this->activeWindow.isNull()) {
			newActiveMenu = this->activeWindow->GetMenu();
		}

		// No active window or that window has no menu, try to use the app menu
		if (newActiveMenu.isNull()) {
			newActiveMenu = this->menu;
		}

		if (force || newActiveMenu.get() != this->activeMenu.get()) {
			AutoMenu oldMenu = this->activeMenu; // Save a reference
			NSMenu* oldNativeMenu = [NSApp mainMenu];

			// Actually create and install the new menu
			NSMenu* newNativeMenu = nil;
			if (newActiveMenu.isNull()) {
				newNativeMenu = this->defaultMenu;
			} else {
				newNativeMenu = [[NSMenu alloc] init];
				newActiveMenu->FillNativeMainMenu(defaultMenu, newNativeMenu);
			}
			SetupAppMenuParts(newNativeMenu);
			[NSApp setMainMenu:newNativeMenu];
			this->activeMenu = newActiveMenu;

			// Cleanup the old native menu
			if (!oldMenu.isNull() && oldNativeMenu
				&& oldNativeMenu != this->defaultMenu) {
				oldMenu->DestroyNative(oldNativeMenu);
			}
		}
	}

	void UIBinding::SetupAppMenuParts(NSMenu* nativeMainMenu)
	{
		Menu::FixWindowMenu(nativeMainMenu);
		[NSApp setWindowsMenu:Menu::GetWindowMenu(nativeMainMenu)];
		[NSApp performSelector:@selector(setAppleMenu:)
			withObject:Menu::GetAppleMenu(nativeMainMenu)];
		[NSApp setServicesMenu:Menu::GetServicesMenu(nativeMainMenu)];
	}

	void UIBinding::SetDockMenu(AutoMenu menu)
	{
		this->dockMenu = menu;
	}

	void UIBinding::SetBadge(std::string& badgeLabel)
	{
		//TODO: Put Dock Icon support back in for 10.4.
		if (![NSApp respondsToSelector:@selector(dockTile)]){
			return;
		}

		DockTileStandin *tile = (DockTileStandin *)[[NSApplication sharedApplication] dockTile];
		if (!badgeLabel.empty()) {
			[tile setBadgeLabel:[NSString stringWithUTF8String:badgeLabel.c_str()]];

		} else {
			[tile setBadgeLabel:@""];
		}
	}

	void UIBinding::SetBadgeImage(std::string& badgePath)
	{
		//TODO: need to support allowing custom badge images
	}

	void UIBinding::SetIcon(std::string& iconPath)
	{
		if (iconPath.empty())
		{
			[[NSApplication sharedApplication] setApplicationIconImage:nil];
		}
		else
		{
			NSImage *image = MakeImage(iconPath);
			[[NSApplication sharedApplication] setApplicationIconImage:image];
			[image release];
		}
	}

	AutoMenu UIBinding::GetDockMenu()
	{
		return this->dockMenu;
	}

	long UIBinding::GetIdleTime()
	{
		// some of the code for this was from:
		// http://ryanhomer.com/blog/2007/05/31/detecting-when-your-cocoa-application-is-idle/
		CFMutableDictionaryRef properties = 0;
		CFTypeRef obj;
		mach_port_t masterPort;
		io_iterator_t iter;
		io_registry_entry_t curObj;

		IOMasterPort(MACH_PORT_NULL, &masterPort);

		/* Get IOHIDSystem */
		IOServiceGetMatchingServices(masterPort, IOServiceMatching("IOHIDSystem"), &iter);
		if (iter == 0)
		{
			return -1;
		}
		else
		{
			curObj = IOIteratorNext(iter);
		}
		if (IORegistryEntryCreateCFProperties(curObj, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS && properties != NULL)
		{
			obj = CFDictionaryGetValue(properties, CFSTR("HIDIdleTime"));
			CFRetain(obj);
		}
		else
		{
			return -1;
		}

		uint64_t tHandle = 0;
		if (obj)
		{
			CFTypeID type = CFGetTypeID(obj);

			if (type == CFDataGetTypeID())
			{
				CFDataGetBytes((CFDataRef) obj, CFRangeMake(0, sizeof(tHandle)), (UInt8*) &tHandle);
			}
			else if (type == CFNumberGetTypeID())
			{
				CFNumberGetValue((CFNumberRef)obj, kCFNumberSInt64Type, &tHandle);
			}
			else
			{
				// error
				tHandle = 0;
			}

			CFRelease(obj);

			tHandle /= 1000000; // return as milliseconds
		}
		else
		{
			tHandle = -1;
		}

		CFRelease((CFTypeRef)properties);
		IOObjectRelease(curObj);
		IOObjectRelease(iter);
		return (long)tHandle;
	}

	/*static*/
	void UIBinding::ErrorDialog(std::string msg)
	{
		NSApplicationLoad();
		NSRunCriticalAlertPanel(@"Application Error", 
			[NSString stringWithUTF8String:msg.c_str()], nil, nil, nil);
		UIBinding::ErrorDialog(msg);
	}
}
