//
// EVTCPServiceRequest.h
//
// Library: EVNet
// Package: EVTCPServer
// Module:  EVTCPServer
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//

#include "Poco/Net/Net.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/NotificationQueue.h"

using Poco::Net::StreamSocket;

#ifndef POCO_EVNET_EVTCPSERVICEREQUEST_INCLUDED
#define POCO_EVNET_EVTCPSERVICEREQUEST_INCLUDED

namespace Poco{ namespace EVNet {
class EVTCPServiceRequest: public Notification
{
public:
	typedef enum {
		CONNECTION_REQUEST
		,SENDDATA_REQUEST
		,RECVDATAIVE_REQUEST
		,CLEANUP_REQUEST
	} what;
	EVTCPServiceRequest(long sr_num, what event, poco_socket_t acc_fd, Net::StreamSocket& ss);
	EVTCPServiceRequest(long sr_num, int cb_event_num, what event, poco_socket_t acc_fd, Net::StreamSocket& ss);
	EVTCPServiceRequest(long sr_num, int cb_event_num, what event, poco_socket_t acc_fd, Net::StreamSocket& ss, Net::SocketAddress& addr);

	~EVTCPServiceRequest();

	poco_socket_t sockfd();

	poco_socket_t accSockfd();

	what getEvent();

	StreamSocket& getStreamSocket();

	Net::SocketAddress& getAddr();

	int getCBEVIDNum();

	void setSRNum(long sr_num);

	long getSRNum();

private:
	long					_sr_num;
	int						_cb_evid_num; // Unique Service request number, for identificaton
	what					_event; // One of connect, send data or recieve data
	poco_socket_t			_acc_fd; // fd of the accepted(listen) socket
	Net::StreamSocket		_ss; // Connected StreamSocket
	Net::SocketAddress		_addr; // Optional address needed only in the connect request
};

inline EVTCPServiceRequest::what EVTCPServiceRequest::getEvent()
{
	return _event;
}

inline void EVTCPServiceRequest::setSRNum(long sr_num)
{
	_sr_num = sr_num;
}

inline long EVTCPServiceRequest::getSRNum()
{
	return _sr_num;
}

} } // namespace EVNet and Poco end.


#endif
