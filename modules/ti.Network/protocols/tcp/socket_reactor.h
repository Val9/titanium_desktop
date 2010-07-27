/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TINET_SOCKET_REACTOR_H_
#define _TINET_SOCKET_REACTOR_H_

#include <list>
#include <Poco/Runnable.h>
#include <Poco/Mutex.h>

namespace ti
{
	class SocketEventListener;

	enum SocketEvent
	{
		SOCKET_READABLE,
		SOCKET_WRITABLE,
		SOCKET_ERROR,

		SOCKET_EVENT_COUNT
	};

	/**
	 * The socket reactor monitors a collection of sockets
	 * for events and notifies listeners when they occur.
	 */
	class SocketReactor : public Runnable
	{
	public:
		SocketReactor(long timeout = 5);
		virtual ~SocketReactor();
		void run();

		// Perform one event loop then return.
		void RunOnce();

		// Signals for the reactor to shutdown.
		void Shutdown();

		void AddEventListener(SocketEventListener* listener);
		void RemoveEventListener(SocketEventListener* listener);

	private:
		bool BuildSelectSets();
		void NotifyListeners();
		void FindMaxFD();

		typedef std::list<SocketEventListener*> EventListenerList;
		EventListenerList listeners;
		Poco::Mutex listenersMutex;

		fd_set fdSets[SOCKET_EVENT_COUNT];
		int maxFD;
		struct timeval timeout;
	};

	/**
	 * Notified when a socket event has occured of which
	 * the listener has subscribed.
	 */
	class SocketEventListener
	{
	public:
		SocketEventListener(int socket, SocketEvent event);
		virtual ~SocketEventListener() {}

		// Called when the event being listened for has occurred.
		virtual void Handle();

	private:
		int socket;
		SocketEvent event;

		friend SocketReactor;
	};
}

#endif