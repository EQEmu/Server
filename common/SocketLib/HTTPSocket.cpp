/*  EQEMu:  Everquest Server Emulator
 * 
 * 	This code originated from `C++ Sockets Library` referenced below.
 * 	Taken and stripped/modified to remove dependancies on parts of
 * 	the library which we are not using, and to suit other needs.
 * 		2006 - EQEMu Development Team (http://eqemulator.net)
 * 
 * 
 */
 
/** \file HTTPSocket.cpp
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
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include "../global_define.h"
#include <stdio.h>
#include <string>
#include <stdarg.h>
#include "Parse.h"
#include "HTTPSocket.h"
#include "../tcp_connection.h"
#include <cstdlib>
#include <cstring>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif




HTTPSocket::HTTPSocket(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort)
:TCPConnection(ID,in_socket,irIP,irPort)
,m_first(true)
,m_header(true)
,m_http_version("HTTP/1.0")
,m_request(false)
,m_response(false)
{
}


HTTPSocket::~HTTPSocket()
{
}

/*
 * eqemu stuff 
 */

bool HTTPSocket::ProcessReceivedData(char *errbuf)
{
	if (errbuf)
		errbuf[0] = 0;
	if (!recvbuf)
		return true;

	char *buff=(char *)recvbuf;
	unsigned long bufflen=recvbuf_used;

	while(1) {
		if (m_header) {
			char *ptr=(char *)memchr(buff,'\n',bufflen);
			if (!ptr)
				break;
			int length=(ptr-buff)+1;
			std::string line;
			line.append(buff,length-2);
			OnLine(line);

			buff+=length;
			bufflen-=length;
		} else {
			OnData(buff,bufflen);
			buff+=bufflen;
			bufflen=0;
			break;
		}
	}

	if (bufflen) {
		memmove(recvbuf,buff,bufflen);
		recvbuf_used=bufflen;
	} else {
		safe_delete_array(recvbuf);
	}
}

bool HTTPSocket::SendString(const char *str) {
	return(TCPConnection::Send((const uchar *) str, strlen(str)));
}

bool HTTPSocket::SendBuf(const char *dat, unsigned int len) {
	return(TCPConnection::Send((const uchar *) dat, len));
}

/*
 * /eqemu stuff 
 */

void HTTPSocket::OnLine(const std::string& line)
{
	if (m_first)
	{
		Parse pa(line);
		std::string str = pa.getword();
		if (str.substr(0,4) == "HTTP") // response
		{
			m_http_version = str;
			m_status = pa.getword();
			m_status_text = pa.getrest();
			m_response = true;
		}
		else // request
		{
			m_method = str;
			m_url = pa.getword();
			size_t spl = m_url.find("?");
			if (spl != std::string::npos)
			{
				m_uri = m_url.substr(0,spl);
				m_query_string = m_url.substr(spl + 1);
			}
			else
			{
				m_uri = m_url;
			}
			m_http_version = pa.getword();
			m_request = true;
		}
		m_first = false;
		OnFirst();
		return;
	}
	if (!line.size())
	{
//		SetLineProtocol(false);
		m_header = false;
		OnHeaderComplete();
		return;
	}
	Parse pa(line,":");
	std::string key = pa.getword();
	std::string value = pa.getrest();
	OnHeader(key,value);
	/* If remote end tells us to keep connection alive, and we're operating
	in http/1.1 mode (not http/1.0 mode), then we mark the socket to be
	retained. */
/*	if (!strcasecmp(key.c_str(), "connection") &&
	    !strcasecmp(value.c_str(), "keep-alive") )
	{
		SetRetain();
	}*/
}


void HTTPSocket::SendResponse()
{
	std::string msg;
	msg = m_http_version + " " + m_status + " " + m_status_text + "\r\n";
	for (string_m::iterator it = m_response_header.begin(); it != m_response_header.end(); it++)
	{
		std::string key = (*it).first;
		std::string val = (*it).second;
		msg += key + ": " + val + "\r\n";
	}
	msg += "\r\n";
	SendString( msg.c_str() );
}


void HTTPSocket::AddResponseHeader(const std::string& header, const char *format, ...)
{
	static char slask[5000];
	va_list ap;

	va_start(ap, format);
#ifdef _WIN32
	vsprintf(slask, format, ap);
#else
	vsnprintf(slask, 5000, format, ap);
#endif
	va_end(ap);

	m_response_header[header] = slask;
}


void HTTPSocket::SendRequest()
{
	std::string msg;
	msg = m_method + " " + m_url + " " + m_http_version + "\r\n";
	for (string_m::iterator it = m_response_header.begin(); it != m_response_header.end(); it++)
	{
		std::string key = (*it).first;
		std::string val = (*it).second;
		msg += key + ": " + val + "\r\n";
	}
	msg += "\r\n";
	SendString( msg.c_str() );
}


std::string HTTPSocket::MyUseragent()
{
	std::string version = "C++Sockets/";
#ifdef _VERSION
	version += _VERSION;
#endif
	return version;
}


void HTTPSocket::Reset()
{
	m_first = true;
	m_header = true;
	m_request = false;
	m_response = false;
//	SetLineProtocol(true);
        while (m_response_header.size())
        {
                string_m::iterator it = m_response_header.begin();
                m_response_header.erase(it);
        }

}


const std::string& HTTPSocket::GetMethod()
{
	return m_method;
}


void HTTPSocket::SetMethod(const std::string& x)
{
	m_method = x;
}


const std::string& HTTPSocket::GetUrl()
{
	return m_url;
}


void HTTPSocket::SetUrl(const std::string& x)
{
	m_url = x;
}


const std::string& HTTPSocket::GetUri()
{
	return m_uri;
}


const std::string& HTTPSocket::GetQueryString()
{
	return m_query_string;
}


const std::string& HTTPSocket::GetHttpVersion()
{
	return m_http_version;
}


const std::string& HTTPSocket::GetStatus()
{
	return m_status;
}


const std::string& HTTPSocket::GetStatusText()
{
	return m_status_text;
}


bool HTTPSocket::IsRequest()
{
	return m_request;
}


bool HTTPSocket::IsResponse()
{
	return m_response;
}


void HTTPSocket::SetHttpVersion(const std::string& x)
{
	m_http_version = x;
}


void HTTPSocket::SetStatus(const std::string& num, const std::string& text) {
	m_status = num;
	m_status_text = text;
}

void HTTPSocket::SetStatus(const std::string& x)
{
	m_status = x;
}


void HTTPSocket::SetStatusText(const std::string& x)
{
	m_status_text = x;
}


void HTTPSocket::AddResponseHeader(const std::string& x,const std::string& y)
{
	m_response_header[x] = y;
}


void HTTPSocket::SetUri(const std::string& x)
{
	m_uri = x;
}

void HTTPSocket::SendResponse(const std::string& status_num, const std::string& status_text) {
	SetStatus(status_num, status_text);
	SendResponse();
}

#ifdef SOCKETS_NAMESPACE
}
#endif

