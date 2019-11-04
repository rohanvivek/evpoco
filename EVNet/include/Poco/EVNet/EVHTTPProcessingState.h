//
// EVHTTPProcessingState.h
//
// Library: EVHTTPProcessingState
// Package: EVNet
// Module:  EVHTTPProcessingState
//
// Basic definitions for the Poco EVNet library.
// This file must be the first file included by every other EVNet
// header file.
//
// Copyright (c) 2005-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include "Poco/Net/Net.h"
#include "Poco/EVNet/EVNet.h"
#include "Poco/EVNet/EVHTTPServerSession.h"
#include "Poco/EVNet/EVProcessingState.h"
#include "Poco/EVNet/EVHTTPServerRequestImpl.h"
#include "Poco/EVNet/EVHTTPServerResponseImpl.h"
#include "Poco/EVNet/EVHTTPRequestHandler.h"
#include "Poco/EVNet/EVServer.h"
#include <string>

#include <chunked_memory_stream.h>
#include <http_parser.h>

#ifndef EVNet_EVHTTPProcessingState_INCLUDED
#define EVNet_EVHTTPProcessingState_INCLUDED

namespace Poco {
namespace EVNet {

class Net_API EVHTTPProcessingState : public EVProcessingState
	// This class is used as a marker to hold state data of processing of a request in a connection.
	// In case of event driven model of processing, the processing of a request may have to be
	// suspended mulptiple times, while data is being fetched from sources (e.g. client)
	//
	// The processing of the request is coded in such a way, that all the intermediate data is held within
	// a derivation of this base class and the state is destroyed at the end of processing of the request.
{
public:

	EVHTTPProcessingState(EVServer *);
	virtual ~EVHTTPProcessingState();
	void setRequest(EVHTTPServerRequestImpl * req);
	EVHTTPServerRequestImpl * getRequest();
	void setResponse(EVHTTPServerResponseImpl * resp);
	EVHTTPServerResponseImpl * getResponse();
	void setSession(EVHTTPServerSession *);
	EVHTTPServerSession * getSession();
	virtual int getState();
	void setState(int state);
	int continueRead();
		/// Continues reading of the request status line and header
		/// In case of async processing, a socket might be out of data
		/// in which case the partially read data is held in state and 
		/// reading is continued, when data again becomes available on the
		/// socket
	
	enum Limits
	{
		MAX_METHOD_LENGTH  = 32,
		MAX_URI_LENGTH     = 16384,
		MAX_VERSION_LENGTH = 8
	};

	enum MHLimits
		/// Limits for basic sanity checks when reading a header
	{
		MAX_NAME_LENGTH  = 256,
		MAX_VALUE_LENGTH = 8192,
		DFL_FIELD_LIMIT  = 100
	};

	void appendToUri(const char * , size_t);
	void appendToName(const char * , size_t, int);
	void appendToValue(const char * , size_t, int);
	void setMethod(const char * );
	void setVersion(const char * );
	void clearName();
	void clearValue();
	void messageBegin();
	void headerComplete();
	void messageComplete();
	void setReqMemStream(chunked_memory_stream *);
	void setResMemStream(chunked_memory_stream *);
	chunked_memory_stream* getReqMemStream();
	chunked_memory_stream* getResMemStream();
	bool trEncodingPresent();
	void setTrEncodingPresent();
	EVHTTPRequestHandler * getRequestHandler();
	void setRequestHandler(EVHTTPRequestHandler *);

private:
	void setReqProperties();

	int							_state;
	int							_header_field_in_progress;
	int							_header_value_in_progress;
	EVHTTPServerRequestImpl*	_request;
	EVHTTPServerResponseImpl*	_response;
	EVHTTPServerSession*		_session;
	EVHTTPRequestHandler*		_pHandler;
	std::string					_name;
	std::string					_value;
	std::string					_method;
	std::string					_uri;
	chunked_memory_stream*		_req_memory_stream;
	chunked_memory_stream*		_res_memory_stream;
	http_parser*				_parser;
	int							_tr_encoding_present;
	void*						_prev_node_ptr;
};

}
} // End namespace Poco::EVNet





#endif
