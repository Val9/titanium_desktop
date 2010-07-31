/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "tcp_socket.h"

#include <Poco/ThreadPool.h>

#define READ_BUFFER_SIZE 40*1024
#define READ_BUFFER_MIN_SIZE 128

namespace ti
{
	TCPSocket::TCPSocket(std::string& host, int port) :
		KEventObject("Network.TCPSocket"),
		address(host, port),
		closed(true),
		reader(*this, &TCPSocket::ReadThread),
		writer(*this, &TCPSocket::WriteThread)
	{
		SetMethod("connect", &TCPSocket::_Connect);
		SetMethod("close", &TCPSocket::_Close);
		SetMethod("isClosed", &TCPSocket::_IsClosed);
		SetMethod("write", &TCPSocket::_Write);
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

		this->socket.shutdown();
		this->closed = true;
		FireEvent("close");
	}

	bool TCPSocket::Write(BytesRef data)
	{
		if (this->closed)
			throw ValueException::FromString("socket is not connected");

		Poco::FastMutex::ScopedLock lock(this->writeQueueMutex);
		BytesRef queueData;

		// Send to socket directly if no data is queued for writing.
		if (this->writeQueue.empty())
		{
			try
			{
				queueData = this->Send(data);
			}
			catch (Poco::Exception& e)
			{
				FireErrorEvent(e);
			}
		}
		else
		{
			queueData = data;
		}

		// Push any unsent data into write queue to be sent later
		// on our write thread.
		if (!queueData.isNull())
		{
			AppendWriteQueue(queueData);
			return false;
		}

		return true;
	}

	void TCPSocket::SetKeepAlive(bool enable)
	{
		this->socket.setKeepAlive(enable);
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

	void TCPSocket::AppendWriteQueue(BytesRef data)
	{
		bool startWriter = this->writeQueue.empty();
		this->writeQueue.push_back(data);
		if (startWriter)
		{
			Poco::ThreadPool::defaultPool().start(writer);
		}
	}

	void TCPSocket::_Connect(const ValueList& args, KValueRef result)
	{
		Connect();
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

		result->SetBool(Write(data));
	}
}

