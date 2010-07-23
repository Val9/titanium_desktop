/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include <Poco/Net/StreamSocket.h>

#include <kroll/kroll.h>
#include "socket_observer.h"

namespace ti
{
	class TCPSocket : public KEventObject, public SocketObserver<TCPSocket>
	{
	public:
		TCPSocket(std::string& host, int port);
		virtual ~TCPSocket();

		void Connect(const ValueList& args, KValueRef result);
		void Close(const ValueList& args, KValueRef result);
		void Write(const ValueList& args, KValueRef result);
		void SetKeepAlive(const ValueList& args, KValueRef result);
		void SetReadBufferSize(const ValueList& args, KValueRef result);

		// Socket event callbacks
		void OnConnect();
		void OnWritable();
		void OnReadable();
		void OnError();

	private:
		void FireErrorEvent(int socketErrorCode);

		Poco::Net::SocketAddress address;
		Poco::Net::StreamSocket socket;
		size_t readBufferSize;
	};
}

#endif
