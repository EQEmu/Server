/*  EQEMu:  Everquest Server Emulator
 * 
 * 	This code originated from `C++ Sockets Library` referenced below.
 * 	Taken and stripped/modified to remove dependancies on parts of
 * 	the library which we are not using, and to suit other needs.
 * 		2006 - EQEMu Development Team (http://eqemulator.net)
 * 
 * 
 */

/** \file HTTPSocket.h 	Class HTTPSocket definition.
 **	\date  2004-04-06
 **	\author grymse@alhem.net
**/
/*
Copyright (C) 2004,2005  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef _HTTPSOCKET_H
#define _HTTPSOCKET_H

#include <map>
#include <string>
#include "../tcp_connection.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

/** \defgroup http HTTP Sockets */
/** HTTP request/response base class. 
	\ingroup http */
class HTTPSocket : public TCPConnection
{
	/** map to hold http header values. */
	typedef std::map<std::string,std::string> string_m;
public:
	HTTPSocket(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort);
	virtual ~HTTPSocket();

	void OnLine(const std::string& line);

	/** Callback executes when first line has been received.
		GetMethod, GetUrl/GetUri, and GetHttpVersion are valid when this callback is executed. */
	virtual void OnFirst() = 0;
	/** For each header line this callback is executed.
		\param key Http header name
		\param value Http header value */
	virtual void OnHeader(const std::string& key,const std::string& value) = 0;
	/** Callback fires when all http headers have been received. */
	virtual void OnHeaderComplete() = 0;
	/** Chunk of http body data recevied. */
	virtual void OnData(const char *,size_t) = 0;

	const std::string& GetMethod();
	void SetMethod(const std::string& x);
	const std::string& GetUrl();
	void SetUrl(const std::string& x);
	const std::string& GetUri();
	void SetUri(const std::string& x);
	const std::string& GetQueryString();
	const std::string& GetHttpVersion();
	const std::string& GetStatus();
	const std::string& GetStatusText();
	bool IsRequest();
	bool IsResponse();

	void SetHttpVersion(const std::string& x);
	void SetStatus(const std::string& x);
	void SetStatus(const std::string& num, const std::string& text);
	void SetStatusText(const std::string& x);
	void AddResponseHeader(const std::string& x,const std::string& y);
	void AddResponseHeader(const std::string& x,const char *format, ...);
	void SendResponse();
	void SendResponse(const std::string& status_num, const std::string& status_text);
	void SendRequest();

	/** Implement this to return your own User-agent string. */
	virtual std::string MyUseragent();

protected:
	/** Reset state of socket to sucessfully implement keep-alive. */
	virtual void Reset();
	
	//stubs for crap which used to be in our parent class (TcpSocket)
	bool SendString(const char *str);
	bool SendBuf(const char *dat, unsigned int len);

	virtual bool ProcessReceivedData(char* errbuf = 0);
	
private:
//	HTTPSocket& operator=(const HTTPSocket& ) { return *this; }
	bool m_first;
	bool m_header;
	std::string m_line;
	std::string m_method;
	std::string m_url;
	std::string m_uri;
	std::string m_query_string;
	std::string m_http_version;
	std::string m_status;
	std::string m_status_text;
	bool m_request;
	bool m_response;
	string_m m_response_header;
};




#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _HTTPSOCKET_H
