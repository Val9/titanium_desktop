/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../menu.h" 

namespace ti
{
	using std::vector;
	using std::map;
	using std::string;

	void Menu::Initialize()
	{
	}

	void Menu::Shutdown()
	{
		std::vector<GtkMenuShell* >::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end()) {
			this->ClearNativeMenu((*i++));
		}
		nativeMenus.clear();
	}

	void Menu::AppendItemImpl(AutoMenuItem item)
	{
		std::vector<GtkMenuShell*>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			GtkMenuShell* nativeMenu = (*i++);
			GtkMenuItem* nativeItem = item->CreateNative(true);
			gtk_menu_shell_append(GTK_MENU_SHELL(nativeMenu), GTK_WIDGET(nativeItem));
			gtk_widget_show(GTK_WIDGET(nativeItem));
		}

		this->oldChildren = this->children;
	}

	void Menu::InsertItemAtImpl(AutoMenuItem item, unsigned int index)
	{
		std::vector<GtkMenuShell*>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			GtkMenuShell* nativeMenu = (*i++);
			GtkMenuItem* nativeItem = item->CreateNative(true);
			gtk_menu_shell_insert(GTK_MENU_SHELL(nativeMenu), GTK_WIDGET(nativeItem), index);
			gtk_widget_show(GTK_WIDGET(nativeItem));
		}

		this->oldChildren = this->children;
	}

	void Menu::RemoveItemAtImpl(unsigned int index)
	{
		std::vector<GtkMenuShell*>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			GtkMenuShell* nativeMenu = (*i++);
			this->RemoveItemAtFromNativeMenu(nativeMenu, index);
		}

		this->oldChildren = this->children;
	}

	void Menu::ClearImpl()
	{
		std::vector<GtkMenuShell*>::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end())
		{
			GtkMenuShell* nativeMenu = (*i++);
			this->ClearNativeMenu(nativeMenu);
		}

		this->oldChildren = this->children;
	}

	void Menu::ClearNativeMenu(GtkMenuShell* nativeMenu)
	{
		GList* children = gtk_container_get_children(GTK_CONTAINER(nativeMenu));
		for (size_t i = 0; i < g_list_length(children); i++)
		{
			GtkMenuItem* w = static_cast<GtkMenuItem*>(g_list_nth_data(children, i));
			this->oldChildren.at(i)->DestroyNative(w);
			gtk_container_remove(GTK_CONTAINER(nativeMenu), GTK_WIDGET(w));
		}
	}

	void Menu::RemoveItemAtFromNativeMenu(GtkMenuShell* nativeMenu, unsigned int index)
	{
		GList* children = gtk_container_get_children(GTK_CONTAINER(nativeMenu));
		GtkMenuItem* w = (GtkMenuItem*) g_list_nth_data(children, index);
		this->oldChildren.at(index)->DestroyNative(w);
		gtk_container_remove(GTK_CONTAINER(nativeMenu), GTK_WIDGET(w));
	}

	void Menu::DestroyNative(GtkMenuShell* nativeMenu)
	{
		// Remove the native menu from our list of known native menus
		std::vector<GtkMenuShell* >::iterator i = this->nativeMenus.begin();
		while (i != this->nativeMenus.end()) {
			if (*i == nativeMenu)
				i = this->nativeMenus.erase(i);
			else
				i++;
		}

		// Clear the native menu and release, so that children will be freed
		this->ClearNativeMenu(nativeMenu);
	}

	GtkMenuShell* Menu::CreateNativeBar(bool registerNative)
	{
		GtkMenuShell* nativeMenu = GTK_MENU_SHELL(gtk_menu_bar_new());
		this->AddChildrenToNativeMenu(nativeMenu, registerNative);
		if (registerNative)
			this->nativeMenus.push_back(nativeMenu);
		return nativeMenu;
	}

	GtkMenuShell* Menu::CreateNative(bool registerNative)
	{
		GtkMenuShell* nativeMenu = GTK_MENU_SHELL(gtk_menu_new());
		this->AddChildrenToNativeMenu(nativeMenu, registerNative);
		if (registerNative)
			this->nativeMenus.push_back(nativeMenu);
		return nativeMenu;
	}

	void Menu::AddChildrenToNativeMenu(GtkMenuShell* nativeMenu, bool registerNative)
	{
		vector<AutoMenuItem>::iterator i = this->children.begin();
		while (i != this->children.end()) {
			AutoMenuItem item = *i++;
			GtkMenuItem* nativeItem = item->CreateNative(registerNative);
			gtk_menu_shell_append(GTK_MENU_SHELL(nativeMenu), GTK_WIDGET(nativeItem));
			gtk_widget_show_all(GTK_WIDGET(nativeItem));
		}
	}
}
