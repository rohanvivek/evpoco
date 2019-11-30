//
// EVHTTPRequestHandler.h
//
// Library: EVNet
// Package: EVHTTPServer
// Module:  EVHTTPRequestHandler
//
// Definition of the EVHTTPRequestHandler class.
//
// Copyright (c) 2019-2020, Tekenlight Solutions Pvt Ltd
// and Contributors.
//
//


#ifndef Net_EVHTTPRequestHandler_INCLUDED
#define Net_EVHTTPRequestHandler_INCLUDED

#include <ostream>
#include <map>

#include <functional>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "Poco/Net/Net.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/EVNet/EVHTTPRequest.h"
#include "Poco/EVNet/EVHTTPClientSession.h"
#include "Poco/EVNet/EVNet.h"
#include "Poco/EVNet/EVUpstreamEventNotification.h"
#include "Poco/EVNet/EVServer.h"


namespace Poco {
namespace EVNet {

class EVHTTPRequestHandler;

class Net_API EVHTTPRequestHandler
	/// The abstract base class for EVHTTPRequestHandlers 
	/// created by EVHTTPServer.
	///
	/// Derived classes must override the handleRequest() method.
	/// Furthermore, a EVHTTPRequestHandlerFactory must be provided.
	///
	/// The handleRequest() method must perform the complete handling
	/// of the HTTP request connection. As soon as the handleRequest() 
	/// method returns, the request handler object is destroyed.
	///
	/// A new EVHTTPRequestHandler object will be created for
	/// each new HTTP request that is received by the HTTPServer.
{
public:
	typedef std::function<int ()> TCallback;

	class EventHandler {
		public:
		EventHandler() { }
		virtual int operator ()() =0;
	};
	struct SRData {
		SRData(): cb_evid_num(0), session_ptr(0), response(0), cb_handler(0), cb(0), addr_info_ptr_ptr(0),
				  domain_name(0), serv_name(0), port_num(0) {}
		~SRData() {}
		Net::SocketAddress		addr;
		EVHTTPClientSession*	session_ptr;
		int						cb_evid_num;
		EVHTTPResponse*			response;
		EventHandler*			cb_handler;
		TCallback				cb;
		const char*				domain_name;
		const char*				serv_name;
		unsigned short			port_num;
		struct addrinfo**       addr_info_ptr_ptr;
	} ;
	typedef std::map<long,SRData *> SRColMapType;
	static const int INITIAL = 0;

	/* Return values of handleRequest method. */
	static const int INVALID_STATE = -1;
	static const int PROCESSING_ERROR = -1000;
	static const int PROCESSING = 0;
	static const int PROCESSING_COMPLETE = 1000;

	static const int HTTPRH_INVALID_CB_NUM = -1;

	static const int HTTPRH_DNSR_HOST_RESOLUTION_DONE = -100;

	static const int HTTPRH_HTTPCONN_HOSTRESOLVED = -200;
	static const int HTTPRH_HTTPCONN_PROXYSOCK_READY = -210;
	static const int HTTPRH_HTTPCONN_PROXY_RESPONSE = -220;
	static const int HTTPRH_HTTPCONN_CONNECTION_ESTABLISHED = -230;

	static const int HTTPRH_HTTPRESP_MSG_FROM_HOST = -300;


	static const int HTTPRH_CALL_CB_HANDLER = -10000;

	EVHTTPRequestHandler();
		/// Creates the EVHTTPRequestHandler.

	virtual ~EVHTTPRequestHandler();
		/// Destroys the EVHTTPRequestHandler.

	virtual int handleRequest() = 0;
		/// Must be overridden by subclasses.
		///
		/// Handles the given request.

	int handleRequestSurrogate();
	int handleRequestSurrogateInitial();

	int getState();
	void setState(int);
	EVUpstreamEventNotification & getUNotification();
	void setUNotification(EVUpstreamEventNotification *);
	int getEvent();
	EVServer& getServer();
	void setServer(EVServer * server);
	poco_socket_t getAccSockfd();
	void setAccSockfd(poco_socket_t fd);
	Net::HTTPServerRequest& getRequest();
	void setRequest(Net::HTTPServerRequest* req);
	Net::HTTPServerResponse& getResponse();
	void setResponse(Net::HTTPServerResponse* res);

	/*
	long waitForHTTPResponse(int cb_evid_num, EVHTTPClientSession& sess, EVHTTPResponse &req);
	long waitForHTTPResponse(EventHandler& cb_handler, EVHTTPClientSession& sess, EVHTTPResponse& res);

	long makeNewSocketConnection(int cb_evid_num, Net::SocketAddress& addr, Net::StreamSocket& css);
	long makeNewSocketConnection(EventHandler& cb_handler, Net::SocketAddress& addr, Net::StreamSocket& css);

	long makeNewHTTPConnection(int cb_evid_num, EVHTTPClientSession& sess);
	long makeNewHTTPConnection(EventHandler& cb_handler, EVHTTPClientSession& sess);
	 *
	 */
	long resolveHost(TCallback cb, const char* domain_name, const char* serv_name, struct addrinfo ** addr_info_ptr_ptr);

	long makeNewHTTPConnection(TCallback cb, const char * domain_name, const char * serv_name, EVHTTPClientSession& sess);
	long makeNewHTTPConnection(TCallback cb, const char * domain_name, const unsigned short port_num, EVHTTPClientSession& sess);
	long makeNewHTTPConnection(TCallback cb, EVHTTPClientSession& sess);

	long makeNewSocketConnection(TCallback cb, Net::SocketAddress& addr, Net::StreamSocket& css);

	long sendHTTPHeader(EVHTTPClientSession &sess, EVHTTPRequest &req);
	long sendHTTPRequestData(EVHTTPClientSession &ses, EVHTTPRequest & req);

	long waitForHTTPResponse(TCallback cb, EVHTTPClientSession& sess, EVHTTPResponse& res);

	long closeHTTPSession(EVHTTPClientSession& sess);

private:
	EVHTTPRequestHandler(const EVHTTPRequestHandler&);
	EVHTTPRequestHandler& operator = (const EVHTTPRequestHandler&);
	bool bypassProxy(std::string host);
	const Net::HTTPClientSession::ProxyConfig& proxyConfig();

	int								_state;
	EVUpstreamEventNotification*	_usN;
	EVServer*						_server;
	poco_socket_t					_acc_fd;
	Net::HTTPServerRequest*			_req = NULL;
	Net::HTTPServerResponse*		_rsp = NULL;
	SRColMapType					_srColl;
};

inline EVUpstreamEventNotification & EVHTTPRequestHandler::getUNotification()
{
	return *_usN;
}

inline void EVHTTPRequestHandler::setUNotification(EVUpstreamEventNotification * usN)
{
	_usN = usN;
}

inline int EVHTTPRequestHandler::getEvent()
{
	int event = getState();
	if (!_usN || INITIAL == event) return INITIAL;

	return _usN->getCBEVIDNum();
}

inline EVServer& EVHTTPRequestHandler::getServer()
{
	return *_server;
}

inline void EVHTTPRequestHandler::setServer(EVServer * server)
{
	_server = server;
}

inline poco_socket_t EVHTTPRequestHandler::getAccSockfd()
{
	return _acc_fd;
}

inline void EVHTTPRequestHandler::setAccSockfd(poco_socket_t fd)
{
	_acc_fd = fd;
}

inline Net::HTTPServerRequest& EVHTTPRequestHandler::getRequest()
{
	return *_req;
}

inline void EVHTTPRequestHandler::setRequest(Net::HTTPServerRequest* req)
{
	_req = req;
}

inline Net::HTTPServerResponse& EVHTTPRequestHandler::getResponse()
{
	return *_rsp;
}

inline void EVHTTPRequestHandler::setResponse(Net::HTTPServerResponse* rsp)
{
	_rsp = rsp;
}

inline const Net::HTTPClientSession::ProxyConfig& EVHTTPRequestHandler::proxyConfig()
{
	return Net::HTTPClientSession::getGlobalProxyConfig();
}


} } // namespace Poco::EVNet


#endif // Net_EVHTTPRequestHandler_INCLUDED
