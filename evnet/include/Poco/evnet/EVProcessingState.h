//
// EVProcessingState.h
//
// Library: EVProcessingState
// Package: evnet
// Module:  EVProcessingState
//
// Basic definitions for the Poco evnet library.
// This file must be the first file included by every other evnet
// header file.
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include <chunked_memory_stream.h>
#include <ev_queue.h>
#include "Poco/Net/Net.h"
#include "Poco/evnet/evnet.h"
#include "Poco/evnet/EVServer.h"
#include "Poco/evnet/EVConnectedStreamSocket.h"

#include <map>
#ifndef EVNet_EVProcessingState_INCLUDED
#define EVNet_EVProcessingState_INCLUDED

namespace Poco {
namespace evnet {

class Net_API EVProcessingState
	// This class is used as a marker to hold state data of processing of a request in a connection.
	// In case of event driven model of processing, the processing of a request may have to be
	// suspended mulptiple times, while data is being fetched from sources (e.g. client)
	//
	// The processing of the request is coded in such a way, that all the intermediate data is held within
	// a derivation of this base class and the state is destroyed at the end of processing of the request.
{
public:
	typedef std::map<poco_socket_t,EVConnectedStreamSocket *> CSColMapType;

	EVProcessingState(EVServer * server);
	virtual int getState() = 0;
	virtual ~EVProcessingState();
	virtual void setReqMemStream(chunked_memory_stream *memory_stream) = 0;
	virtual void setResMemStream(chunked_memory_stream *memory_stream) = 0;
	EVServer* getServer();
	void setNewDataNotProcessed();
	void setNewDataProcessed();
	bool newDataProcessed();
	void noMoreDataNecessary();
	void moreDataNecessary();
	bool needMoreData();
	EVConnectedStreamSocket * getEVConnSock(int fd);
	void setEVConnSock(EVConnectedStreamSocket * cs);
	ev_queue_type getUpstreamEventQ();
	void setUpstreamEventQ(ev_queue_type);
	void eraseEVConnSock(int fd);
	void setClientAddress(Net::SocketAddress addr);
	void setServerAddress(Net::SocketAddress addr);
	Net::SocketAddress& clientAddress();
	Net::SocketAddress& serverAddress();
	std::map<int,int>& getFileEvtSubscriptions();

private:
	EVServer*					_server;
	int							_no_new_data;
	int							_need_more_data;
	CSColMapType				_cssMap;
	ev_queue_type				_upstream_io_event_queue;
	Net::SocketAddress			_clientAddress;
	Net::SocketAddress			_serverAddress;
	std::map<int,int>			_file_evt_subscriptions;
};

inline std::map<int,int>& EVProcessingState::getFileEvtSubscriptions()
{
	return _file_evt_subscriptions;
}

inline EVProcessingState::EVProcessingState(EVServer * server):_server(server),
												_no_new_data(0), _need_more_data(0), _upstream_io_event_queue(0) { }
inline EVProcessingState::~EVProcessingState()
{
    for ( CSColMapType::iterator it = _cssMap.begin(); it != _cssMap.end(); ++it ) {
        delete it->second;
    }
    _cssMap.clear();
}
inline EVServer* EVProcessingState::getServer() { return _server; }
inline void EVProcessingState::setNewDataNotProcessed() { _no_new_data = 1; }
inline void EVProcessingState::setNewDataProcessed() { _no_new_data = 0; }
inline bool EVProcessingState::newDataProcessed() { return (_no_new_data == 0); }
inline void EVProcessingState::noMoreDataNecessary() { _need_more_data = 0; }
inline void EVProcessingState::moreDataNecessary() { _need_more_data = 1; }
inline bool EVProcessingState::needMoreData() { return (_need_more_data != 0); }
inline EVConnectedStreamSocket * EVProcessingState::getEVConnSock(int fd)
{
	EVConnectedStreamSocket * cn = _cssMap[fd];
	return (cn);
}
inline void EVProcessingState::setEVConnSock(EVConnectedStreamSocket * cs)
{
	_cssMap[cs->getSockfd()] = cs;
}
inline void EVProcessingState::eraseEVConnSock(int fd)
{
	EVConnectedStreamSocket * cn = _cssMap[fd];
	if (cn) {
		_cssMap.erase(fd);
		delete cn;
	}
	return;
}
inline ev_queue_type EVProcessingState::getUpstreamEventQ() { return _upstream_io_event_queue; }
inline void EVProcessingState::setUpstreamEventQ(ev_queue_type  q) { _upstream_io_event_queue = q; }

inline void EVProcessingState::setClientAddress(Net::SocketAddress addr)
{
	_clientAddress = addr;
}

inline void EVProcessingState::setServerAddress(Net::SocketAddress addr)
{
	_serverAddress = addr;
}

inline Net::SocketAddress& EVProcessingState::clientAddress()
{
	return _clientAddress;
}

inline Net::SocketAddress& EVProcessingState::serverAddress()
{
	return _serverAddress;
}

}
} // End namespace Poco::evnet

#endif
