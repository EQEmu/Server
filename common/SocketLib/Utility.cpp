/** \file Utility.cpp
 **	\date  2004-02-13
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
#include "Utility.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


std::string Utility::base64(const std::string& str_in)
{
	std::string str;
	Base64::encode(str_in, str, false); // , false == do not add cr/lf
	return str;
}


std::string Utility::base64d(const std::string& str_in)
{
	std::string str;
	Base64::decode(str_in, str);
	return str;
}


std::string Utility::l2string(long l)
{
	std::string str;
	char tmp[100];
	sprintf(tmp,"%ld",l);
	str = tmp;
	return str;
}


std::string Utility::bigint2string(uint64_t l)
{
	std::string str;
	uint64_t tmp = l;
	while (tmp)
	{
		uint64_t a = tmp % 10;
		str = (char)(a + 48) + str;
		tmp /= 10;
	}
	if (!str.size())
	{
		str = "0";
	}
	return str;
}


uint64_t Utility::atoi64(const std::string& str) 
{
	uint64_t l = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		l = l * 10 + str[i] - 48;
	}
	return l;
}


unsigned int Utility::hex2unsigned(const std::string& str)
{
	unsigned int r = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		r = r * 16 + str[i] - 48 - ((str[i] >= 'A') ? 7 : 0) - ((str[i] >= 'a') ? 32 : 0);
	}
	return r;
}


/*
* Encode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
std::string Utility::rfc1738_encode(const std::string& src)
{
static	char hex[] = "0123456789ABCDEF";
	std::string dst;
	for (size_t i = 0; i < src.size(); i++)
	{
		if (isalnum(src[i]))
		{
			dst += src[i];
		}
		else
		if (src[i] == ' ')
		{
			dst += '+';
		}
		else
		{
			dst += '%';
			dst += hex[src[i] / 16];
			dst += hex[src[i] % 16];
		}
	}
	return dst;
} // rfc1738_encode


/*
* Decode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
std::string Utility::rfc1738_decode(const std::string& src)
{
	std::string dst;
	for (size_t i = 0; i < src.size(); i++)
	{
		if (src[i] == '%' && isxdigit(src[i + 1]) && isxdigit(src[i + 2]))
		{
			char c1 = src[++i];
			char c2 = src[++i];
			c1 = c1 - 48 - ((c1 >= 'A') ? 7 : 0) - ((c1 >= 'a') ? 32 : 0);
			c2 = c2 - 48 - ((c2 >= 'A') ? 7 : 0) - ((c2 >= 'a') ? 32 : 0);
			dst += (char)(c1 * 16 + c2);
		}
		else
		if (src[i] == '+')
		{
			dst += ' ';
		}
		else
		{
			dst += src[i];
		}
	}
	return dst;
} // rfc1738_decode


#ifdef SOCKETS_NAMESPACE
}
#endif

