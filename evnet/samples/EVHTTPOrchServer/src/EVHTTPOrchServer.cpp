//
// EVHTTPOrchServer.cpp
//
// This sample demonstrates the HTTPServer and HTMLForm classes.
//
// Copyright (c) 2018-2019, Tekenlight Solutions Pvt Ltd.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//

#include <iostream>

#include "Poco/Net/StreamSocket.h"
#include "Poco/evnet/EVHTTPServer.h"
#include "Poco/evnet/EVHTTPRequestHandler.h"
#include "Poco/evnet/EVHTTPRequestHandlerFactory.h"
#include "Poco/evnet/EVHTTPClientSession.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/CountingStream.h"
#include "Poco/NullStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"


#include "Poco/evnet/EVHTTPRequest.h"
#include "Poco/evnet/EVHTTPResponse.h"

using Poco::Net::ServerSocket;
using Poco::evnet::EVHTTPRequestHandler;
using Poco::evnet::EVHTTPRequestHandlerFactory;
using Poco::evnet::EVHTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::MessageHeader;
using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::Net::HTTPRequest;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::CountingInputStream;
using Poco::NullOutputStream;
using Poco::StreamCopier;


class EVMyPartHandler: public Poco::Net::PartHandler
{
public:
	EVMyPartHandler():
		_length(0)
	{
	}
	
	void handlePart(const MessageHeader& header, std::istream& stream)
	{
		try {
		_type = header.get("Content-Type", "(unspecified)");
		if (header.has("Content-Disposition"))
		{
			std::string disp;
			NameValueCollection params;
			MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
			_name = params.get("name", "(unnamed)");
			_fileName = params.get("filename", "(unnamed)");
		}
		
		CountingInputStream istr(stream);
		NullOutputStream ostr;
		StreamCopier::copyStream(istr, ostr);
		_length = istr.chars();
		} catch (std::exception& ex) {
			DEBUGPOINT("EXCEPTION HERE %s\n", ex.what());
			abort();
		}
	}

	int length() const
	{
		return _length;
	}

	const std::string& name() const
	{
		return _name;
	}

	const std::string& fileName() const
	{
		return _fileName;
	}

	const std::string& contentType() const
	{
		return _type;
	}

private:
	int _length;
	std::string _type;
	std::string _name;
	std::string _fileName;
};


class EVFormRequestHandler: public EVHTTPRequestHandler
	/// Return a HTML document with the current date and time.
{
private:
	static const int PART_ONE = 1;
	static const int PART_TWO = 2;
	static const int PART_THREE = 3;
	static const int FINAL = 4;

	EVMyPartHandler partHandler;
	HTMLForm *form1 = NULL;
	Poco::evnet::EVHTTPClientSession session;
	Poco::evnet::EVHTTPResponse uresponse;

	void send_error_response(int line_no)
	{
		HTTPServerRequest& request = (getRequest());
		HTTPServerResponse& response = (getResponse());


		HTMLForm& form = *form1;

		std::ostream& ostr = getResponse().getOStream();

		ostr <<
			"<html>\n"
			"<head>\n"
			"<title>EVHTTPOrchServer Processing ERROR</title>\n"
			"</head>\n"
			"<body>\n"
			"<h1>EVHTTP Form Server Sample</h1>\n";

		ostr << line_no << ":" << "COULD NOT CONTACT DATA PROVIDER\n";

		ostr << "</body>\n";
		ostr << "</html>\n";
		ostr.flush();
	}

	void init()
	{
		Application::instance().logger().information("Request from " + getRequest().clientAddress().toString());
		try {
			form1 = new HTMLForm((getRequest()), getRequest().stream(), partHandler);
		} catch (std::exception& ex) {
			DEBUGPOINT("CHA %s\n",ex.what());
			throw(ex);
		}

		getResponse().setChunkedTransferEncoding(true);
		getResponse().setContentType("text/html");

		getResponse().send();
	}

	/*
	 * These two below are imppmenetation of the request handler
	 * using the event handling mechanism.
	class part_three : public EventHandler {
		EVFormRequestHandler* handler;
		public:
		part_three(EVFormRequestHandler*h):EventHandler() { handler=h; }
		int operator ()()
		{
			Poco::evnet::EVUpstreamEventNotification &usN = handler->getUNotification();
			DEBUGPOINT("PART_THREE from %d\n", handler->session.getAccfd());
			HTTPServerRequest& request = handler->getRequest();
			HTTPServerResponse& response = handler->getResponse();

			if (usN.getRet() < 0) {
				handler->send_error_response(__LINE__);
				return -1;
			}

			char str[1024] = {0};
			std::istream * istr = handler->uresponse.getStream();
			istr->get(str, 512);
			//DEBUGPOINT("RECEIVED DATA = \n%s\n from %d\n", str, handler->session.getAccfd());


			HTMLForm& form = *(handler->form1);

			std::ostream& ostr = handler->getResponse().getOStream();

			ostr <<
				"<html>\n"
				"<head>\n"
				"<title>POCO Form Server Sample</title>\n"
				"</head>\n"
				"<body>\n"
				"<h1>POCO Form Server Sample</h1>\n"
				"<h2>GET Form</h2>\n"
				"<form method=\"GET\" action=\"/form\">\n"
				"<input type=\"text\" name=\"text\" size=\"31\">\n"
				"<input type=\"submit\" value=\"GET\">\n"
				"</form>\n"
				"<h2>POST Form</h2>\n"
				"<form method=\"POST\" action=\"/form\">\n"
				"<input type=\"text\" name=\"text\" size=\"31\">\n"
				"<input type=\"submit\" value=\"POST\">\n"
				"</form>\n"
				"<h2>File Upload</h2>\n"
				"<form method=\"POST\" action=\"/form\" enctype=\"multipart/form-data\">\n"
				"<input type=\"file\" name=\"file\" size=\"31\"> \n"
				"<input type=\"submit\" value=\"Upload\">\n"
				"</form>\n";
				
			ostr << "<h2>Request</h2><p>\n";
			ostr << "Method: " << request.getMethod() << "<br>\n";
			ostr << "URI: " << request.getURI() << "<br>\n";
			NameValueCollection::ConstIterator it = request.begin();
			NameValueCollection::ConstIterator end = request.end();
			for (; it != end; ++it)
			{
				ostr << it->first << ": " << it->second << "<br>\n";
			}
			ostr << "<h3>Received data from upstream server</h3>\n";
			ostr << str;
			ostr << "\n";
			ostr << "</p>";

			if (!form.empty())
			{
				ostr << "<h2>Form</h2><p>\n";
				it = form.begin();
				end = form.end();
				for (; it != end; ++it)
				{
					ostr << it->first << ": " << it->second << "<br>\n";
				}
				ostr << "</p>";
			}

			if (!handler->partHandler.name().empty())
			{
				ostr << "<h2>Upload</h2><p>\n";
				ostr << "Name: " << handler->partHandler.name() << "<br>\n";
				ostr << "File Name: " << handler->partHandler.fileName() << "<br>\n";
				ostr << "Type: " << handler->partHandler.contentType() << "<br>\n";
				ostr << "Size: " << handler->partHandler.length() << "<br>\n";
				ostr << "</p>";
			}
			ostr << "</body>\n";
			ostr << "</html>\n";
			ostr.flush();

			return PROCESSING_COMPLETE;
		}
	}; part_three three{this};

	class part_two : public EventHandler {
		EVFormRequestHandler* handler;
		public:
		part_two(EVFormRequestHandler*h):EventHandler() { handler=h; }
		int operator ()()
		{
			DEBUGPOINT("PART_TWO, from %d\n", handler->session.getAccfd());
			Poco::evnet::EVUpstreamEventNotification &usN = handler->getUNotification();
			DEBUGPOINT("Socket = %d Refcount = %d state = %d from %d\n",
					usN.sockfd(), handler->session.getSS().impl()->referenceCount(),
					handler->session.getState(), handler->session.getAccfd());
			DEBUGPOINT("Service Request Number = %ld from %d\n", usN.getSRNum(), handler->session.getAccfd());
			if (usN.getRet() < 0) {
				handler->send_error_response(__LINE__);
				return -1;
			}
			Poco::evnet::EVHTTPRequest request(HTTPRequest::HTTP_POST, "http://localhost:9980/echo");
			request.setHost("localhost:9980");
			std::string body("this is a random request body");
			request.setContentLength((int) body.length());
			handler->sendHTTPHeader(handler->session, request);
			*(request.getRequestStream()) << body;
			handler->sendHTTPRequestData(handler->session, request);

			handler->waitForHTTPResponse(handler->three, (handler->session), handler->uresponse);
			return PROCESSING;
		}
	} ; part_two two{this};
	*/

	int part_two()
	{
		DEBUGPOINT("PART_TWO, from %d\n", session.getAccfd());
		Poco::evnet::EVUpstreamEventNotification &usN = getUNotification();
		DEBUGPOINT("Socket = %d Refcount = %d state = %d from %d\n",
				usN.sockfd(), session.getSS().impl()->referenceCount(),
				session.getState(), session.getAccfd());
		DEBUGPOINT("Service Request Number = %ld from %d\n", usN.getSRNum(), session.getAccfd());
		if (usN.getRet() < 0) {
			//usN.debug(__FILE__, __LINE__);
			send_error_response(__LINE__);
			return -1;
		}
		Poco::evnet::EVHTTPRequest request(HTTPRequest::HTTP_POST, "http://localhost:9980/echo");
		request.setHost("localhost:9980");
		request.setExpectContinue(true);
		std::string body("this is a random request body");
		request.setContentLength((int) body.length());
		sendHTTPHeader(session, request);
		*(request.getRequestStream()) << body;
		sendHTTPRequestData(session, request);

		waitForHTTPResponse(std::bind(&EVFormRequestHandler::part_three, this), (session), uresponse);

		return PROCESSING;
	}

	int part_three()
	{
		Poco::evnet::EVUpstreamEventNotification &usN = getUNotification();
		DEBUGPOINT("PART_THREE from %d\n", session.getAccfd());
		HTTPServerRequest& request = getRequest();
		HTTPServerResponse& response = getResponse();

		if (usN.getRet() < 0) {
			usN.debug(__FILE__, __LINE__);
			send_error_response(__LINE__);
			return -1;
		}

		char str[1024] = {0};
		std::istream * istr = uresponse.getStream();
		istr->get(str, 512);
		//DEBUGPOINT("RECEIVED DATA = \n%s\n from %d\n", str, session.getAccfd());


		HTMLForm& form = *(form1);

		std::ostream& ostr = getResponse().getOStream();

		ostr <<
			"<html>\n"
			"<head>\n"
			"<title>POCO Form Server Sample</title>\n"
			"</head>\n"
			"<body>\n"
			"<h1>POCO Form Server Sample</h1>\n"
			"<h2>GET Form</h2>\n"
			"<form method=\"GET\" action=\"/form\">\n"
			"<input type=\"text\" name=\"text\" size=\"31\">\n"
			"<input type=\"submit\" value=\"GET\">\n"
			"</form>\n"
			"<h2>POST Form</h2>\n"
			"<form method=\"POST\" action=\"/form\">\n"
			"<input type=\"text\" name=\"text\" size=\"31\">\n"
			"<input type=\"submit\" value=\"POST\">\n"
			"</form>\n"
			"<h2>File Upload</h2>\n"
			"<form method=\"POST\" action=\"/form\" enctype=\"multipart/form-data\">\n"
			"<input type=\"file\" name=\"file\" size=\"31\"> \n"
			"<input type=\"submit\" value=\"Upload\">\n"
			"</form>\n";
			
		ostr << "<h2>Request</h2><p>\n";
		ostr << "Method: " << request.getMethod() << "<br>\n";
		ostr << "URI: " << request.getURI() << "<br>\n";
		NameValueCollection::ConstIterator it = request.begin();
		NameValueCollection::ConstIterator end = request.end();
		for (; it != end; ++it)
		{
			ostr << it->first << ": " << it->second << "<br>\n";
		}
		ostr << "<h3>Received data from upstream server</h3>\n";
		ostr << str;
		ostr << "\n";
		ostr << "</p>";

		if (!form.empty())
		{
			ostr << "<h2>Form</h2><p>\n";
			it = form.begin();
			end = form.end();
			for (; it != end; ++it)
			{
				ostr << it->first << ": " << it->second << "<br>\n";
			}
			ostr << "</p>";
		}

		if (!partHandler.name().empty())
		{
			ostr << "<h2>Upload</h2><p>\n";
			ostr << "Name: " << partHandler.name() << "<br>\n";
			ostr << "File Name: " << partHandler.fileName() << "<br>\n";
			ostr << "Type: " << partHandler.contentType() << "<br>\n";
			ostr << "Size: " << partHandler.length() << "<br>\n";
			ostr << "</p>";
		}
		ostr << "</body>\n";
		ostr << "</html>\n";
		ostr.flush();

		return PROCESSING_COMPLETE;
	}

	void cleanup()
	{
		if (form1) delete form1;
		//if (ostr_ptr) delete ostr_ptr;
	}

public:
	EVFormRequestHandler() 
	{
	}

	virtual ~EVFormRequestHandler() 
	{
		cleanup();
	}

	int handleRequest()
	{
		init();
		//This is the event handler (functor) mechanism
		//if (0 > makeNewHTTPConnection(two, &session)) 
		if (0 > makeNewHTTPConnection(std::bind(&EVFormRequestHandler::part_two, this), "localhost", 9980, session)) {
			send_error_response(__LINE__);
			return -1;
		}
		DEBUGPOINT("PART_ONE from %d\n", session.getAccfd());

		//sleep(1);
		return PROCESSING;
	}
};


class EVFormRequestHandlerFactory: public EVHTTPRequestHandlerFactory
{
public:
	EVFormRequestHandlerFactory()
	{
	}

	EVHTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		return new EVFormRequestHandler;
	}
};


class EVHTTPOrchServer: public Poco::Util::ServerApplication
	/// The main application class.
	///
	/// This class handles command-line arguments and
	/// configuration files.
	/// Start the EVHTTPOrchServer executable with the help
	/// option (/help on Windows, --help on Unix) for
	/// the available command line options.
	///
	/// To use the sample configuration file (EVHTTPOrchServer.properties),
	/// copy the file to the directory where the EVHTTPOrchServer executable
	/// resides. If you start the debug version of the EVHTTPOrchServer
	/// (EVHTTPOrchServerd[.exe]), you must also create a copy of the configuration
	/// file named EVHTTPOrchServerd.properties. In the configuration file, you
	/// can specify the port on which the server is listening (default
	/// 9980) and the format of the date/Form string sent back to the client.
	///
	/// To test the FormServer you can use any web browser (http://localhost:9980/).
{
public:
	EVHTTPOrchServer(): _helpRequested(false)
	{
	}
	
	~EVHTTPOrchServer()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}
		
	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);
		
		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A web server that shows how to work with HTML forms.");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			HTTPServerParams *p = new HTTPServerParams();
			unsigned short port = (unsigned short) config().getInt("EVHTTPOrchServer.port", 9981);

			p->setBlocking(config().getBool("EVHTTPOrchServer.blocking", false));
			
			// set-up a server socket
			ServerSocket svs(port);
			// set-up a HTTPServer instance
			EVHTTPServer srv(new EVFormRequestHandlerFactory, svs, p);
			// start the HTTPServer
			srv.start();
			// wait for CTRL-C or kill
			waitForTerminationRequest();
			// Stop the HTTPServer
			srv.stop();
		}
		return Application::EXIT_OK;
	}
	
private:
	bool _helpRequested;
};

int func(int argc, char ** argv)
{
	int ret = 0;
	EVHTTPOrchServer app;
	ret =  app.run(argc, argv);
	return ret;
}

int main(int argc, char** argv)
{
	int ret = 0;

	ret = func(argc,argv);

	return ret;
}
