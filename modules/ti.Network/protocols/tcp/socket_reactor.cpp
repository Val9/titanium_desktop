/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "socket_reactor.h"

namespace ti
{
	SocketReactor::SocketReactor(long timeout)
	{
		this->maxFD = -1;
		this->timeout.tv_sec = timeout;
	}

	SocketReactor::~SocketReactor()
	{
		Shutdown();
	}

	void SocketReactor::run()
	{
	}

	void SocketReactor::RunOnce()
	{
		// Build the set of sockets we want to watch.
		if (!BuildSelectSets()) return;

		// Perform the select() system call.
		int ret = select(maxFD + 1, &read, &write, &error, &timeout);
		if (ret < 0)
		{
			// TODO: handle error
			return;
		}

		// Process sockets with events and alert listeners.
		if (ret > 0) NotifyListeners();
	}

	void SocketReactor::AddEventListener(SocketEventListener* listener)
	{
		Poco::Mutex::ScopedLock lock(this->listenersMutex);
		this->listeners.push_back(listener);

		#ifndef OS_WIN32
		if (listener->socket > this->maxFD)
			this->maxFD = listener->socket;
		#endif
	}

	void SocketReactor::RemoveEventListener(SocketEventListener* listener)
	{
		Poco::Mutex::ScopedLock lock(this->listenersMutex);

		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			if (*i == listener)
			{
				this->listeners.erase(i);

				#ifndef OS_WIN32
				if (listener->socket > this->maxFD)
					FindMaxFD();
				#endif

				break;
			}
			i++;
		}
	}

	void SocketReactor::BuildSelectSets()
	{
		Poco::Mutex::ScopedLock lock(this->listenerMutex);

		if (this->listeners.empty())
			return false;

		FD_ZERO(&this->read);
		FD_ZERO(&this->write);
		FD_ZERO(&this->error);

		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			SocketEventListener* listener = *i++;
			FD_SET(listener->socket, &this->fdSets[listener->event]);
		}

		return true;
	}

	void SocketReactor::NotifyListeners()
	{
		Poco::Mutex::ScopedLock lock(this->listenerMutex);

		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			SocketEventListener* listener = *i++;
			if (FD_ISSET(listener->socket, &this->fdSets[listener->event]))
			{
				listener->Handle();
			}
		}
	}

	void SocketReactor::FindMaxFD()
	{
		this->maxFD = -1;
		EventListenerList::iterator i = this->listeners.begin();
		while (i != this->listeners.end())
		{
			SocketEventListener* listener = *i++;
			if (listener->socket > this->maxFD)
				this->maxFD = listener->socket;
		}
	}
}