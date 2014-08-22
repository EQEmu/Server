/** \file socket_include.h
 **	\date  2005-04-12
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
#ifndef _SOCKET_INCLUDE_H
#define _SOCKET_INCLUDE_H

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif


#ifndef _WIN32 
// ----------------------------------------
// common unix includes / defines
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#define Errno errno
#define StrError strerror

// WIN32 adapt
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif // INADDR_NONE

#ifdef SOCKETS_NAMESPACE
}
#endif

#endif // !_WIN32


// ----------------------------------------
// Generic
#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#endif


// ----------------------------------------
// OS specific adaptions

#ifdef SOLARIS 
// ----------------------------------------
// Solaris
#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

typedef unsigned short port_t;
#ifdef SOCKETS_NAMESPACE
}
#endif

#define s6_addr16 _S6_un._S6_u8
#define MSG_NOSIGNAL 0

#elif defined __FreeBSD__
// ----------------------------------------
// FreeBSD
# if __FreeBSD_version >= 400014
#  define s6_addr16 __u6_addr.__u6_addr16
#  if !defined(MSG_NOSIGNAL)
#   define MSG_NOSIGNAL 0
#  endif
#  include <netinet/in.h>
#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

typedef	in_addr_t ipaddr_t;
typedef	in_port_t port_t;
#ifdef SOCKETS_NAMESPACE
}
#endif

#  define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#  define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
# else
#  error FreeBSD versions prior to 400014 does not support ipv6
# endif

#elif defined MACOSX 
// ----------------------------------------
// Mac OS X
#include <string.h>
#include <mach/port.h>
#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

typedef unsigned long ipaddr_t;
#ifdef SOCKETS_NAMESPACE
}
#endif

#define s6_addr16 __u6_addr.__u6_addr16
#define MSG_NOSIGNAL 0 // oops - thanks Derek
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP

#elif defined _WIN32 
// ----------------------------------------
// Win32
#pragma comment(lib, "wsock32.lib")
#define strcasecmp _stricmp

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

typedef unsigned long ipaddr_t;
typedef unsigned short port_t;
typedef int socklen_t;
#ifdef SOCKETS_NAMESPACE
}
#endif

#define MSG_NOSIGNAL 0
#define SHUT_RDWR 2

// 1.8.6: define FD_SETSIZE to something bigger than 64 if there are a lot of
// simultaneous connections (must be done before including winsock.h)
//#define FD_SETSIZE 1024
#include <winsock.h>

#define Errno WSAGetLastError()
#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

const char *StrError(int x);

// class WSAInitializer is a part of the Socket class (on win32)
// as a static instance - so whenever an application uses a Socket,
// winsock is initialized
class WSAInitializer // Winsock Initializer
{
public:
	WSAInitializer() {
		if (WSAStartup(0x101,&m_wsadata)) 
		{
			exit(-1);
		}
	}
	~WSAInitializer() {
		WSACleanup();
	}
private:
	WSADATA m_wsadata;
};

#ifdef SOCKETS_NAMESPACE
}
#endif

#else 
// ----------------------------------------
// LINUX 
#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

typedef unsigned long ipaddr_t;
typedef unsigned short port_t;
#ifdef SOCKETS_NAMESPACE
}
#endif


#endif

#ifdef _THREADSAFE_SOCKETS
#include "mutex.h"
#include "Lock.h"
#endif

#endif // _SOCKET_INCLUDE_H
