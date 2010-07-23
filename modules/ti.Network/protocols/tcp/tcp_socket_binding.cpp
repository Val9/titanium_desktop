/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "tcp_socket_binding.h"

namespace ti
{
	TCPSocket::TCPSocket(std::string host, int port) :
		KEventObject("Network.TCPSocket"),
		SocketObserver(socket),
		address(host, port),
		readBufferSize(1024)
	{
		this->SetMethod("connect", &TCPSocket::Connect);
		this->SetMethod("close", &TCPSocket::Close);
		this->SetMethod("write", &TCPSocket::Write);
		this->SetMethod("setKeepAlive", &TCPSocket::SetKeepAlive);
	}

	TCPSocket::~TCPSocket()
	{
	}

	void TCPSocket::Connect(const ValueList& args, KValueRef result)
	{
		try
		{
			// Put socket into non-blocking mode then begin connecting.
			Poco::Net::SocketAddress address(this->host, this->port);
			this->socket.connectNB(address, this->timeout);

			// Wait for writable event which confirms
			// we have successfully connected.
			this->SetEventHandler(SOCKET_WRITE, &TCPSocket::OnConnect);
		}
		catch (Poco::IOException& e)
		{
			this->FireErrorEvent(e.what());
		}
	}

	void TCPSocket::Close(const ValueList& args, KValueRef result)
	{
	}

	void TCPSocket::Write(const ValueList& args, KValueRef result)
	{
		args.VerifyException("write", "s");
	}

	void TCPSocket::SetKeepAlive(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setKeepAlive", "b");
		this->socket.setKeepAlive(args.GetBool(0));
	}

	void TCPSocket::OnConnect()
	{
		// If there is no errors, we should be connected now.
		int lastError = this->socket.impl()->lastError();
		if (lastError == 0)
		{
			this->SetEventHandler(SOCKET_READ, &TCPSocket::OnReadable);
			this->SetEventHandler(SOCKET_WRITE, &TCPSocket::OnWritable);
			this->FireEvent("connect");
		}
		else if (lastError != POCO_EINPROGRESS)
		{
			this->FireErrorEvent(lastError);
		}
	}

	void TCPSocket::OnRead()
	{
	}

	void TCPSocket::OnWrite()
	{
	}

	void TCPSocket::OnTimeout()
	{
	}

	void TCPSocket::OnError()
	{
	}
}

