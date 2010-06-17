/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../menu_item.h"

namespace ti
{
	static void MenuCallback(GtkMenuItem* nativeItem, gpointer data)
	{
		MenuItem* item = static_cast<MenuItem*>(data);
		if (item->IsCheck())
		{
			g_signal_handlers_block_by_func(G_OBJECT(nativeItem),
				(void*) MenuCallback, item);
			gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(nativeItem), item->GetState());
			g_signal_handlers_unblock_by_func(G_OBJECT(nativeItem),
				(void*) MenuCallback, item);
		}
		item->HandleClickEvent(0);
	}

	void MenuItem::SetLabelImpl(std::string newLabel)
	{
		if (this->type == SEPARATOR)
			return;

		// Make a copy of the list of nativeItems, because we may modify it
		std::vector<GtkMenuItem*> nativeCopy = this->nativeItems;
		std::vector<GtkMenuItem*>::iterator i = nativeCopy.begin();
		while (i != nativeCopy.end())
		{
			GtkMenuItem* nativeItem = *i++;
			GtkMenuItem* newNativeItem = this->CreateNative(true);
			this->ReplaceNativeItem(nativeItem, newNativeItem);
		}
	}

	void MenuItem::SetIconImpl(std::string newIconPath)
	{
		if (this->type == SEPARATOR || this->type == CHECK)
			return;

		// Make a copy of the list of nativeItems, because SetNativeItemIcon may modify it
		std::vector<GtkMenuItem*> nativeCopy = this->nativeItems;

		std::vector<GtkMenuItem*>::iterator i = nativeCopy.begin();
		while (i != nativeCopy.end())
		{
			GtkMenuItem* nativeItem = *i++;
			this->SetNativeItemIcon(nativeItem, newIconPath);
		}
	}

	void MenuItem::SetStateImpl(bool newState)
	{
		if (!this->IsCheck())
			return;

		std::vector<GtkMenuItem*>::iterator i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			GtkMenuItem* nativeItem = *i++;
			g_signal_handlers_block_by_func(G_OBJECT(nativeItem),
				(void*) MenuCallback, this);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(nativeItem), newState);
			g_signal_handlers_unblock_by_func(G_OBJECT(nativeItem),
				(void*) MenuCallback, this);
		}
	}

	void MenuItem::SetSubmenuImpl(AutoMenu newSubmenu)
	{
		if (this->IsSeparator())
			return;

		std::vector<GtkMenuItem*>::iterator i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			GtkMenuItem* nativeItem = *i++;
			this->SetNativeItemSubmenu(nativeItem, newSubmenu);
		}

		this->oldSubmenu = newSubmenu;
	}

	void MenuItem::SetEnabledImpl(bool enabled)
	{
		if (this->type == SEPARATOR)
			return;

		std::vector<GtkMenuItem*>::iterator i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			GtkMenuItem* nativeItem = *i++;
			gtk_widget_set_sensitive(GTK_WIDGET(nativeItem), enabled);
		}
	}

	void MenuItem::SetNativeItemIcon(GtkMenuItem* nativeItem, std::string& newIconPath)
	{
		// If we are clearing the icon and this is already an image item or we are
		// not an image item and we are trying to set an image, then remake the native
		// item.
		if ((newIconPath.empty() &&
				G_TYPE_FROM_INSTANCE(nativeItem) == GTK_TYPE_IMAGE_MENU_ITEM) ||
			(G_TYPE_FROM_INSTANCE(nativeItem) != GTK_TYPE_IMAGE_MENU_ITEM))
		{
			GtkMenuItem* newNativeItem = this->CreateNative(true);
			this->ReplaceNativeItem(nativeItem, newNativeItem);
		}
		else
		{
			GtkWidget* image = gtk_image_new_from_file(newIconPath.c_str());
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(nativeItem), image);
		}
	}

	void MenuItem::ReplaceNativeItem(GtkMenuItem* nativeItem, GtkMenuItem* newNativeItem)
	{
		GtkMenuShell* nativeMenu = GTK_MENU_SHELL(gtk_widget_get_parent(GTK_WIDGET(nativeItem)));
		GList* children = gtk_container_get_children(GTK_CONTAINER(nativeMenu));

		for (size_t i = 0; i < g_list_length(children); i++)
		{
			GtkMenuItem* w = static_cast<GtkMenuItem*>(g_list_nth_data(children, i));
			if (w == nativeItem)
			{
				gtk_container_remove(GTK_CONTAINER(nativeMenu), GTK_WIDGET(w));
				gtk_menu_shell_insert(nativeMenu, GTK_WIDGET(newNativeItem), i);
				gtk_widget_show(GTK_WIDGET(newNativeItem));
				this->DestroyNative(nativeItem);
				return;
			}
		}
	}

	void MenuItem::SetNativeItemSubmenu(GtkMenuItem* nativeItem, AutoMenu newSubmenu)
	{
		GtkMenuShell* oldNativeMenu = GTK_MENU_SHELL(gtk_menu_item_get_submenu(nativeItem));
		if (oldNativeMenu && !this->oldSubmenu.isNull())
		{
			this->oldSubmenu->DestroyNative(oldNativeMenu);
		}

		GtkMenuShell* newNativeMenu = 0;
		if (!newSubmenu.isNull())
		{
			newNativeMenu = newSubmenu->CreateNative(true);
		}
		gtk_menu_item_set_submenu(nativeItem, GTK_WIDGET(newNativeMenu));
	}

	GtkMenuItem* MenuItem::CreateNative(bool registerNative)
	{
		GtkMenuItem* newNativeItem;
		if (this->IsSeparator())
		{
			return (GtkMenuItem*) gtk_separator_menu_item_new();
		}
		else if (this->IsCheck())
		{
			newNativeItem = (GtkMenuItem*) gtk_check_menu_item_new_with_label(label.c_str());
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(newNativeItem), this->state);
		}
		else if (this->iconPath.empty())
		{
			newNativeItem = (GtkMenuItem*) gtk_menu_item_new_with_label(label.c_str());
		}
		else
		{
			newNativeItem = (GtkMenuItem*) gtk_image_menu_item_new_with_label(label.c_str());
			GtkWidget* image = gtk_image_new_from_file(this->iconPath.c_str());
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(newNativeItem), image);
		}

		gtk_widget_set_sensitive(GTK_WIDGET(newNativeItem), this->enabled);
		this->SetNativeItemSubmenu(newNativeItem, this->submenu);
		g_signal_connect(G_OBJECT(newNativeItem),
			"activate", G_CALLBACK(MenuCallback), this);

		if (registerNative)
				this->nativeItems.push_back(newNativeItem);
		return newNativeItem;
	}

	void MenuItem::DestroyNative(GtkMenuItem* nativeItem)
	{
		std::vector<GtkMenuItem*>::iterator i = this->nativeItems.begin();
		while (i != this->nativeItems.end())
		{
			GtkMenuItem* item = *i;
			if (item == nativeItem)
				i = this->nativeItems.erase(i);
			else
				i++;
		}

		this->SetNativeItemSubmenu(nativeItem, 0);
	}
}


