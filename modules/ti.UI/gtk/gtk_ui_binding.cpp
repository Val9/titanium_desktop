/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_binding.h"

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <Poco/Thread.h>
#include <libsoup/soup.h>
#include <libsoup/soup-gnome.h>

namespace ti
{
	void UIBinding::Initialize()
	{
		// Prepare the custom URL handlers
		webkit_titanium_set_normalize_url_cb(NormalizeURLCallback);
		webkit_titanium_set_url_to_file_url_cb(URLToFileURLCallback);
		webkit_titanium_set_can_preprocess_cb(CanPreprocessURLCallback);
		webkit_titanium_set_preprocess_cb(PreprocessURLCallback);

		// Setup libsoup proxy support
		SoupSession* session = webkit_get_default_session();
		soup_session_add_feature_by_type(session, SOUP_TYPE_PROXY_RESOLVER_GNOME);

		std::string webInspectorPath(host->GetApplication()->runtime->path);
		webInspectorPath = FileUtils::Join(webInspectorPath.c_str(), "webinspector", NULL);
		webkit_titanium_set_inspector_url(webInspectorPath.c_str());

		//webkit_set_cache_model(WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);
	}

	void UIBinding::Shutdown()
	{
	}

	void UIBinding::SetIcon(std::string& iconPath)
	{
		this->iconPath = iconPath;
	}

	long UIBinding::GetIdleTime()
	{
		Display *display = gdk_x11_get_default_xdisplay();
		if (display == NULL)
			return -1;
		int screen = gdk_x11_get_default_screen();

		XScreenSaverInfo *mit_info = XScreenSaverAllocInfo();
		XScreenSaverQueryInfo(display, RootWindow(display, screen), mit_info);
		long idle_time = mit_info->idle;
		XFree(mit_info);

		return idle_time;
	}

	/*static*/
	void UIBinding::ErrorDialog(std::string msg)
	{
		GtkWidget* dialog = gtk_message_dialog_new(
			NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"%s",
			msg.c_str());
		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);
	}
}
