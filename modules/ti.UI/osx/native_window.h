/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _NATIVE_WINDOW_H_
#define _NATIVE_WINDOW_H_

#import "native_window.h"
#import "webview_delegate.h"

#import <WebKit/WebInspector.h>

@class WebViewDelegate;

using namespace ti;

@interface NativeWindow : NSWindow
{
	BOOL canReceiveFocus;
	WebView* webView;
	WebViewDelegate* delegate;
	BOOL requiresDisplay;
	AutoPtr<UserWindow>* userWindow;
	WebInspector* inspector;
	BOOL fullscreen;
	BOOL focused;
	NSRect savedFrame;
}
- (void)setUserWindow:(AutoUserWindow*)inUserWindow;
- (void)setupDecorations:(AutoPtr<WindowConfig>)config;
- (void)setTransparency:(double)transparency;
- (void)setFullscreen:(BOOL)yn;
- (void)close;
- (void)finishClose;
- (void)open;
- (void)frameLoaded;
- (WebView*)webView;
- (UserWindow*)userWindow;
- (void)showInspector:(BOOL)console;
@end

#endif
