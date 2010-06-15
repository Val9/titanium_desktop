/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../tray_item.h"
#include "../ui_binding.h"

namespace ti
{
	std::vector<AutoTrayItem> TrayItem::trayItems;
	UINT TrayItem::trayClickedMessage =
		::RegisterWindowMessageA(PRODUCT_NAME"TrayClicked");

	void TrayItem::Initialize()
	{
		this->oldNativeMenu = 0;
		this->trayIconData = 0;

		this->AddEventListener(Event::CLICKED, this->callback);

		HWND hwnd = Host::GetInstance()->AddMessageHandler(
			&TrayItem::MessageHandler);

		NOTIFYICONDATA* notifyIconData = new NOTIFYICONDATA;
		notifyIconData->cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData->hWnd = hwnd;
		notifyIconData->uID = ++UIBinding::nextItemId;
		notifyIconData->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		notifyIconData->uCallbackMessage = trayClickedMessage;

		HICON icon = UIBinding::LoadImageAsIcon(iconPath,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON));
		notifyIconData->hIcon = icon;

		lstrcpy(notifyIconData->szTip, L"Titanium Application");
		Shell_NotifyIcon(NIM_ADD, notifyIconData);
		this->trayIconData = notifyIconData;

		trayItems.push_back(this);
	}
	
	void TrayItem::Shutdown()
	{
		this->Remove();
	}
	
	void TrayItem::SetIcon(std::string& iconPath)
	{
		if (!this->trayIconData)
			return;

		HICON icon = UIBinding::LoadImageAsIcon(iconPath,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON));
		this->trayIconData->hIcon = icon;
		Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
	}
	
	void TrayItem::SetMenu(AutoMenu menu)
	{
		this->menu = menu;
	}
	
	void TrayItem::SetHint(std::string& hint)
	{
		if (this->trayIconData)
		{
			// NotifyIconData.szTip has 128 character limit.
			ZeroMemory(this->trayIconData->szTip, 128);

			// make sure we don't overflow the static buffer.
			std::wstring hintW = ::UTF8ToWide(hint);
			lstrcpyn(this->trayIconData->szTip, hintW.c_str(), 128);

			Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
		}
	}
	
	void TrayItem::Remove()
	{
		if (this->trayIconData)
		{
			Shell_NotifyIcon(NIM_DELETE, this->trayIconData);
			this->trayIconData = 0;
		}
	}

	void TrayItem::HandleRightClick()
	{
		if (this->oldNativeMenu)
		{
			DestroyMenu(this->oldNativeMenu);
			this->oldNativeMenu = 0;
		}

		if (this->menu.isNull())
			return;

		AutoMenu win32menu = this->menu;
		if (win32menu.isNull())
			return;

		this->oldNativeMenu = win32menu->CreateNative(false);
		POINT pt;
		GetCursorPos(&pt);
		
		SetForegroundWindow(this->trayIconData->hWnd);	
		TrackPopupMenu(this->oldNativeMenu, TPM_BOTTOMALIGN, 
			pt.x, pt.y, 0, this->trayIconData->hWnd, NULL);
		PostMessage(this->trayIconData->hWnd, WM_NULL, 0, 0);
	}

	void TrayItem::HandleLeftClick()
	{
		try
		{
			this->FireEvent(Event::CLICKED);
		}
		catch (ValueException& e)
		{
			Logger* logger = Logger::Get("UI.TrayItem");
			SharedString ss = e.DisplayString();
			logger->Error("Tray icon single click callback failed: %s", ss->c_str());
		}
	}
	
	void TrayItem::HandleDoubleLeftClick()
	{
		try
		{
			this->FireEvent(Event::DOUBLE_CLICKED);
		}
		catch (ValueException& e)
		{
			Logger* logger = Logger::Get("UI.TrayItem");
			SharedString ss = e.DisplayString();
			logger->Error("Tray icon double left click callback failed: %s", ss->c_str());
		}
	}
	
	UINT TrayItem::GetId()
	{
		return this->trayIconData->uID;
	}

	/*static*/
	bool TrayItem::MessageHandler(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == trayClickedMessage)
		{
			UINT button = (UINT) lParam;
			int id = LOWORD(wParam);
			bool handled = false;

			for (size_t i = 0; i < trayItems.size(); i++)
			{
				AutoTrayItem item = trayItems[i];

				item->is_double_clicked = false;
				if(item->GetId() == id && button == WM_LBUTTONDBLCLK)
				{
					item->is_double_clicked = true;
					KillTimer(hWnd, 100);
					item->HandleDoubleLeftClick();
					handled = true;
				}
				if (item->GetId() == id && button == WM_LBUTTONDOWN)
				{
					SetTimer(hWnd, 100, GetDoubleClickTime(), (TIMERPROC)DoubleClickTimerProc); 
					handled = true;
				}
				else if (item->GetId() == id && button == WM_RBUTTONDOWN)
				{
					item->HandleRightClick();
					handled = true;
				}
			}
			return handled;
		}
		else if (message == WM_MENUCOMMAND)
		{
			HMENU nativeMenu = (HMENU) lParam;
			UINT position = (UINT) wParam;
			return MenuItem::HandleClickEvent(nativeMenu, position);
		}
		else
		{
			// Not handled;
			return false;
		}
	}
	
	/*static*/
	LRESULT CALLBACK TrayItem::DoubleClickTimerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		bool handled = false;

		KillTimer(hWnd, 100);
		for (size_t i = 0; i < trayItems.size(); i++)
		{
			AutoTrayItem item = trayItems[i];
			if (!(item->is_double_clicked))
			{
				item->HandleLeftClick();
			}

			item->is_double_clicked = false;
		}
		return 0;
	}
}
