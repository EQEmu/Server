/** \file Base64.h
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
#ifndef _BASE64_H
#define _BASE64_H

#include <stdio.h>
#include <string>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

/** \defgroup util Utilities */

/** Base64 encode/decode. 
	\ingroup util */
class Base64 {
public:

	static void encode(FILE *, std::string& , bool add_crlf = true);
	static void encode(const std::string&, std::string& , bool add_crlf = true);
	static void encode(const char *, size_t, std::string& , bool add_crlf = true);
	static void encode(unsigned char *, size_t, std::string& , bool add_crlf = true);

	static void decode(const std::string&, std::string& );
	static void decode(const std::string& in, unsigned char *out, size_t&);

	static size_t decode_length(const std::string& );

private:
static	const char *bstr;
static	const char rstr[128];
};


#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // _BASE64_H
