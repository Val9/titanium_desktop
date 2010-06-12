/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "../user_window.h"

namespace ti
{
	/* TODO: can we kill this code off?
	static unsigned int toWindowMask(AutoPtr<WindowConfig> config)
	{
		unsigned int mask = 0;
		if (!config->HasTransparentBackground() && config->HasTexturedBackground())
			mask |= NSTexturedBackgroundWindowMask;

		if (!config->IsUsingChrome() || config->IsFullscreen())
		{
			mask |= NSBorderlessWindowMask;
		}
		else
		{
			mask |= NSTitledWindowMask | NSClosableWindowMask |
				NSResizableWindowMask | NSMiniaturizableWindowMask;
		}

		return mask;
	}*/

	AutoUserWindow UserWindow::CreateWindow(AutoPtr<WindowConfig> config, AutoUserWindow parent)
	{
		return new UserWindow(config, parent);
	}

	void UserWindow::OpenImpl()
	{
		NSRect frame;
		if (!config->IsFullscreen())
		{
			frame = CalculateWindowFrame(config->GetX(), config->GetY(),
				config->GetWidth(), config->GetHeight());
		}
		else
		{
			frame = [[NSScreen mainScreen] frame];
		}

		NSRect contentRect = [NSWindow contentRectForFrameRect:frame
			styleMask:nativeWindowMask];
		nativeWindow = [[NativeWindow alloc]
			initWithContentRect:contentRect
			styleMask:nativeWindowMask
			backing:NSBackingStoreBuffered
			defer:NO];
		[nativeWindow setUserWindow:new AutoPtr<UserWindow>(this, true)];

		if (!config->IsFullscreen())
		{
			// NSWindow initwithContentRect doesn't seem to honor window placement
			// that insersects with the dock. We set the frame again here, to ensure
			// that the window is placed where we want it.
			[nativeWindow setFrame:frame display:NO animate:NO];

			this->ReconfigureWindowConstraints();
			if (!config->IsResizable())
			{
				[nativeWindow setMinSize:frame.size];
				[nativeWindow setMaxSize:frame.size];
			}
		}

		this->SetCloseable(config->IsCloseable());
		this->SetMaximizable(config->IsMaximizable());
		this->SetMinimizable(config->IsMinimizable());
		[nativeWindow setupDecorations:config];
		this->SetTopMost(config->IsTopMost());

		if (config->IsMaximized())
		{
			[nativeWindow zoom:nativeWindow];
		}

		if (config->IsMinimized())
		{
			[nativeWindow miniaturize:nativeWindow];
		}

		[nativeWindow setExcludedFromWindowsMenu:config->IsToolWindow()];
		[nativeWindow open];
		this->FireEvent(Event::OPENED);
	}

	void UserWindow::Cleanup()
	{
		if (this->active)
			this->Close();
		[nativeWindow dealloc];
	}

	void UserWindow::Hide()
	{
		if (nativeWindow)
		{
			this->Unfocus();
			[nativeWindow orderOut:nil];
			// hidden windows should not show up in windows menu
			[nativeWindow setExcludedFromWindowsMenu:YES];
			this->FireEvent(Event::HIDDEN);
		}
	}
	
	void UserWindow::Focus()
	{
		if (nativeWindow && ![nativeWindow isKeyWindow])
		{
			// visible windows should show up in windows menu
			[nativeWindow setExcludedFromWindowsMenu:NO];
			[NSApp arrangeInFront:nativeWindow];
			[nativeWindow makeKeyAndOrderFront:nativeWindow];
			[NSApp activateIgnoringOtherApps:YES];
			this->Focused();
		}
	}

	void UserWindow::Unfocus()
	{
		// Cocoa doesn't really have a concept of blurring a window, but
		// we can send the window to the back of the window list. We need
		// to fire an unfocused event manually though, because we are still
		// the key window.
		// TODO: Improve this by making Finder the key window.
		if ( nativeWindow && [nativeWindow isKeyWindow])
		{
			[nativeWindow orderBack:nil];
			[nativeWindow windowDidResignKey:nil];
		}
	}

	void UserWindow::Show()
	{
		if (nativeWindow)
		{
			this->Focus();
			this->FireEvent(Event::SHOWN);
		}
	}

	void UserWindow::Minimize()
	{
		if (nativeWindow)
		{
			[nativeWindow miniaturize:nativeWindow];
		}
	}

	void UserWindow::Unminimize()
	{
		if (nativeWindow && [nativeWindow isMiniaturized])
		{
			[nativeWindow deminiaturize:nativeWindow];
		}
	}

	bool UserWindow::IsMinimized()
	{
		if (nativeWindow)
		{
			return [nativeWindow isMiniaturized];
		}
		else
		{
			return this->config->IsMinimized();
		}
	}

	void UserWindow::Maximize()
	{
		if (nativeWindow && ![nativeWindow isZoomed])
		{
			[nativeWindow zoom:nativeWindow];
		}
	}
	
	void UserWindow::Unmaximize()
	{
		if (nativeWindow && [nativeWindow isZoomed])
		{
			[nativeWindow zoom:nativeWindow];
		}
	}

	bool UserWindow::IsMaximized()
	{
		if (nativeWindow)
		{
			return [nativeWindow isZoomed];
		}
		else
		{
			return this->config->IsMinimized();
		}
	}

	bool UserWindow::IsUsingChrome()
	{
		return this->config->IsUsingChrome();
	}

	bool UserWindow::IsUsingScrollbars()
	{
		return this->config->IsUsingScrollbars();
	}

	bool UserWindow::IsFullscreen()
	{
		return this->config->IsFullscreen();
	}

	bool UserWindow::CloseImpl()
	{
		// Hold a reference here so we can still get the value of
		// this->timer and this->active even after calling ::Closed
		// which will remove us from the open window list and decrement
		// the reference count.
		AutoUserWindow keep(this, true);

		// If the window is still active at this point, it  indicates
		// an event listener has cancelled this close event.
		if (!this->active)
		{
			// Actually close the native window and
			// mark this UserWindow as invalid.
			this->Closed();
			[nativeWindow finishClose];
		}

		return !this->active;
	}

	NSScreen* UserWindow::GetWindowScreen()
	{
		NSScreen* screen = [nativeWindow screen];
		if (screen == nil) 
		{
			// Window is offscreen, so set things relative to the main screen.
			// The other option in this case would be to use the "closest" screen,
			// which might be better, but the real fix is to add support for multiple
			// screens in the UI API.
			screen = [NSScreen mainScreen];	

		}
		return screen;
	}

	NSRect UserWindow::CalculateWindowFrame(double x, double y, double width, double height)
	{
		// Adjust for the size of the frame decorations (chrome). Don't modify the
		// position though, because we want it to directly control frame position
		// and not content area position.
		NSRect contentFrame = NSMakeRect(0, 0, width, height);
		contentFrame = [NSWindow frameRectForContentRect:contentFrame
			 styleMask:nativeWindowMask];

		// Center frame, if requested
		NSRect screenFrame = [this->GetWindowScreen() frame];
		if (y == DEFAULT_POSITION)
		{
			y = (screenFrame.size.height - contentFrame.size.height) / 2;
			config->SetY(y);
		}
		if (x == DEFAULT_POSITION)
		{
			x = (screenFrame.size.width - contentFrame.size.width) / 2;
			config->SetX(x);
		}

		// Adjust the position for the origin of this screen and use cartesian coordinates
		contentFrame.origin.x = x + screenFrame.origin.x;
		contentFrame.origin.y = (screenFrame.size.height -
			(contentFrame.size.height + y)) + screenFrame.origin.y;

		return contentFrame;
	}

	double UserWindow::GetX()
	{
		if (!nativeWindow)
			return this->config->GetX();

		// Cocoa frame coordinates are absolute on a plane with all
		// screens, but Titanium wants them relative to the screen.
		NSRect screenFrame = [this->GetWindowScreen() frame];
		return [nativeWindow frame].origin.x - screenFrame.origin.x;
	}

	void UserWindow::SetX(double x)
	{
		this->MoveTo(x, this->GetY());
	}

	double UserWindow::GetY()
	{
		if (!nativeWindow)
			return this->config->GetY();

		// Cocoa frame coordinates are absolute on a plane with all
		// screens, but Titanium wants them relative to the screen.
		NSRect screenFrame = [this->GetWindowScreen() frame];
		double y = [nativeWindow frame].origin.y - screenFrame.origin.y;

		// Adjust for the cartesian coordinate system
		y = screenFrame.size.height - y - [nativeWindow frame].size.height;
		return y;
	}

	void UserWindow::SetY(double y)
	{
		this->MoveTo(this->GetX(), y);
	}

	void UserWindow::MoveTo(double x, double y)
	{
		if (!nativeWindow)
			return;

		NSRect newRect = CalculateWindowFrame(x, y, this->GetWidth(), this->GetHeight());
		[nativeWindow setFrameOrigin:newRect.origin];
	}

	double UserWindow::GetWidth()
	{
		if (!nativeWindow)
			return this->config->GetWidth();

		return [[nativeWindow contentView] bounds].size.width;
	}

	void UserWindow::SetWidth(double width)
	{
		if (!nativeWindow)
			return;

		NSRect newFrame = CalculateWindowFrame(
			this->GetX(), this->GetY(), width, this->GetHeight());

		// We only want to change the width
		newFrame.size.height = [nativeWindow frame].size.height;

		if (!config->IsResizable())
		{
			[nativeWindow setMinSize: newFrame.size];
			[nativeWindow setMaxSize: newFrame.size];
		}
		[nativeWindow setFrame:newFrame display:config->IsVisible() animate:YES];
	}

	double UserWindow::GetHeight()
	{
		if (!nativeWindow)
			return this->config->GetHeight();

		return [[nativeWindow contentView] bounds].size.height;
	}

	void UserWindow::SetHeight(double height)
	{
		if (!nativeWindow)
			return;

		NSRect newFrame = CalculateWindowFrame(
			this->GetX(), this->GetY(), this->GetWidth(), height);

		// We only want to change the height
		newFrame.size.width = [nativeWindow frame].size.width;

		if (!config->IsResizable())
		{
			[nativeWindow setMinSize: newFrame.size];
			[nativeWindow setMaxSize: newFrame.size];
		}
		[nativeWindow setFrame:newFrame display:config->IsVisible() animate:NO];
	}

	void UserWindow::ReconfigureWindowConstraints()
	{
		if (!nativeWindow)
			return;

		NSSize minSize, maxSize;
		double maxWidth = this->config->GetMaxWidth();
		double minWidth = this->config->GetMinWidth();
		double maxHeight = this->config->GetMaxHeight();
		double minHeight = this->config->GetMinHeight();

		if (maxWidth == -1)
		{
			maxSize.width = FLT_MAX;
		}
		else
		{
			maxSize.width = maxWidth;
		}

		if (minWidth == -1)
		{
			minSize.width = 1;
		}
		else
		{
			minSize.width = minWidth;
		}

		if (maxHeight == -1)
		{
			maxSize.height = FLT_MAX;
		}
		else
		{
			maxSize.height = maxHeight;
		}

		if (minHeight == -1)
		{
			minSize.height = 1;
		}
		else
		{
			minSize.height = minHeight;
		}

		[nativeWindow setContentMinSize:minSize];
		[nativeWindow setContentMaxSize:maxSize];
	}

	double UserWindow::GetMaxWidth()
	{
		return this->config->GetMaxWidth();
	}

	void UserWindow::SetMaxWidth(double width)
	{
		this->ReconfigureWindowConstraints();
	}

	double UserWindow::GetMinWidth()
	{
		return this->config->GetMinWidth();
	}

	void UserWindow::SetMinWidth(double width)
	{
		this->ReconfigureWindowConstraints();
	}

	double UserWindow::GetMaxHeight()
	{
		return this->config->GetMaxHeight();
	}

	void UserWindow::SetMaxHeight(double height)
	{
		this->ReconfigureWindowConstraints();
	}
	
	double UserWindow::GetMinHeight()
	{
		return this->config->GetMinHeight();
	}
	
	void UserWindow::SetMinHeight(double height)
	{
		this->ReconfigureWindowConstraints();
	}

	Bounds UserWindow::GetBoundsImpl()
	{
		Bounds b = {this->GetX(), this->GetY(), this->GetWidth(), this->GetHeight() };
		return b;
	}

	void UserWindow::SetBoundsImpl(Bounds bounds)
	{
		if (nativeWindow)
		{
			NSRect newFrame = CalculateWindowFrame(
				bounds.x, bounds.y, bounds.width, bounds.height);

			if (!config->IsResizable())
			{
				[nativeWindow setMinSize: newFrame.size];
				[nativeWindow setMaxSize: newFrame.size];
			}
			[nativeWindow setFrame:newFrame display:config->IsVisible() animate:YES];
		}
	}

	std::string UserWindow::GetTitle()
	{
		return this->config->GetTitle();
	}

	void UserWindow::SetTitleImpl(const std::string& newTitle)
	{
		if (nativeWindow != nil)
		{
			[nativeWindow setTitle:[NSString stringWithUTF8String:newTitle.c_str()]];
		}
	}

	std::string UserWindow::GetURL()
	{
		if (nativeWindow) {
			NSString* url = [[nativeWindow webView] mainFrameURL];
			if (url) {
				return [url UTF8String];
			}
		}
		return this->config->GetURL();
	}

	void UserWindow::SetURL(std::string& url)
	{
		if (nativeWindow != nil)
		{
			std::string nurl = kroll::URLUtils::NormalizeURL(url);
			NSURL* nsurl = [NSURL URLWithString: [NSString stringWithUTF8String:nurl.c_str()]];
			[[[nativeWindow webView] mainFrame] loadRequest:[NSURLRequest requestWithURL:nsurl]];
		}
	}

	bool UserWindow::IsResizable()
	{
		return this->config->IsResizable();
	}

	void UserWindow::SetResizableImpl(bool resizable)
	{
		if (!nativeWindow)
			return;

		[nativeWindow setShowsResizeIndicator:resizable];
		if (resizable)
		{
			this->ReconfigureWindowConstraints();
		}
		else
		{
			[nativeWindow setMinSize: [nativeWindow frame].size];
			[nativeWindow setMaxSize: [nativeWindow frame].size];
		}
	}

	bool UserWindow::IsMaximizable()
	{
		return this->config->IsMaximizable();
	}

	void UserWindow::SetMaximizable(bool maximizable)
	{
		if (nativeWindow != nil)
		{
			[[nativeWindow standardWindowButton:NSWindowZoomButton] setHidden:!maximizable];
		}
	}

	bool UserWindow::IsMinimizable()
	{
		return this->config->IsMinimizable();
	}

	void UserWindow::SetMinimizable(bool minimizable)
	{
		if (nativeWindow != nil)
		{
			[[nativeWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:!minimizable];
		}
	}

	bool UserWindow::IsCloseable()
	{
		return this->config->IsCloseable();
	}

	void UserWindow::SetCloseable(bool closeable)
	{
		if (nativeWindow != nil)
		{
			[[nativeWindow standardWindowButton:NSWindowCloseButton] setHidden:!closeable];
		}
	}

	bool UserWindow::IsVisible()
	{
		return this->config->IsVisible();
	}

	double UserWindow::GetTransparency()
	{
		return this->config->GetTransparency();
	}

	void UserWindow::SetTransparency(double transparency)
	{
		if (nativeWindow != nil)
		{
			[nativeWindow setTransparency:transparency];
		}
	}

	void UserWindow::SetFullscreen(bool fullscreen)
	{
		if (nativeWindow != nil)
		{
			[nativeWindow setFullscreen:fullscreen];
		}
	}

	void UserWindow::SetUsingChrome(bool chrome)
	{
		this->config->SetUsingChrome(chrome);
	}

	void UserWindow::SetMenu(AutoMenu menu)
	{	
		if (this->menu.get() == menu.get())
		{
			return;
		}
		this->menu = menu;
		binding->SetupMainMenu();
	}

	AutoMenu UserWindow::GetMenu()
	{
		return this->menu;
	}

	void UserWindow::Focused()
	{
		this->binding->WindowFocused(AutoPtr<UserWindow>(this, true));
	}

	void UserWindow::Unfocused()
	{
		this->binding->WindowUnfocused(AutoPtr<UserWindow>(this, true));
	}
	
	void UserWindow::SetContextMenu(AutoMenu menu)
	{
		this->contextMenu = menu;
	}

	AutoMenu UserWindow::GetContextMenu()
	{
		return this->contextMenu;
	}

	void UserWindow::SetIcon(std::string& iconPath)
	{
		this->iconPath = iconPath;
	}

	std::string& UserWindow::GetIcon()
	{
		return this->iconPath;
	}

	bool UserWindow::IsTopMost()
	{
		return this->config->IsTopMost();
	}

	void UserWindow::SetTopMost(bool topmost)
	{
		if (nativeWindow != nil)
		{
			if (topmost)
			{
				[nativeWindow setLevel:NSPopUpMenuWindowLevel];
			}
			else
			{
				[nativeWindow setLevel:NSNormalWindowLevel];
			}
		}
	}

	void UserWindow::OpenChooserDialog(bool files, KMethodRef callback,
		bool multiple, std::string& title, std::string& path, std::string& defaultName,
		std::vector<std::string>& types, std::string& typesDescription)
	{
		KListRef results = new StaticBoundList();
		NSOpenPanel* openDlg = [NSOpenPanel openPanel];
		[openDlg setTitle:[NSString stringWithUTF8String:title.c_str()]];
		[openDlg setCanChooseFiles:files];
		[openDlg setCanChooseDirectories:!files];
		[openDlg setAllowsMultipleSelection:multiple];
		[openDlg setResolvesAliases:YES];

		NSMutableArray *filetypes = nil;
		NSString *begin = nil, *filename = nil;

		if (!defaultName.empty())
		{
			filename = [NSString stringWithUTF8String:defaultName.c_str()];
		}
		if (!path.empty())
		{
			begin = [NSString stringWithUTF8String:path.c_str()];
		}
		if (types.size() > 0)
		{
			filetypes = [[NSMutableArray alloc] init];
			for (size_t t = 0; t < types.size(); t++)
			{
				const char *s = types.at(t).c_str();
				[filetypes addObject:[NSString stringWithUTF8String:s]];
			}
		}

		if ([openDlg runModalForDirectory:begin file:filename types:filetypes] == NSOKButton)
		{
			NSArray* selected = [openDlg filenames];
			for (int i = 0; i < (int)[selected count]; i++)
			{
				NSString* fileName = [selected objectAtIndex:i];
				std::string fn = [fileName UTF8String];
				results->Append(Value::NewString(fn));
			}
		}
		[filetypes release];

		callback->Call(ValueList(Value::NewList(results)));
		this->Show();

	}

	void UserWindow::OpenFileChooserDialog(
		KMethodRef callback,
		bool multiple,
		std::string& title,
		std::string& path,
		std::string& defaultName,
		std::vector<std::string>& types,
		std::string& typesDescription)
	{
		this->OpenChooserDialog(
			true, callback, multiple,
			title, path, defaultName, types, typesDescription);
	}

	void UserWindow::OpenFolderChooserDialog(KMethodRef callback, bool multiple,
		std::string& title, std::string& path, std::string& defaultName)
	{
		std::vector<std::string> types;
		std::string typesDescription;
		this->OpenChooserDialog(
			false, callback, multiple,
			title, path, defaultName, types, typesDescription);
	}

	void UserWindow::OpenSaveAsDialog(KMethodRef callback, std::string& title,
		std::string& path, std::string& defaultName, std::vector<std::string>& types,
		std::string& typesDescription)
	{
		int runResult;

		NSMutableArray *filetypes = [[NSMutableArray alloc] init];
		std::vector<std::string>::const_iterator iter = types.begin();
		while (iter != types.end())
		{
			std::string ft = (*iter++);
			[filetypes addObject:[NSString stringWithUTF8String:ft.c_str()]];
		}

		NSSavePanel *sp = [NSSavePanel savePanel];
		[sp setTitle:[NSString stringWithUTF8String:title.c_str()]];

		if ([filetypes count] > 0)
		{
			[sp setAllowedFileTypes:filetypes];
		}

		runResult = [sp 
			runModalForDirectory:[NSString stringWithUTF8String:path.c_str()]
			file:[NSString stringWithUTF8String:defaultName.c_str()]];

		ValueList args;

		KListRef results = new StaticBoundList();
		if (runResult == NSFileHandlingPanelOKButton) 
		{
			NSString *selected = [sp filename];
			results->Append(Value::NewString([selected UTF8String]));
		}

		callback->Call(ValueList(Value::NewList(results)));
		this->Show();
	}
	
	void UserWindow::ShowInspector(bool console)
	{
		[nativeWindow showInspector:console];
	}

	void UserWindow::SetContentsImpl(const std::string& content, const std::string& baseURL)
	{
		[[[nativeWindow webView] mainFrame]
			loadHTMLString: [NSString stringWithUTF8String:content.c_str()]
			baseURL: [NSURL URLWithString:
				[NSString stringWithUTF8String:baseURL.c_str()]]];
	}
}
