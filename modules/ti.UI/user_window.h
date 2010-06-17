/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _USER_WINDOW_H_
#define _USER_WINDOW_H_


#include "ui.h"
#include "menu.h"

#include <string>
#include <vector>
#include <map>

#ifdef OS_WIN32
#undef CreateWindow

#include "win32/webkit_frame_load_delegate.h"
#include "win32/webkit_policy_delegate.h"
#include "win32/webkit_ui_delegate.h"
#include "win32/webkit_resource_load_delegate.h"
#endif

#ifdef OS_OSX
#include "osx/native_window.h"
#endif

namespace ti
{
	typedef struct {
		double x;
		double y;
		double width;
		double height;
	} Bounds;

	class UserWindow : public KEventObject
	{
		public:
			static AutoUserWindow CreateWindow(AutoPtr<WindowConfig> config, AutoUserWindow parent);

			void Open();
			bool Close();
			SharedString DisplayString(int levels=3);
			virtual ~UserWindow();
			void UpdateWindowForURL(std::string url);
			void RegisterJSContext(JSGlobalContextRef);
			void InsertAPI(KObjectRef frameGlobal);
			void PageLoaded(KObjectRef scope, std::string &url, JSGlobalContextRef context);
			inline KObjectRef GetDOMWindow() { return this->domWindow; }
			inline Host* GetHost() { return this->host; }
			inline bool IsToolWindow() {return this->config->IsToolWindow(); }
			inline void SetToolWindow(bool toolWindow) {this->config->SetToolWindow(toolWindow); }
			inline bool HasTransparentBackground() { return this->config->HasTransparentBackground(); }
			inline void SetTransparentBackground(bool transparentBackground) { this->config->SetTransparentBackground(transparentBackground); }
			inline std::string GetId() { return this->config->GetID(); }

			void _GetCurrentWindow(const kroll::ValueList&, kroll::KValueRef);
			void _GetDOMWindow(const kroll::ValueList&, kroll::KValueRef);
			void _InsertAPI(const kroll::ValueList&, kroll::KValueRef);
			void _Hide(const kroll::ValueList&, kroll::KValueRef);
			void _Show(const kroll::ValueList&, kroll::KValueRef);
			void _Minimize(const kroll::ValueList&, kroll::KValueRef);
			void _Maximize(const kroll::ValueList&, kroll::KValueRef);
			void _Unminimize(const kroll::ValueList&, kroll::KValueRef);
			void _Unmaximize(const kroll::ValueList&, kroll::KValueRef);
			void _IsMaximized(const kroll::ValueList&, kroll::KValueRef);
			void _IsMinimized(const kroll::ValueList&, kroll::KValueRef);
			void _Focus(const kroll::ValueList&, kroll::KValueRef);
			void _Unfocus(const kroll::ValueList&, kroll::KValueRef);
			void _IsUsingChrome(const kroll::ValueList&, kroll::KValueRef);
			void _SetUsingChrome(const kroll::ValueList&, kroll::KValueRef);
			void _IsToolWindow(const kroll::ValueList&, kroll::KValueRef);
			void _SetToolWindow(const kroll::ValueList&, kroll::KValueRef);
			void _HasTransparentBackground(const kroll::ValueList&, kroll::KValueRef);
			void _SetTransparentBackground(const kroll::ValueList&, kroll::KValueRef);
			void _IsUsingScrollbars(const kroll::ValueList&, kroll::KValueRef);
			void _IsFullscreen(const kroll::ValueList&, kroll::KValueRef);
			void _SetFullscreen(const kroll::ValueList&, kroll::KValueRef);
			void _GetId(const kroll::ValueList&, kroll::KValueRef);
			void _Open(const kroll::ValueList&, kroll::KValueRef);
			void _Close(const kroll::ValueList&, kroll::KValueRef);
			void _GetX(const kroll::ValueList&, kroll::KValueRef);
			double _GetX();
			void _SetX(const kroll::ValueList&, kroll::KValueRef);
			void _SetX(double x);
			void _GetY(const kroll::ValueList&, kroll::KValueRef);
			double _GetY();
			void _SetY(const kroll::ValueList&, kroll::KValueRef);
			void _SetY(double y);
			void _MoveTo(const kroll::ValueList&, kroll::KValueRef);
			void _GetWidth(const kroll::ValueList&, kroll::KValueRef);
			double _GetWidth();
			void _SetWidth(const kroll::ValueList&, kroll::KValueRef);
			void _SetWidth(double width);
			void _GetMaxWidth(const kroll::ValueList&, kroll::KValueRef);
			void _SetMaxWidth(const kroll::ValueList&, kroll::KValueRef);
			void _GetMinWidth(const kroll::ValueList&, kroll::KValueRef);
			void _SetMinWidth(const kroll::ValueList&, kroll::KValueRef);
			void _GetHeight(const kroll::ValueList&, kroll::KValueRef);
			double _GetHeight();
			void _SetHeight(const kroll::ValueList&, kroll::KValueRef);
			void _SetHeight(double height);
			void _GetMaxHeight(const kroll::ValueList&, kroll::KValueRef);
			void _SetMaxHeight(const kroll::ValueList&, kroll::KValueRef);
			void _GetMinHeight(const kroll::ValueList&, kroll::KValueRef);
			void _SetMinHeight(const kroll::ValueList&, kroll::KValueRef);
			void _GetBounds(const kroll::ValueList&, kroll::KValueRef);
			void _SetBounds(const kroll::ValueList&, kroll::KValueRef);
			void _GetTitle(const kroll::ValueList&, kroll::KValueRef);
			void _SetTitle(const kroll::ValueList&, kroll::KValueRef);
			void _GetURL(const kroll::ValueList&, kroll::KValueRef);
			void _SetURL(const kroll::ValueList&, kroll::KValueRef);
			void _IsResizable(const kroll::ValueList&, kroll::KValueRef);
			void _SetResizable(const kroll::ValueList&, kroll::KValueRef);
			void _IsMaximizable(const kroll::ValueList&, kroll::KValueRef);
			void _SetMaximizable(const kroll::ValueList&, kroll::KValueRef);
			void _IsMinimizable(const kroll::ValueList&, kroll::KValueRef);
			void _SetMinimizable(const kroll::ValueList&, kroll::KValueRef);
			void _IsCloseable(const kroll::ValueList&, kroll::KValueRef);
			void _SetCloseable(const kroll::ValueList&, kroll::KValueRef);
			void _IsVisible(const kroll::ValueList&, kroll::KValueRef);
			void _IsActive(const kroll::ValueList&, kroll::KValueRef);
			void _SetVisible(const kroll::ValueList&, kroll::KValueRef);
			void _GetTransparency(const kroll::ValueList&, kroll::KValueRef);
			void _SetTransparency(const kroll::ValueList&, kroll::KValueRef);
			void _GetMenu(const kroll::ValueList&, kroll::KValueRef);
			void _SetMenu(const kroll::ValueList&, kroll::KValueRef);
			void _GetContextMenu(const kroll::ValueList&, kroll::KValueRef);
			void _SetContextMenu(const kroll::ValueList&, kroll::KValueRef);
			void _GetIcon(const kroll::ValueList&, kroll::KValueRef);
			void _SetIcon(const kroll::ValueList&, kroll::KValueRef);
			void _GetParent(const kroll::ValueList&, kroll::KValueRef);
			void _GetChildren(const kroll::ValueList&, kroll::KValueRef);
			void _CreateWindow(const kroll::ValueList&, kroll::KValueRef);
			void _OpenFileChooserDialog(const ValueList& args, KValueRef result);
			void _OpenFolderChooserDialog(const ValueList& args, KValueRef result);
			void _OpenSaveAsDialog(const ValueList& args, KValueRef result);
			void _IsTopMost(const kroll::ValueList&, kroll::KValueRef);
			void _SetTopMost(const kroll::ValueList&, kroll::KValueRef);
			void _ShowInspector(const ValueList& args, KValueRef result);
			void _SetContents(const ValueList& args, KValueRef result);
			void SetContents(const std::string& content, const std::string& baseURL);
			void OpenFileChooserDialog(KMethodRef callback, bool multiple,
				std::string& title, std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription);
			void OpenFolderChooserDialog( KMethodRef callback,
				bool multiple, std::string& title, std::string& path,
				std::string& defaultName);
			void OpenSaveAsDialog(KMethodRef callback, std::string& title,
				std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription);

			// Platform implementations
			void Cleanup();
			void Hide();
			void Show();
			void Minimize();
			void Maximize();
			void Unminimize();
			void Unmaximize();
			bool IsMaximized();
			bool IsMinimized();
			void Focus();
			void Unfocus();
			bool IsUsingChrome();
			bool IsUsingScrollbars();
			bool IsFullscreen();
			void OpenImpl();
			bool CloseImpl();
			void Closed();
			double GetX();
			void SetX(double x);
			double GetY();
			void SetY(double y);
			void MoveTo(double x, double y);
			double GetWidth();
			void SetWidth(double width);
			double GetMaxWidth();
			void SetMaxWidth(double width);
			double GetMinWidth();
			void SetMinWidth(double width);
			double GetHeight();
			void SetHeight(double height);
			double GetMaxHeight();
			void SetMaxHeight(double height);
			double GetMinHeight();
			void SetMinHeight(double height);
			Bounds GetBounds();
			Bounds GetBoundsImpl();
			void SetBounds(Bounds bounds);
			void SetBoundsImpl(Bounds bounds);
			std::string GetTitle();
			void SetTitle(const std::string& title);
			void SetTitleImpl(const std::string& title);
			std::string GetURL();
			void SetURL(std::string &url);
			bool IsResizable();
			void SetResizable(bool resizable);
			void SetResizableImpl(bool resizable);
			bool IsMaximizable();
			void SetMaximizable(bool maximizable);
			bool IsMinimizable();
			void SetMinimizable(bool minimizable);
			bool IsCloseable();
			void SetCloseable(bool closeable);
			bool IsVisible();
			double GetTransparency();
            void SetTransparency(double transparency);
			void SetFullscreen(bool fullscreen);
			void SetUsingChrome(bool chrome);
			void SetMenu(AutoMenu menu);
			AutoMenu GetMenu();
			void SetContextMenu(AutoMenu menu);
			AutoMenu GetContextMenu();
			void SetIcon(std::string& iconPath);
			std::string& GetIcon();
			bool IsTopMost();
			void SetTopMost(bool topmost);
			void ShowInspector(bool console=false);
			void AppIconChanged();
			void AppMenuChanged();
			void SetContentsImpl(const std::string& content,  const std::string& baseURL);

#ifdef OS_LINUX
			void CreateWidgets();
			void ShowWidgets();
			void SetupDecorations();
			void SetupTransparency();
			void SetupSizeLimits();
			void SetupSize();
			void SetupPosition();
			void SetupMenu();
			void SetupIcon();
			void RemoveOldMenu();
			void SetInspectorWindow(GtkWidget* inspectorWindow);
			GtkWidget *GetInspectorWindow();
			WebKitWebView* GetWebView() { return this->webView; }

			// These values contain the most-recently-set dimension
			// information for this UserWindow. GDK is asynchronous,
			// so if a user sets the value the and fetches it without
			// giving up control to the UI thread, returning one of them
			// will yield the correct information. When we actually
			// detect a GDK window resize, these values will also be
			// updated, so they will be an accurate representation of
			// the window size.
			int targetWidth;
			int targetHeight;
			int targetX;
			int targetY;
			bool targetMaximized;
			bool targetMinimized;
#endif

#ifdef OS_OSX
			NativeWindow* GetNative() { return nativeWindow; }
			void Focused();
			void Unfocused();
			void ReconfigureWindowConstraints();
			void OpenChooserDialog(bool files, KMethodRef callback, bool multiple,
				std::string& title, std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription);
#endif

#ifdef OS_WIN32
			void FrameLoaded();
			HWND GetWindowHandle();
			static UserWindow* FromWindow(HWND hWnd);
			static AutoUserWindow FromWebView(IWebView *webView);
			void RedrawMenu();
			static void RedrawAllMenus();
			IWebView* GetWebView() { return webView; };
			void SetBitmap(HBITMAP bitmap) { this->webkitBitmap = bitmap; }
			UINT_PTR GetTimer() { return this->timer; }
			void UpdateBitmap();
			void GetMinMaxInfo(MINMAXINFO* minMaxInfo);
			void ResizeSubViews();
#endif

		protected:
			Logger* logger;
			AutoUIBinding binding;
			KObjectRef domWindow;
			Host* host;
			AutoPtr<WindowConfig> config;
			AutoUserWindow parent;
			std::vector<AutoUserWindow> children;
			bool active;
			bool initialized;
			std::string iconURL;

			UserWindow(AutoPtr<WindowConfig> config, AutoUserWindow parent);
			virtual AutoUserWindow GetParent();
			virtual void AddChild(AutoUserWindow);
			virtual void RemoveChild(AutoUserWindow);
			void ReadChooserDialogObject(KObjectRef o, bool& multiple,
				std::string& title, std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription);
			static void LoadUIJavaScript(JSGlobalContextRef context);

		private:
#ifdef OS_LINUX
			GtkWindow* gtkWindow;
			GtkWidget* vbox;
			WebKitWebView* webView;
			bool topmost;
			gulong deleteCallbackId; 
			GtkMenuBar* nativeMenu; // The widget this window uses for a menu.
			GtkWidget *inspectorWindow; // This window's web inspector window
#endif

#ifdef OS_OSX
			NativeWindow* nativeWindow;
			unsigned int nativeWindowMask;
			bool focused;
			static bool initial;

			NSRect CalculateWindowFrame(double x, double y,
				double width, double height);
			NSScreen* GetWindowScreen();
#endif

#ifdef OS_WIN32
			Win32WebKitFrameLoadDelegate* frameLoadDelegate;
			Win32WebKitUIDelegate* uiDelegate;
			Win32WebKitPolicyDelegate* policyDelegate;
			Win32WebKitResourceLoadDelegate* resourceLoadDelegate;
			Bounds restoreBounds;
			long restoreStyles;
			HWND windowHandle;
			HWND viewWindowHandle;
			HBITMAP webkitBitmap;
			UINT_PTR timer;
			IWebView* webView;
			IWebFrame* mainFrame;
			IWebInspector* webInspector;
			Bounds chromeSize;
			HMENU nativeMenu; // This window's active native menu

			// Set this flag to indicate that when the frame is loaded we want to
			// show the window - we do this to prevent white screen while the first
			// URL loads in the WebView.
			bool requiresDisplay;

			void RemoveOldMenu();
			DWORD GetStyleFromConfig();
			void InitWindow();
			void InitWebKit();
			void SetupFrame();
			void SetupDecorations();
			void SetupState();
			void SetupMenu();
			void SetupIcon();

			KListRef SelectFile(
				bool saveDialog, bool multiple, std::string& title,
				std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription);
			KListRef SelectDirectory(bool multiple, std::string& title,
				std::string& path, std::string& defaultName);
#endif

			AutoMenu menu;
			AutoMenu activeMenu;
			AutoMenu contextMenu;
			std::string iconPath;

			DISALLOW_EVIL_CONSTRUCTORS(UserWindow);
	};
}
#endif
