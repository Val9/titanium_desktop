/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TINET_SOCKET_OBSERVER_H_
#define _TINET_SOCKET_OBSERVER_H_

#include <list>
#include <Poco/Net/Socket.h>
#include <Poco/Runnable.h>
#include <Poco/Mutex.h>

namespace ti
{
	class SocketReactor;

	enum SocketEvent
	{
		SOCKET_READ,
		SOCKET_WRITE,
		SOCKET_ERROR
	};

	class AbstractSocketObserver
	{
	public:
		virtual void BuildQuerySets(fd_set* read, fd_set* write, fd_set* error) = 0;
		virtual void ProcessResultSets(fd_set* read, fd_set* write, fd_set* error) = 0;
	};

	/**
	  * Socket event observer
	  */
	template <class C>
	class SocketObserver : public AbstractSocketObserver
	{
	public:
		typedef void (C::*Callback)();

		SocketObserver(Poco::Net::Socket& socket, C* object = 0) :
			socketfd(socket.sockfd())
		{
			this->object = (object == 0) ? this : object;

			this->callbacks[SOCKET_READ] = 0;
			this->callbacks[SOCKET_WRITE] = 0;
			this->callbacks[SOCKET_ERROR] = 0;
		}

		virtual ~SocketObserver()
		{
		}

		/**
		  * Once the observer has been registered with the reactor, you
		  * may only call this method from callbacks or on the same thread
		  * running the reactor.
		  */
		void SetEventHandler(SocketEvent event, Callback method)
		{
			this->callbacks[event] = method;
		}

		void BuildQuerySets(fd_set* read, fd_set* write, fd_set* error)
		{
			if (this->callbacks[SOCKET_READ])
				FD_SET(this->socketfd, read);
			if (this->callbacks[SOCKET_WRITE])
				FD_SET(this->socketfd, write);
			if (this->callbacks[SOCKET_ERROR])
				FD_SET(this->socketfd, error);
		}

		void ProcessResultSets(fd_set* read, fd_set* write, fd_set* error)
		{
			if (this->callbacks[SOCKET_READ] && FD_ISSET(this->socketfd, read))
				InvokeCallback(SOCKET_READ);
			if (this->callbacks[SOCKET_WRITE] && FD_ISSET(this->socketfd, write))
				InvokeCallback(SOCKET_WRITE);
			if (this->callbacks[SOCKET_ERROR] && FD_ISSET(this->socketfd, error))
				InvokeCallback(SOCKET_ERROR);
		}

		void InvokeCallback(SocketEvent event)
		{
			// When an event occurs, notify the observer by invoking
			// the callback method.
			(object->*callbacks[event])();
		}

	private:
		int socketfd;
		C* object;
		Callback callbacks[SOCKET_EVENT_COUNT];
	};

	/**
	 * The Socket Reactor is responsible for "pooling" events
	 * that occur on a set of sockets. These include when a socket
	 * becomes readable or writable. If such events occur then a read or
	 * write operation will not block. This is vital to creating a true
	 * async socket API.
	 */
	class SocketReactor : public Runnable
	{
	public:
		SocketReactor(long idleTimeout = 0);
		virtual ~SocketReactor();
		void run();

		// Perform one event loop then return.
		void Once();

		void RegisterObserver(AbstractSocketObserver* observer);
		void UnregisterObserver(AbstractSocketObserer* observer);

	private:
		typedef std::list<AbstractSocketObserver*> ObserverList;

		long idleTimeout;
		fd_set read, write, error;
		ObserverList observers;
		Poco::Mutex observersMutex;
	};
}

#endif