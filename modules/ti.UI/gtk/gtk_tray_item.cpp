/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../tray_item.h"

namespace ti
{

	void TrayClickedCallback(GtkStatusIcon*, gpointer);
	void TrayMenuCallback(GtkStatusIcon*, guint, guint, gpointer);

	void TrayItem::Initialize()
	{
		this->item = gtk_status_icon_new();
		this->active = true;

		g_signal_connect(
			G_OBJECT(this->item), "activate",
			G_CALLBACK(TrayClickedCallback), this);
		g_signal_connect(
			G_OBJECT(this->item), "popup-menu",
			G_CALLBACK(TrayMenuCallback), this);

		this->SetIcon(this->iconPath);
		gtk_status_icon_set_visible(this->item, TRUE);
	}

	void TrayItem::Shutdown()
	{
	}

	void TrayItem::SetIcon(std::string& iconPath)
	{
		if (active) {
			if (iconPath.empty()) {
				gtk_status_icon_set_from_file(this->item, NULL);
			} else {
				gtk_status_icon_set_from_file(this->item, iconPath.c_str());
			}
		}
	}

	void TrayItem::SetMenu(AutoMenu menu)
	{
		this->menu = menu;
	}

	void TrayItem::SetHint(std::string& hint)
	{
		if (active) {
			if (hint.empty()) {
				gtk_status_icon_set_tooltip(this->item, NULL);
			} else {
				gtk_status_icon_set_tooltip(this->item, hint.c_str());
			}
		}
	}

	void TrayItem::Remove()
	{
		if (active) {
			this->active = false;
			g_object_unref(this->item);
		}
	}

	GtkStatusIcon* TrayItem::GetWidget()
	{
		if (active)
			return this->item;
		else
			return NULL;
	}

	AutoMenu TrayItem::GetMenu()
	{
		return this->menu;
	}

	KMethodRef TrayItem::GetCallback()
	{
		return this->callback;
	}

	void TrayClickedCallback(GtkStatusIcon *status_icon, gpointer data)
	{
		TrayItem* item = static_cast<TrayItem*>(data);
		KMethodRef cb = item->GetCallback();

		if (cb.isNull())
			return;

		try {
			ValueList args;
			cb->Call(args);

		} catch (ValueException& e) {
			Logger* logger = Logger::Get("UI.TrayItem");
			SharedString ss = e.DisplayString();
			logger->Error("Tray icon callback failed: %s", ss->c_str());
		}
	}

	void TrayMenuCallback(
		GtkStatusIcon *status_icon, guint button,
		guint activate_time, gpointer data)
	{
		TrayItem* item = static_cast<TrayItem*>(data);
		GtkStatusIcon* trayWidget = item->GetWidget();
		AutoMenu menu = item->GetMenu();

		if (!menu.isNull()) {
			GtkMenu* nativeMenu = (GtkMenu*) menu->CreateNative(false);
			gtk_menu_popup(
				nativeMenu, NULL, NULL,
				gtk_status_icon_position_menu,
				trayWidget, button, activate_time);
		}
	}


}
