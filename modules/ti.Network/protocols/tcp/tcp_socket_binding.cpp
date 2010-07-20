/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"

#include <Poco/NObserver.h>

namespace ti
{
	TCPSocketBinding::TCPSocketBinding(std::string host, int port) :
		KEventObject("Network.TCPSocket"),
		address(host, port)
	{
		this->SetMethod("connect", &TCPSocketBinding::Connect);
		this->SetMethod("close", &TCPSocketBinding::Close);
		this->SetMethod("write", &TCPSocketBinding::Write);
		this->SetMethod("setKeepAlive", &TCPSocketBinding::SetKeepAlive);
	}

	TCPSocketBinding::~TCPSocketBinding()
	{
	}

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		try
		{
			// Make connection without blocking
			Poco::Net::SocketAddress address(this->host, this->port);
			this->socket.connectNB(address, this->timeout);
		}
		catch (Poco::IOException& e)
		{
			this->FireErrorEvent(e.what());
		}
	}

	void TCPSocketBinding::Close(const ValueList& args, KValueRef result)
	{
	}

	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		args.VerifyException("write", "s");
	}

	void TCPSocketBinding::SetKeepAlive(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setKeepAlive", "b");
		this->socket.setKeepAlive(args.GetBool(0));
	}

	void TCPSocketBinding::OnRead(const Poco::AutoPtr<Poco::Net::ReadableNotification>& n)
	{
	}

	void TCPSocketBinding::OnWrite(const Poco::AutoPtr<Poco::Net::WritableNotification>& n)
	{
	}

	void TCPSocketBinding::OnTimeout(const Poco::AutoPtr<Poco::Net::TimeoutNotification>& n)
	{
	}

	void TCPSocketBinding::OnError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& n)
	{
	}
}

