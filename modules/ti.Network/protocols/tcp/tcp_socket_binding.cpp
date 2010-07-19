/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <Poco/NObserver.h>
#include <Poco/RunnableAdapter.h>
#include <kroll/kroll.h>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	TCPSocketBinding::TCPSocketBinding(std::string host, int port) :
		KEventObject("Network.TCPSocket"),
		host(host), port(port)
	{
		this->SetMethod("connect", &TCPSocketBinding::Connect);
		this->SetMethod("setKeepAlive", &TCPSocketBinding::SetKeepAlive);
		this->SetMethod("close", &TCPSocketBinding::Close);
		this->SetMethod("write", &TCPSocketBinding::Write);
	}

	TCPSocketBinding::~TCPSocketBinding()
	{
		// TODO: Remove the reactor event handlers
	}

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		try
		{
			// Make connection without blocking
			Poco::Net::SocketAddress address(this->host, this->port);
			this->socket.connectNB(address, this->timeout);

			// Enable Keepalive mode on this socket. See:
			// http://tldp.org/HOWTO/TCP-Keepalive-HOWTO/index.html
			this->socket.setKeepAlive(true);
		}
		catch (Poco::IOException &e)
		{
			error.append(e.displayText());
		}
		catch (std::exception &e)
		{
			error.append(e.what());
		}
		catch (...)
		{
			error.append("Unknown exception");
		}

		if (!this->onError.isNull())
			RunOnMainThread(this->onError, ValueList(Value::NewString(error)), false);
	}

	void TCPSocketBinding::SetKeepAlive(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setKeepAlive", "b");
		this->socket.setKeepAlive(args.GetBool(0));
	}

	void TCPSocketBinding::Connect(const ValueList& args, KValueRef result)
	{
		int timeout = 10;
		if (args.size() > 0)
		{
			timeout = args.at(0)->ToInt();
		}

		if (this->opened || connectThread.isRunning())
		{
			throw ValueException::FromString("Socket is already open");
		}

		result->SetBool(true);
		this->timeout = Poco::Timespan(timeout, 0);
		connectThread.start(*connectAdapter);
	}

	void TCPSocketBinding::ReadyForRead(
		const Poco::AutoPtr<Poco::Net::ReadableNotification>& n)
	{
		static std::string eprefix("TCPSocketBinding::OnRead: ");

		try
		{
			// Always read bytes, so that the tubes get cleared.
			char data[BUFFER_SIZE + 1];
			int size = socket.receiveBytes(&data, BUFFER_SIZE);

			// A read is only complete if we've already read some bytes from the socket.
			bool readComplete = this->readStarted && (size <= 0);
			this->readStarted = (size > 0);

			if (readComplete && !this->onReadComplete.isNull())
			{
				ValueList args;
				RunOnMainThread(this->onReadComplete, args, false);
			}
			else if (size > 0 && !this->onRead.isNull())
			{
				data[size] = '\0';

				BytesRef bytes(new Bytes(data, size));
				ValueList args(Value::NewObject(bytes));
				RunOnMainThread(this->onRead, args, false);
			}
		}
		catch (ValueException& e)
		{
			GetLogger()->Error("Read failed: %s", e.ToString().c_str());
			ValueList args(Value::NewString(e.ToString()));

			if (!this->onError.isNull())
				RunOnMainThread(this->onError, args, false);
		}
		catch (Poco::Exception &e)
		{
			GetLogger()->Error("Read failed: %s", e.displayText().c_str());
			ValueList args(Value::NewString(e.displayText()));

			if (!this->onError.isNull())
				RunOnMainThread(this->onError, args, false);
		}
		catch (...)
		{
			GetLogger()->Error("Read failed: unknown exception");
			ValueList args(Value::NewString("Unknown exception during read"));

			if (!this->onError.isNull())
				RunOnMainThread(this->onError, args, false);
		}
	}

	void TCPSocketBinding::ReadyForWrite(
		const Poco::AutoPtr<Poco::Net::WritableNotification>& n)
	{
		if (sendData.empty())
			return;

		BytesRef buffer(0);
		{
			Poco::Mutex::ScopedLock lock(sendDataMutex);
			buffer = sendData.front();
		}

		const char* data = buffer->Get() + currentSendDataOffset;
		size_t length = buffer->Length() - currentSendDataOffset;
		size_t count = this->socket.sendBytes(data, length);
		currentSendDataOffset += count;

		if (currentSendDataOffset == (size_t) buffer->Length())
		{
			// Only send the onWrite message when we've exhausted a Bytes.
			if (!this->onWrite.isNull())
			{
				ValueList args(Value::NewInt(buffer->Length()));
				RunOnMainThread(this->onWrite, args, false);
			}

			Poco::Mutex::ScopedLock lock(sendDataMutex);
			sendData.pop();
			currentSendDataOffset = 0;

			// Uninstall the ReadyForWrite reactor handler, because it will push
			// the CPU to 100% usage if there is nothing to write.
			if (sendData.size() == 0)
			{
				this->reactor.removeEventHandler(this->socket, writeObserver);
				writeReadyHandlerInstalled = false;
			}
		}
	}

	void TCPSocketBinding::OnTimeout(
		const Poco::AutoPtr<Poco::Net::TimeoutNotification>& n)
	{
		if (this->onTimeout.isNull())
		{
			return;
		}
		RunOnMainThread(this->onTimeout, ValueList(), false);
	}

	void TCPSocketBinding::OnError(
		const Poco::AutoPtr<Poco::Net::ErrorNotification>& n)
	{
		if (this->onError.isNull())
		{
			return;
		}
		ValueList args(Value::NewString(n->name()));
		RunOnMainThread(this->onError, args, false);
	}

	void TCPSocketBinding::Write(const ValueList& args, KValueRef result)
	{
		args.VerifyException("Write", "o|s");

		static std::string eprefix("TCPSocketBinding::Write: ");
		if (!this->opened && !this->connectThread.isRunning())
			throw ValueException::FromString(eprefix +  "Socket is not open");

		BytesRef data(0);
		if (args.at(0)->IsString())
		{
			std::string sendString(args.GetString(0));
			data = new Bytes(sendString.c_str(), sendString.size());
		}
		else if (args.at(0)->IsObject())
		{
			KObjectRef dataObject(args.GetObject(0));
			data = dataObject.cast<Bytes>();
		}

		if (data.isNull())
			throw ValueException::FromString("Cannot send non-Bytes object");

		{
			Poco::Mutex::ScopedLock lock(sendDataMutex);
			sendData.push(data);

			// Only install the ReadyForWrite handler when there is actually data
			// to write, because otherwise the CPU usage will spike to 100%
			if (!writeReadyHandlerInstalled)
			{
				this->reactor.addEventHandler(this->socket, writeObserver);
				writeReadyHandlerInstalled = true;
			}
		}

		result->SetBool(true);
	}

	void TCPSocketBinding::Close(const ValueList& args, KValueRef result)
	{
		if (this->opened)
		{
			this->opened = false;
			this->reactor.stop();
			this->socket.close();
			result->SetBool(true);
		}
		else
		{
			result->SetBool(false);
		}
	}
}

