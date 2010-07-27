/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "socket_observer.h"

namespace ti
{
	SocketReactor::SocketReactor(long idleTimeout) :
		idleTimeout(idleTimeout)
	{
	}

	SocketReactor::~SocketReactor()
	{
	}

	void SocketReactor::run()
	{
	}

	void SocketReactor::Once()
	{
		// First build a set of sockets we want to monitor for events.
		{
			Poco::Mutex::ScopedLock(this->observersMutex);
			if (this->observers.empty())
				return;

			ObserverList::iterator i = this->observers.begin();
			while (i != this->observers.end())
			{
				AbstractSocketObserver
			}
		}
	}

	void SocketReactor::RegisterObserver(SocketObserver* observer)
	{
		Poco::Mutex::ScopedLock lock(this->observersMutex);
		this->observers.push_back(observer);
	}

	void SocketReactor::UnregisterObserver(SocketObserver* observer)
	{
		Poco::Mutex::ScopedLock lock(this->observersMutex);
		this->observers.remove(observer);
	}
}
