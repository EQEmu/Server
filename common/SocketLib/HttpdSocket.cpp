/** \file HttpdSocket.cpp
*/
/*
Copyright (C) 2001-2004,2005  Anders Hedstrom (grymse@alhem.net)

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
#include "Utility.h"
#include "HttpdCookies.h"
#include "HttpdForm.h"
#include "MemFile.h"
#include "HttpdSocket.h"
#include "../types.h"
#include <time.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>

#define DEB(x)
/*
#define DEB(x) { \
	FILE *fil = fopen("httpdlog","at"); \
	if (!fil) \
		fil = fopen("httpdlog","wt"); \
	if (fil) { x; fclose(fil); } \
}
*/

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif



// statics
int HttpdSocket::m_request_count = 0;
std::string HttpdSocket::m_start = "";


HttpdSocket::HttpdSocket(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort)
: HTTPSocket(ID,in_socket,irIP,irPort)
,m_content_length(0)
,m_file(nullptr)
,m_received(0)
,m_request_id(++m_request_count)
,m_cookies(nullptr)
,m_form(nullptr)
{
	m_http_date = datetime2httpdate(GetDate());
	if (!m_start.size())
		m_start = m_http_date;
}


HttpdSocket::~HttpdSocket()
{
	if (m_file)
	{
		delete m_file;
	}
	if (m_cookies)
		delete m_cookies;
	if (m_form)
		delete m_form;
}


void HttpdSocket::OnFirst()
{
//	printf("Request: %s %s %s\n",GetMethod().c_str(),GetUrl().c_str(),GetHttpVersion().c_str());
}


void HttpdSocket::OnHeader(const std::string& key,const std::string& value)
{
	if (!strcasecmp(key.c_str(),"content-length"))
	{
		m_content_length = atoi(value.c_str());
		m_content_length_str = value;
	}
	else
	if (!strcasecmp(key.c_str(),"cookie"))
	{
		m_http_cookie = value;
	}
	else
	if (!strcasecmp(key.c_str(),"content-type"))
	{
		m_content_type = value;
	}
	else
	if (!strcasecmp(key.c_str(),"if-modified-since"))
	{
		m_if_modified_since = value;
	}
}


void HttpdSocket::OnHeaderComplete()
{
	m_cookies = new HttpdCookies(m_http_cookie);

#if (defined(SOLARIS8) || defined(SOLARIS))
	{
		char slask[1000];
		if (GetMethod() == "GET")
		{
			sprintf(slask,"QUERY_STRING=%s", GetQueryString().c_str());
			putenv(slask);
		}
		sprintf(slask,"REQUEST_METHOD=%s", GetMethod().c_str());
		putenv(slask);
		sprintf(slask,"HTTP_COOKIE=%s", m_http_cookie.c_str());
		putenv(slask);
		sprintf(slask,"CONTENT_TYPE=%s", m_content_type.c_str());
		putenv(slask);
		sprintf(slask,"CONTENT_LENGTH=%s", m_content_length_str.c_str());
		putenv(slask);
	}
#elif defined _WIN32
	{
		char slask[1000];
		if (GetMethod() == "GET")
		{
			sprintf(slask,"QUERY_STRING=%s", GetQueryString().c_str());
			_putenv(slask);
		}
		sprintf(slask,"REQUEST_METHOD=%s", GetMethod().c_str());
		_putenv(slask);
		sprintf(slask,"HTTP_COOKIE=%s", m_http_cookie.c_str());
		_putenv(slask);
		sprintf(slask,"CONTENT_TYPE=%s", m_content_type.c_str());
		_putenv(slask);
		sprintf(slask,"CONTENT_LENGTH=%s", m_content_length_str.c_str());
		_putenv(slask);
	}
#else
	if (GetMethod() == "GET")
	{
		setenv("QUERY_STRING", GetQueryString().c_str(), 1);
	}
	setenv("REQUEST_METHOD", GetMethod().c_str(), 1);
	setenv("HTTP_COOKIE", m_http_cookie.c_str(), 1);
	setenv("CONTENT_TYPE", m_content_type.c_str(), 1);
	setenv("CONTENT_LENGTH", m_content_length_str.c_str(), 1);
#endif

	if (GetMethod() == "POST")
	{
		m_file = new MemFile;
	}
	else
	if (GetMethod() == "GET")
	{
		m_form = new HttpdForm(GetQueryString(), GetQueryString().size() );
		AddResponseHeader("Date", datetime2httpdate(GetDate()) );
		Exec();
		Reset(); // prepare for next request
	}
	else
	{
		AddResponseHeader("Date", GetHttpDate());
		AddResponseHeader("Connection", "close");
		SetStatus("405");
		SetStatusText("Method not allowed");
		SendResponse();
	}
}


void HttpdSocket::OnData(const char *p,size_t l)
{
	if (m_file)
	{
		m_file -> fwrite(p,1,l);
	}
	m_received += l;
	if (m_received >= m_content_length && m_content_length)
	{
		// all done
		if (m_file && !m_form)
		{
			m_form = new HttpdForm(m_file);
			AddResponseHeader("Date", datetime2httpdate(GetDate()) );
			Exec();
			Reset(); // prepare for next request
		}
	}
}


void HttpdSocket::Send64(const std::string& str64, const std::string& type)
{
	if (!strcasecmp(m_start.c_str(), m_if_modified_since.c_str()))
	{
		SetStatus("304");
		SetStatusText("Not Modified");
		SendResponse();
	}
	else
	{
		size_t len = Base64::decode_length(str64);
		unsigned char *buf = new unsigned char[len];

		SetStatus("200");
		SetStatusText("OK");

		AddResponseHeader("Content-length", Utility::l2string( (long)len) );
		AddResponseHeader("Content-type", type );
		AddResponseHeader("Last-modified", m_start);
		SendResponse();

		Base64::decode(str64, buf, len);
		SendBuf( (char *)buf, len);
		delete[] buf;
	}
}


std::string HttpdSocket::datetime2httpdate(const std::string& dt)
{
	struct tm tp;
	time_t t;
	const char *days[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
	const char *months[] = { "Jan","Feb","Mar","Apr","May","Jun",
	                   "Jul","Aug","Sep","Oct","Nov","Dec" };
	int i;
	char s[40];

/* 1997-12-16 09:50:40 */

	if (dt.size() == 19) 
	{
		tp.tm_year = atoi(dt.substr(0,4).c_str()) - 1900;
		i = atoi(dt.substr(5,2).c_str()) - 1;
		tp.tm_mon = i >= 0 ? i : 0;
		tp.tm_mday = atoi(dt.substr(8,2).c_str());
		tp.tm_hour = atoi(dt.substr(11,2).c_str());
		tp.tm_min = atoi(dt.substr(14,2).c_str());
		tp.tm_sec = atoi(dt.substr(17,2).c_str());
		tp.tm_wday = 0;
		tp.tm_yday = 0;
		tp.tm_isdst = 0;
		t = mktime(&tp);
		/*if (t == -1)
		{
			Handler().LogError(this, "datetime2httpdate", 0, "mktime() failed");
		}*/

		sprintf(s,"%s, %02d %s %d %02d:%02d:%02d GMT",
		 days[tp.tm_wday],
		 tp.tm_mday,
		 months[tp.tm_mon],
		 tp.tm_year + 1900,
		 tp.tm_hour,tp.tm_min,tp.tm_sec);
	} 
	else
	{
		*s = 0;
	}
	return s;
}


std::string HttpdSocket::GetDate()
{
	time_t t = time(nullptr);
	struct tm* tp = localtime(&t);
	char slask[40];
	if (tp)
	{
		sprintf(slask,"%d-%02d-%02d %02d:%02d:%02d",
			tp -> tm_year + 1900,
			tp -> tm_mon + 1,
			tp -> tm_mday,
			tp -> tm_hour,tp -> tm_min,tp -> tm_sec);
	}
	else
	{
		*slask = 0;
	}
	return slask;
}


void HttpdSocket::Reset()
{
	HTTPSocket::Reset();
	m_content_length = 0;
	if (m_file)
	{
		delete m_file;
		m_file = nullptr;
	}
	m_received = 0;
	m_request_id = ++m_request_count;
	if (m_cookies)
		delete m_cookies;
	m_cookies = nullptr;
	if (m_form)
		delete m_form;
	m_form = nullptr;
}


const std::string& HttpdSocket::GetHttpDate() 
{
	return m_http_date; 
}


HttpdCookies *HttpdSocket::GetCookies() 
{
	return m_cookies; 
}


HttpdForm *HttpdSocket::GetHttpForm() 
{
	return m_form; 
}



#ifdef SOCKETS_NAMESPACE
}
#endif

