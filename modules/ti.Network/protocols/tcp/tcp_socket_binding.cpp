/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "tcp_socket_binding.h"

#ifdef OS_WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#define READ_BUFFER_SIZE 40*1024
#define READ_BUFFER_MIN_SIZE 128

namespace ti
{
	TCPSocket::TCPSocket(std::string host, int port) :
		KEventObject("Network.TCPSocket"),
		SocketObserver(socket),
		address(host, port)
	{
	}

	TCPSocket::~TCPSocket()
	{
	}

	void TCPSocket::Connect()
	{
		try
		{
			// Put socket into non-blocking mode and connect.
			Poco::Net::SocketAddress address(this->host, this->port);
			this->socket.connectNB(address, this->timeout);

			// Once the socket becomes writable we should be connected.
			this->SetEventHandler(SOCKET_WRITE, &TCPSocket::OnConnect);
		}
		catch (Poco::IOException& e)
		{
			this->FireErrorEvent(e.what());
		}
	}

	void TCPSocket::Close()
	{
	}

	bool TCPSocket::Write(BytesRef data)
	{
		Poco::FastMutex::ScopedLock lock(this->writeQueueMutex);

		if (this->writeQueueMutex.empty())
		{
			// If the write queue is empty, just write directly to socket.
			return this->WriteImpl(data);
		}
		else
		{
			// Append data to write queue so it can be
			// sent to socket on next flush operation.
			this->writeQueue.push_back(data);
			return false;
		}
	}

	bool TCPSocket::Flush()
	{
		Poco::FastMutex::ScopedLock lock(this->writeQueueMutex);

		while (!this->writeQueue.empty())
		{
			BytesRef data = this->writeQueue.front();
			this->writeQueue.pop_front();
			if (!this->WriteImpl(data))
			{
				// Only partially sent the last data,
				// stop flushing until the next write event.
				return false;
			}
		}

		this->SetEventHandler(SOCKET_WRITE, 0);
		return true;
	}

	void TCPSocket::SetKeepAlive(bool enable)
	{
		this->socket.setKeepAlive(enable);
	}

	void TCPSocket::OnConnect()
	{
		// If there is no errors, we should be connected now.
		int lastError = this->socket.impl()->lastError();
		if (lastError == 0)
		{
			this->SetEventHandler(SOCKET_READ, &TCPSocket::OnReadable);
			this->FireEvent("connect");
		}
		else if (lastError != POCO_EINPROGRESS)
		{
			this->FireErrorEvent(lastError);
		}
	}

	void TCPSocket::OnRead()
	{
		// If the free space in our read buffer falls below
		// the min, allocate a new buffer.
		size_t freeSpace = readBuffer->Length() - readBufferUsed;
		if (freeSpace < READ_BUFFER_MIN_SIZE)
		{
			readBuffer = new Bytes(READ_BUFFER_SIZE);
			freeSpace = READ_BUFFER_SIZE;
		}

		// Read data from socket and store into our buffer.
		// Note: Poco's recv function does some extra un-wanted magic, so
		// directly use the recv() function.
		int bytesRecv = recv(this->socket.sockfd(), readBuffer->Pointer(), freeSpace);

		if (bytesRecv < 0)
		{
			// Ignore these errors and just perform another read later.
			if (errno == EAGAIN || errno == EINTR) return;
			this->FireErrorEvent(errno);
		}
		else
		{
			// Slice out the data we just read into a new bytes object.
			BytesRef dataRead = new Bytes(readBuffer, readBufferUsed, readBufferUsed + bytesRecv);
			readBufferUsed += bytesRecv;

			// Fire 'data' event passing along the data read
			this->FireEvent("data", ValueList(dataRead));
		}
	}

	void TCPSocket::OnWrite()
	{
		// Flush any data we have queued to be sent.
		if (this->Flush())
		{
			// If all queued data was flushed, emit the 'drain' event
			// to signal the write queue is now empty and ready for more data.
			this->FireEvent("drain");
		}
	}

	void TCPSocket::OnError()
	{
		this->FireErrorEvent(errno);
	}

	bool TCPSocket::WriteImpl(BytesRef data)
	{
		// Send data using send() since Poco does some unwanted magic.
		int bytesSent = send(this->socket.sockfd(), data->Pointer(), data->Length());

		if (bytesSent < 0)
		{
			if (errno == EAGAIN || errno == EINTR)
			{
				// If these errors occur, just queue the data so we
				// can try sending it later.
				bytesSent = 0;
			}
			else
			{
				this->FireErrorEvent(errno);
				return false;
			}
		}

		if (bytesSent < data->Length())
		{
			// Push any unsent data onto front of write queue.
			BytesRef unsent = new Bytes(data, bytesSent);
			this->writeQueue.push_front(unsent);
			this->SetEventHandler(SOCKET_WRITE, &TCPSocket::OnWritable);
			return false;
		}

		// All data sent!
		return true;
	}
}

