/** \file HttpdCookies.cpp
*/
/*
Copyright (C) 2003-2005  Anders Hedstrom

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

#include "../global_define.h"
#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include "Parse.h"
#include "Utility.h"
#include "HTTPSocket.h"
#include "HttpdCookies.h"
#include "../types.h"
#include <time.h>
#include <cstdlib>
#include <cstring>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


HttpdCookies::HttpdCookies()
{
}

HttpdCookies::HttpdCookies(const std::string& s)
{
	Parse *pa = new Parse(s,";");

	std::string slask = pa -> getword();
	while (slask.size())
	{
		Parse *pa2 = new Parse(slask,"=");
		std::string name = pa2 -> getword();
		std::string value = pa2 -> getword();
		delete pa2;
		COOKIE *c = new COOKIE(name,value);
		m_cookies.push_back(c);
		//
		slask = pa -> getword();
	}
	delete pa;
}

HttpdCookies::~HttpdCookies()
{
	for (cookie_v::iterator it = m_cookies.begin(); it != m_cookies.end(); it++)
	{
		COOKIE *c = *it;
		delete c;
	}
}

bool HttpdCookies::getvalue(const std::string& name,std::string& buffer) //char *buffer,size_t length)
{
	for (cookie_v::iterator it = m_cookies.begin(); it != m_cookies.end(); it++)
	{
		COOKIE *c = *it;
		if (!strcasecmp(c -> name.c_str(),name.c_str()))
		{
			buffer = c -> value;
			return true;
		}
	}
	buffer = "";
	return false;
}

void HttpdCookies::replacevalue(const std::string& name,const std::string& value)
{
	COOKIE *c = nullptr;
	
	for (cookie_v::iterator it = m_cookies.begin(); it != m_cookies.end(); it++)
	{
		c = *it;
		if (!strcasecmp(c -> name.c_str(),name.c_str()))
			break;
		c = nullptr;
	}

	if (c)
	{
		c -> value = value;
	}
	else
	{
		c = new COOKIE(name,value);
		m_cookies.push_back(c);
	}
}

void HttpdCookies::replacevalue(const std::string& name,long l)
{
	replacevalue(name, Utility::l2string(l));
}

void HttpdCookies::replacevalue(const std::string& name,int i)
{
	replacevalue(name, Utility::l2string(i));
}

size_t HttpdCookies::getlength(const std::string& name)
{
	COOKIE *c = nullptr;

	for (cookie_v::iterator it = m_cookies.begin(); it != m_cookies.end(); it++)
	{
		c = *it;
		if (!strcasecmp(c -> name.c_str(),name.c_str()))
			break;
		c = nullptr;
	}
	return c ? c -> value.size() : 0;
}

void HttpdCookies::setcookie(HTTPSocket *sock, const std::string& domain, const std::string& path, const std::string& name, const std::string& value)
{
	char *str = new char[name.size() + value.size() + domain.size() + path.size() + 100];

	// set-cookie response
	if (domain.size())
	{
		sprintf(str, "%s=%s; domain=%s; path=%s; expires=%s",
		 name.c_str(), value.c_str(),
		 domain.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	else
	{
		sprintf(str, "%s=%s; path=%s; expires=%s",
		 name.c_str(), value.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	sock -> AddResponseHeader("Set-cookie", str);
	delete[] str;

	replacevalue(name, value);
}

void HttpdCookies::setcookie(HTTPSocket *sock, const std::string& domain, const std::string& path, const std::string& name, long value)
{
	char *str = new char[name.size() + domain.size() + path.size() + 100];
	char dt[80];

	// set-cookie response
	if (domain.size())
	{
		sprintf(str, "%s=%ld; domain=%s; path=%s; expires=%s",
		 name.c_str(), value,
		 domain.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	else
	{
		sprintf(str, "%s=%ld; path=%s; expires=%s",
		 name.c_str(), value,
		 path.c_str(),
		 expiredatetime().c_str());
	}
	sock -> AddResponseHeader("Set-cookie", str);
	delete[] str;

	sprintf(dt, "%ld", value);
	replacevalue(name, dt);
}

void HttpdCookies::setcookie(HTTPSocket *sock, const std::string& domain, const std::string& path, const std::string& name, int value)
{
	char *str = new char[name.size() + domain.size() + path.size() + 100];
	char dt[80];

	// set-cookie response
	if (domain.size())
	{
		sprintf(str, "%s=%d; domain=%s; path=%s; expires=%s",
		 name.c_str(), value,
		 domain.c_str(),
		 path.c_str(),
		 expiredatetime().c_str());
	}
	else
	{
		sprintf(str, "%s=%d; path=%s; expires=%s",
		 name.c_str(), value,
		 path.c_str(),
		 expiredatetime().c_str());
	}
	sock -> AddResponseHeader("Set-cookie", str);
	delete[] str;

	sprintf(dt, "%d", value);
	replacevalue(name, dt);
}


const std::string& HttpdCookies::expiredatetime()
{
	time_t t = time(nullptr);
	struct tm * tp = gmtime(&t);
	const char *days[7] = {"Sunday", "Monday",
	 "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May",
	 "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char dt[100];

	sprintf(dt, "%s, %02d-%s-%04d %02d:%02d:%02d GMT",
	 days[tp -> tm_wday],
	 tp -> tm_mday,
	 months[tp -> tm_mon],
	 tp -> tm_year + 1910,
	 tp -> tm_hour,
	 tp -> tm_min,
	 tp -> tm_sec);
	m_date = dt;
	return m_date;
}


#ifdef SOCKETS_NAMESPACE
}
#endif

