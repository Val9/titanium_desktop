/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include <dequeue>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Mutex.h>
#include <kroll/kroll.h>

namespace ti
{
	class TCPSocket : public KEventObject, public SocketObserver<TCPSocket>
	{
	public:
		TCPSocket(std::string& host, int port);
		virtual ~TCPSocket();

		void Connect();
		void Close();
		bool Write(BytesRef data);
		bool Flush();
		void SetKeepAlive(bool enable);

		// Socket event callbacks
		void OnConnect();
		void OnWritable();
		void OnReadable();
		void OnError();

	private:
		// Write data to socket and if all data was sent, return true.
		// Any unsent data will be queued for later and the method will return false.
		bool WriteImpl(BytesRef data);

		// Fire an error event for the given socket error code.
		void FireErrorEvent(int socketErrorCode);

		Poco::Net::SocketAddress address;
		Poco::Net::StreamSocket socket;

		typedef std::dequeue<BytesRef> BytesQueue;
		BytesQueue writeQueue;
		Poco::FastMutex writeQueueMutex;

		static BytesRef readBuffer;
		static size_t readBufferUsed;
	};
}

#endif
