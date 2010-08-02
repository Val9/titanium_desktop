/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TINET_TCP_SOCKET_H_
#define _TINET_TCP_SOCKET_H_

#include <deque>
#include <string>

#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Mutex.h>

#include <kroll/kroll.h>

namespace ti
{
	class TCPSocket : public KEventObject
	{
	public:
		TCPSocket(std::string& host, int port);
		virtual ~TCPSocket();

		void Connect();
		void Close();
		bool Write(BytesRef data);
		void SetKeepAlive(bool enable);
		void SetTimeout(long milliseconds);

		void ReadThread();
		void WriteThread();

	private:
		BytesRef Send(BytesRef data);
		void AppendWriteQueue(BytesRef data);

		// Bindings
		void _Connect(const ValueList& args, KValueRef result);
		void _SetTimeout(const ValueList& args, KValueRef result);
		void _Close(const ValueList& args, KValueRef result);
		void _IsClosed(const ValueList& args, KValueRef result);
		void _Write(const ValueList& args, KValueRef result);
		void _OnRead(const ValueList& args, KValueRef result);
		void _OnReadComplete(const ValueList& args, KValueRef result);
		void _OnError(const ValueList& args, KValueRef result);
		void _OnTimeout(const ValueList& args, KValueRef result);

		Poco::Net::SocketAddress address;
		Poco::Net::StreamSocket socket;
		bool closed;

		Poco::Thread readThread;
		Poco::RunnableAdapter<TCPSocket> reader, writer;

		typedef std::deque<BytesRef> BytesQueue;
		BytesQueue writeQueue;
		Poco::FastMutex writeQueueMutex;
	};
}

#endif
