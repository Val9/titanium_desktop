/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "tcp_socket.h"

#include <Poco/ThreadPool.h>
#include <Poco/Timespan.h>

#define READ_BUFFER_SIZE 40*1024
#define READ_BUFFER_MIN_SIZE 128

namespace ti
{
	TCPSocket::TCPSocket(std::string& host, int port) :
		KEventObject("Network.TCPSocket"),
		address(host, port),
		socket(address.family()),
		closed(true),
		reader(*this, &TCPSocket::ReadThread),
		writer(*this, &TCPSocket::WriteThread)
	{
		SetMethod("connect", &TCPSocket::_Connect);
		SetMethod("setTimeout", &TCPSocket::_SetTimeout);
		SetMethod("close", &TCPSocket::_Close);
		SetMethod("isClosed", &TCPSocket::_IsClosed);
		SetMethod("write", &TCPSocket::_Write);
		SetMethod("onRead", &TCPSocket::_OnRead);
		SetMethod("onReadComplete", &TCPSocket::_OnReadComplete);
		SetMethod("onError", &TCPSocket::_OnError);
		SetMethod("onTimeout", &TCPSocket::_OnTimeout);
	}

	TCPSocket::~TCPSocket()
	{
		if (!this->closed)
			Close();
	}

	void TCPSocket::Connect()
	{
		if (!this->closed)
			throw ValueException::FromString("socket is already connected");

		// Start up the reading thread.
		// This thread will establish the connection
		// and then begin reading data from the socket.
		this->closed = false;
		this->readThread.start(reader);
	}

	void TCPSocket::Close()
	{
		if (this->closed)
			throw ValueException::FromString("socket is already closed");

		this->socket.close();
		this->closed = true;
		FireEvent("close");
	}

	void TCPSocket::Write(BytesRef data)
	{
		if (this->closed)
			throw ValueException::FromString("socket is not connected");

		Poco::FastMutex::ScopedLock lock(this->writeQueueMutex);

		bool startWriter = this->writeQueue.empty();
		this->writeQueue.push_back(data);
		if (startWriter)
		{
			Poco::ThreadPool::defaultPool().start(writer);
		}
	}

	void TCPSocket::SetKeepAlive(bool enable)
	{
		this->socket.setKeepAlive(enable);
	}

	void TCPSocket::SetTimeout(long milliseconds)
	{
		try
		{
			Poco::Timespan t(0, milliseconds * 1000);
			this->socket.setReceiveTimeout(t);
		}
		catch (Poco::Exception& e)
		{
			throw ValueException::FromFormat("setTimeout failed: %s", e.what());
		}
	}

	void TCPSocket::ReadThread()
	{
		try
		{
			this->socket.connect(this->address);
			this->FireEvent("connect");
		}
		catch (Poco::Exception& e)
		{
			// Failed to connect socket, report error
			this->FireErrorEvent(e);
			return;
		}

		BytesRef buffer = new Bytes(READ_BUFFER_SIZE);
		size_t usedSpace = 0;

		while (true)
		{
			// Re-allocate a new read buffer if the current
			// one has become too small.
			int freeSpace = READ_BUFFER_SIZE - usedSpace;
			if (freeSpace < READ_BUFFER_MIN_SIZE)
			{
				buffer = new Bytes(READ_BUFFER_SIZE);
				usedSpace = 0;
				freeSpace = READ_BUFFER_SIZE;
			}

			// Attempt to read data from socket into buffer.
			try
			{
				char* bufferPtr = buffer->Pointer() + usedSpace;
				int bytesRecv = this->socket.receiveBytes(bufferPtr, freeSpace);
				if (bytesRecv > 0)
				{
					BytesRef data = new Bytes(buffer, usedSpace, bytesRecv);
					usedSpace += bytesRecv;
					this->FireEvent("data", ValueList(Value::NewObject(data)));
				}
				else
				{
					// Remote host has closed their end, so we will no longer
					// recv any new data.
					FireEvent("end");
					break;
				}
			}
			catch (Poco::TimeoutException& e)
			{
				this->FireEvent("timeout");
			}
			catch (Poco::Exception& e)
			{
				this->FireErrorEvent(e);
				break;
			}
		}
	}

	void TCPSocket::WriteThread()
	{
		while (true)
		{
			{
				Poco::FastMutex::ScopedLock lock(this->writeQueueMutex);
				if (this->writeQueue.empty())
				{
					FireEvent("drain");
					break;
				}

				BytesRef data = this->writeQueue.front();
				this->writeQueue.pop_front();

				// Push back any unsent data to from of queue for sending later.
				try
				{
					BytesRef unsent = this->Send(data);
					if (!unsent.isNull())
						this->writeQueue.push_front(unsent);
				}
				catch (Poco::Exception& e)
				{
					FireErrorEvent(e);
					break;
				}
			}

			Poco::Thread::sleep(250);
		}
	}

	BytesRef TCPSocket::Send(BytesRef data)
	{
		int length = data->Length();
		int bytesSent = this->socket.sendBytes(data->Pointer(), length);
		if (bytesSent < length)
		{
			BytesRef unsentData = new Bytes(data, bytesSent);
			return unsentData;
		}

		return 0;
	}

	void TCPSocket::_Connect(const ValueList& args, KValueRef result)
	{
		Connect();
	}

	void TCPSocket::_SetTimeout(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setTimeout", "n");
		SetTimeout(args.GetNumber(0));
	}

	void TCPSocket::_Close(const ValueList& args, KValueRef result)
	{
		Close();
	}

	void TCPSocket::_IsClosed(const ValueList& args, KValueRef result)
	{
		result->SetBool(this->closed);
	}

	void TCPSocket::_Write(const ValueList& args, KValueRef result)
	{
		args.VerifyException("write", "s|o");

		BytesRef data;
		if (args.at(0)->IsString())
		{
			std::string dataStr(args.GetString(0));
			data = new Bytes(dataStr);
		}
		else
		{
			data = args.GetObject(0).cast<Bytes>();
			if (data.isNull())
			{
				throw ValueException::FromString("Argument is not Bytes object");
			}
		}

		Write(data);
	}

	void TCPSocket::_OnRead(const ValueList& args, KValueRef result)
	{
		args.VerifyException("onRead", "m");
		AddEventListener("data", args.GetMethod(0));
	}

	void TCPSocket::_OnReadComplete(const ValueList& args, KValueRef result)
	{
		args.VerifyException("onReadComplete", "m");
		AddEventListener("end", args.GetMethod(0));
	}

	void TCPSocket::_OnError(const ValueList& args, KValueRef result)
	{
		args.VerifyException("onError", "m");
		AddEventListener("error", args.GetMethod(0));
	}

	void TCPSocket::_OnTimeout(const ValueList& args, KValueRef result)
	{
		args.VerifyException("onTimeout", "m");
		AddEventListener("timeout", args.GetMethod(0));
	}
}

