//
// HTTPClient.h
//
// $Id: //poco/1.4/Net/include/Poco/Net/HTTPClient.h#8 $
//
// Library: Net
// Package: HTTPClient
// Module:  HTTPClient
//
// Definition of the HTTPClient class.
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef Net_HTTPClient_INCLUDED
#define Net_HTTPClient_INCLUDED


#include "Poco/Net/Net.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPEventArgs.h"
#include "Poco/Net/HTTPSessionFactory.h"
#include "Poco/Net/HTTPSessionInstantiator.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/URI.h"
#include "Poco/Activity.h"
#include "Poco/SharedPtr.h"
#include "Poco/Mutex.h"
#include <deque>
#include <map>


namespace Poco {


class Thread;


namespace Net {


class Net_API HTTPClient
	/// This class wraps the client-side of a HTTP session,
	/// providing means for asynchronous communication with web
	/// server; send is a non-blocking function call, while response
	/// is received by registering to response and error notifications.
	/// 
	/// To send a HTTP request to a HTTP server, first instantiate
	/// a HTTPClient object and specify the server's host name
	/// and port number.
	///
	/// Once client is conected, sendRequest() or one of the
	/// send[HTTP METHOD]() functions can be called.
	///
	/// To receive response notifications, register a callback with
	/// the httpResponse event.
	///
	/// To receive error notifications, register a callback with the
	/// httpError event.
	///
	/// See RFC 2616 <http://www.faqs.org/rfcs/rfc2616.html> for more
	/// information about the HTTP protocol.
	///
	/// Proxies and proxy authorization (only HTTP Basic Authorization)
	/// is supported. Use setProxy() and setProxyCredentials() to
	/// set up a session through a proxy.
{
public:
	// TODO: make this SharedPtr and either (a) disable owning in 
	// HTTPSessionFactory or (b) convert the HTTPSessionFactory interface 
	// and internals to SharedPtr as well
	typedef HTTPSessionInstantiator* InstantiatorPtr;

	mutable Poco::BasicEvent<HTTPEventArgs> httpResponse;
	mutable Poco::BasicEvent<HTTPEventArgs> httpException;
	mutable Poco::BasicEvent<HTTPEventArgs> httpError;

	explicit HTTPClient(const URI& uri,
		InstantiatorPtr pInstantiator = new HTTPSessionInstantiator,
		bool redirect = true);
		/// Creates a HTTPClient using the given URI.

	explicit HTTPClient(const SocketAddress& address,
		InstantiatorPtr pInstantiator = new HTTPSessionInstantiator,
		bool redirect = true);
		/// Creates a HTTPClient using the given address.

	HTTPClient(const std::string& host,
		Poco::UInt16 port = HTTPSession::HTTP_PORT,
		InstantiatorPtr pInstantiator = new HTTPSessionInstantiator,
		bool redirect = true);
		/// Creates a HTTPClient using the given host and port.

	HTTPClient(const std::string& host,
		Poco::UInt16 port,
		const HTTPClientSession::ProxyConfig& proxyConfig,
		InstantiatorPtr pInstantiator = new HTTPSessionInstantiator,
		bool redirect = true);
		/// Creates a HTTPClient using the given host, port and proxy configuration.

	~HTTPClient();
		/// Destroys the HTTPClient and closes
		/// the underlying socket.

	void setHost(const std::string& host);
		/// Sets the host name of the target HTTP server.
		
	const std::string& getHost() const;
		/// Returns the host name of the target HTTP server.
		
	void setPort(Poco::UInt16 port);
		/// Sets the port number of the target HTTP server.
	
	Poco::UInt16 getPort() const;
		/// Returns the port number of the target HTTP server.

	void setProxy(const std::string& host, Poco::UInt16 port = HTTPSession::HTTP_PORT);
		/// Sets the proxy host name and port number.
		
	void setProxyHost(const std::string& host);
		/// Sets the host name of the proxy server.
		
	void setProxyPort(Poco::UInt16 port);
		/// Sets the port number of the proxy server.
		
	const std::string& getProxyHost() const;
		/// Returns the proxy host name.
		
	Poco::UInt16 getProxyPort() const;
		/// Returns the proxy port number.
		
	void setProxyCredentials(const std::string& username, const std::string& password);
		/// Sets the username and password for proxy authentication.
		/// Only Basic authentication is supported.
		
	void setProxyUsername(const std::string& username);
		/// Sets the username for proxy authentication.
		/// Only Basic authentication is supported.

	const std::string& getProxyUsername() const;
		/// Returns the username for proxy authentication.
		
	void setProxyPassword(const std::string& password);
		/// Sets the password for proxy authentication.	
		/// Only Basic authentication is supported.

	const std::string& getProxyPassword() const;
		/// Returns the password for proxy authentication.

	void setProxyConfig(const HTTPClientSession::ProxyConfig& config);
		/// Sets the proxy configuration.

	const HTTPClientSession::ProxyConfig& getProxyConfig() const;
		/// Returns the proxy configuration.

	void setKeepAlive(bool keepAlive);
		/// Sets the keep-alive flag.
		///
		/// If the keep-alive flag is enabled, persistent
		/// HTTP/1.1 connections are supported.

	bool getKeepAlive() const;
		/// Returns the value of the keep-alive flag.

	void setTimeout(const Poco::Timespan& timeout);
		/// Sets the timeout.

	Poco::Timespan getTimeout() const;
		/// Returns the timeout.

	void setKeepAliveTimeout(const Poco::Timespan& timeout);
		/// Sets the connection timeout for HTTP connections.
		
	const Poco::Timespan& getKeepAliveTimeout() const;
		/// Returns the connection timeout for HTTP connections.
	
	void sendRequest(SharedPtr<HTTPRequest> request,
		const std::string& body = "");
		/// Sends the header for the given HTTP request to
		/// the server.

	void sendRequest(const std::string& method,
		const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends the header for the given HTTP request to
		/// the server.
		///
		/// The HTTPClientSession will set the request's
		/// Host and Keep-Alive headers accordingly.
	
	void sendGet(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends GET request to the server.
	
	void sendHead(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends HEAD request to the server.
	
	void sendPost(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends POST request to the server.
	
	void sendPut(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends PUT request to the server.
	
	void sendDelete(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends DELETE request to the server.
	
	void sendConnect(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends CONNECT request to the server.
	
	void sendOptions(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends OPTIONS request to the server.
	
	void sendTrace(const std::string& uri,
		const std::string& body = "",
		const std::string& version = HTTPMessage::HTTP_1_1);
		/// Sends TRACE request to the server.

	void reset();
		/// Resets the session and closes the socket.
		///
		/// The next request will initiate a new connection,
		/// even if persistent connections are enabled.
		///
		/// This should be called whenever something went
		/// wrong while sending a request.
		
	bool secure() const;
		/// Return true iff the session uses SSL or TLS,
		/// or false otherwise.
		
	bool bypassProxy() const;
		/// Returns true if the proxy should be bypassed
		/// for the current host.

	int requests() const;
		/// Returns the number of currently pending requests.

protected:
	void runActivity();
	void redirect(HTTPRequest& req,
		const std::string& newURI, HTTPResponse::HTTPStatus status);

	HTTPSessionFactory _factory;
	InstantiatorPtr    _pInstantiator;
	InstantiatorPtr    _pSecureInstantiator;
	HTTPClientSession* _pSession;
	bool               _redirect;
	bool               _redirectHTTPS;

private:
	typedef std::deque<SharedPtr<HTTPRequest> > RequestQueue;
	typedef std::map<SharedPtr<HTTPRequest>, std::string> RequestBodyMap;

	HTTPClient();

	void addRequest(SharedPtr<HTTPRequest> request, const std::string& body = "");
	void wakeUp();
	void clearQueue();

	RequestQueue             _requestQueue;
	RequestBodyMap           _requestBodyMap;
	Activity<HTTPClient>     _activity;
	Mutex                    _mutex;
	Mutex                    _exchangeMutex;
	Thread*                  _pThread;
	int                      _requestsInProcess;
};


//
// inlines
//

inline void HTTPClient::sendRequest(SharedPtr<HTTPRequest> request,
	const std::string& body)
{
	addRequest(request, body);
}


inline void HTTPClient::sendGet(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_GET, uri, body, version);
}


inline void HTTPClient::sendHead(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_HEAD, uri, body, version);
}


inline void HTTPClient::sendPost(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_POST, uri, body, version);
}


inline void HTTPClient::sendPut(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_PUT, uri, body, version);
}


inline void HTTPClient::sendDelete(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_DELETE, uri, body, version);
}


inline void HTTPClient::sendConnect(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_CONNECT, uri, body, version);
}

inline void HTTPClient::sendOptions(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_OPTIONS, uri, body, version);
}


inline void HTTPClient::sendTrace(const std::string& uri,
	const std::string& body,
	const std::string& version)
{
	sendRequest(HTTPRequest::HTTP_TRACE, uri, body, version);
}


inline const std::string& HTTPClient::getHost() const
{
	return _pSession->getHost();
}


inline Poco::UInt16 HTTPClient::getPort() const
{
	return _pSession->getPort();
}


inline const std::string& HTTPClient::getProxyHost() const
{
	return _pSession->getProxyHost();
}


inline Poco::UInt16 HTTPClient::getProxyPort() const
{
	return _pSession->getProxyPort();
}


inline const std::string& HTTPClient::getProxyUsername() const
{
	return _pSession->getProxyUsername();
}


inline const std::string& HTTPClient::getProxyPassword() const
{
	return _pSession->getProxyPassword();
}


inline const HTTPClientSession::ProxyConfig& HTTPClient::getProxyConfig() const
{
	return _pSession->getProxyConfig();
}


inline bool HTTPClient::getKeepAlive() const
{
	return _pSession->getKeepAlive();
}


inline Poco::Timespan HTTPClient::getTimeout() const
{
	return _pSession->getTimeout();
}


inline const Poco::Timespan& HTTPClient::getKeepAliveTimeout() const
{
	return _pSession->getKeepAliveTimeout();
}


inline bool HTTPClient::secure() const
{
	return _pSession->secure();
}


inline bool HTTPClient::bypassProxy() const
{
	return _pSession->bypassProxy();
}


inline int HTTPClient::requests() const
{
	return _requestsInProcess + static_cast<int>(_requestQueue.size());
}


} } // namespace Poco::Net


#endif // Net_HTTPClient_INCLUDED