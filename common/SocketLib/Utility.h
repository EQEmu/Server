/** \file Utility.h
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
#ifndef _UTILITY_H
#define _UTILITY_H

#include <ctype.h>
#ifdef _WIN32
typedef unsigned __int64 uint64_t;
#else
#include <stdlib.h>
#ifdef SOLARIS
# include <sys/types.h>
#else
# include <stdint.h>
#endif
#endif
#include "Base64.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

/** Conversion utilities. 
	\ingroup util */
class Utility
{
public:
	static std::string base64(const std::string& str_in);
	static std::string base64d(const std::string& str_in);
	static std::string l2string(long l);
	static std::string bigint2string(uint64_t l);
	static uint64_t atoi64(const std::string& str);
	static unsigned int hex2unsigned(const std::string& str);
	static std::string rfc1738_encode(const std::string& src);
	static std::string rfc1738_decode(const std::string& src);
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _UTILITY_H
