/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "tcp_socket_binding.h"
#include <Poco/NObserver.h>
#include <kroll/kroll.h>

#define BUFFER_SIZE 1024   // choose a reasonable size to send back to JS

namespace ti
{
	TCPSocketBinding::TCPSocketBinding(Host* ti_host, std::string host, int port) :
		ti_host(ti_host), host(host), port(port), opened(false), 
		onRead(NULL), onWrite(NULL), onTimeout(NULL), onReadComplete(NULL)
	{
		// methods
		this->SetMethod("connect",&TCPSocketBinding::Connect);
		this->SetMethod("close",&TCPSocketBinding::Close);
		this->SetMethod("write",&TCPSocketBinding::Write);
		this->SetMethod("isClosed",&TCPSocketBinding::IsClosed);

		// event handler callbacks
		this->SetMethod("onRead",&TCPSocketBinding::SetOnRead);
		this->SetMethod("onWrite",&TCPSocketBinding::SetOnWrite);
		this->SetMethod("onTimeout",&TCPSocketBinding::SetOnTimeout);
		this->SetMethod("onReadComplete",&TCPSocketBinding::SetOnReadComplete);

		// our reactor event handlers
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, ReadableNotification>(*this, &TCPSocketBinding::OnRead));
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, WritableNotification>(*this, &TCPSocketBinding::OnWrite));
		this->reactor.addEventHandler(this->socket,NObserver<TCPSocketBinding, TimeoutNotification>(*this, &TCPSocketBinding::OnTimeout));
	}
	TCPSocketBinding::~TCPSocketBinding()
	{
		if (this->opened)
		{
			this->reactor.stop();
			this->socket.close();
		}
	}
	void TCPSocketBinding::SetOnRead(const ValueList& args, SharedValue result)
	{
		this->onRead = args.at(0)->ToMethod();
	}
	void TCPSocketBinding::SetOnWrite(const ValueList& args, SharedValue result)
	{
		this->onWrite = args.at(0)->ToMethod();
	}
	void TCPSocketBinding::SetOnTimeout(const ValueList& args, SharedValue result)
	{
		this->onTimeout = args.at(0)->ToMethod();
	}
	void TCPSocketBinding::SetOnReadComplete(const ValueList& args, SharedValue result)
	{
		this->onReadComplete = args.at(0)->ToMethod();
	}
	void TCPSocketBinding::IsClosed(const ValueList& args, SharedValue result)
	{
		return result->SetBool(!this->opened);
	}
	void TCPSocketBinding::Connect(const ValueList& args, SharedValue result)
	{
		std::string eprefix = "Connect exception: ";
		if (this->opened)
		{
			throw ValueException::FromString(eprefix + "Socket is already open");
		}
		try
		{
			SocketAddress a(this->host.c_str(),this->port);
			this->socket.connectNB(a);
			this->thread.start(this->reactor);
			this->opened = true;
			result->SetBool(true);
		}
		catch(Poco::IOException &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}
		catch(std::exception &e)
		{
			throw ValueException::FromString(eprefix + e.what());
		}
		catch(...)
		{
			throw ValueException::FromString(eprefix + "Unknown exception");
		}
	}
	void TCPSocketBinding::OnRead(const Poco::AutoPtr<ReadableNotification>& n)
	{
		std::string eprefix = "TCPSocketBinding::OnRead: ";
		try
		{
			// Always read bytes, so that the tubes get cleared.
			char data[BUFFER_SIZE + 1];
			int size = socket.receiveBytes(&data, BUFFER_SIZE);

			bool read_complete = (size <= 0);
			if (read_complete && !this->onReadComplete.isNull())
			{
				ValueList args;
				ti_host->InvokeMethodOnMainThread(this->onReadComplete, args, false);
			}
			else if (!read_complete && !this->onRead.isNull())
			{
				data[size] = '\0';

				ValueList args;
				args.push_back(Value::NewString(data));
				ti_host->InvokeMethodOnMainThread(this->onRead, args, false);
			}
		}
		catch(ValueException& e)
		{
			std::cerr << eprefix << *(e.GetValue()->DisplayString()) << std::endl;
		}
		catch(Poco::Exception &e)
		{
			std::cerr << eprefix << e.displayText() << std::endl;
		}
		catch(...)
		{
			std::cerr << eprefix << "Unknown exception" << std::endl;
		}
	}
	void TCPSocketBinding::OnWrite(const Poco::AutoPtr<WritableNotification>& n)
	{
		if (this->onWrite.isNull())
		{
			return;
		}
		ValueList args;
		ti_host->InvokeMethodOnMainThread(this->onWrite, args, false);
	}
	void TCPSocketBinding::OnTimeout(const Poco::AutoPtr<TimeoutNotification>& n)
	{
		if (this->onTimeout.isNull())
		{
			return;
		}
		ValueList args;
		ti_host->InvokeMethodOnMainThread(this->onTimeout, args, false);
	}
	void TCPSocketBinding::Write(const ValueList& args, SharedValue result)
	{
		std::string eprefix = "TCPSocketBinding::Write: ";
		if (!this->opened)
		{
			throw ValueException::FromString(eprefix +  "Socket is not open");
		}

		try
		{
			std::string buf = args.at(0)->ToString();
			int count = this->socket.sendBytes(buf.c_str(),buf.length());
			result->SetInt(count);
		}
		catch(Poco::Exception &e)
		{
			throw ValueException::FromString(eprefix + e.displayText());
		}

	}
	void TCPSocketBinding::Close(const ValueList& args, SharedValue result)
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

