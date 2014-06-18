/* sys/socket.h */

/* djl */
/* Provide UNIX compatibility */

#ifndef  _INC_SYS_SOCKET
#define  _INC_SYS_SOCKET

#define WIN32_LEAN_AND_MEAN
#ifdef __GNUC__
#  define Win32_Winsock
#endif
#include <windows.h>

/* Too late to include winsock2.h if winsock.h has already been loaded */
#ifndef _WINSOCKAPI_
#  ifndef FD_SETSIZE
#    define FD_SETSIZE 2048
#  endif
#  if defined(UNDER_CE) && UNDER_CE <= 300
     /* winsock2 only for 4.00+ */
#    include <winsock.h>
#  else
#    include <winsock2.h>
     /* We need to include ws2tcpip.h to get the IPv6 definitions.
      * It will in turn include wspiapi.h.  Later versions of that
      * header in the Windows SDK generate C++ template code that
      * can't be compiled with VC6 anymore.  The _WSPIAPI_COUNTOF
      * definition below prevents wspiapi.h from generating this
      * incompatible code.
      */
#    define _WSPIAPI_COUNTOF(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#    include <ws2tcpip.h>

#    ifndef SIO_GET_INTERFACE_LIST_EX
       /* The ws2tcpip.h header included in VC6 doesn't define the
        * sin6_scope_id member of sockaddr_in6.  We define our own
        * version and redefine sockaddr_in6 to point to this one
        * instead for compiling e.g. Socket.xs.
        */
       struct my_sockaddr_in6 {
           short   sin6_family;        /* AF_INET6 */
           u_short sin6_port;          /* Transport level port number */
           u_long  sin6_flowinfo;      /* IPv6 flow information */
           struct in_addr6 sin6_addr;  /* IPv6 address */
           u_long sin6_scope_id;       /* set of interfaces for a scope */
       };
#      define sockaddr_in6 my_sockaddr_in6

       /* Provide implementations of IN6ADDR_SETANY() and IN6ADDR_SETLOOPBACK
        * that also initialize the sin6_scope_id field.
        */
#      undef IN6ADDR_SETANY
#      define IN6ADDR_SETANY(x) {\
(x)->sin6_family = AF_INET6; \
(x)->sin6_port = 0; \
(x)->sin6_flowinfo = 0; \
*((u_long *)((x)->sin6_addr.s6_addr)    ) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 1) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 2) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 3) = 0; \
(x)->sin6_scope_id = 0; \
}

#      undef IN6ADDR_SETLOOPBACK
#      define IN6ADDR_SETLOOPBACK(x) {\
(x)->sin6_family = AF_INET6; \
(x)->sin6_port = 0; \
(x)->sin6_flowinfo = 0; \
*((u_long *)((x)->sin6_addr.s6_addr)    ) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 1) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 2) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 3) = 1; \
(x)->sin6_scope_id = 0; \
}

#    endif

#  endif
#endif

#include "win32.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef ENOTSOCK
#define ENOTSOCK       WSAENOTSOCK

#undef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED

#undef ECONNRESET
#define ECONNRESET WSAECONNRESET

#undef EAFNOSUPPORT
#define EAFNOSUPPORT WSAEAFNOSUPPORT

#ifdef USE_SOCKETS_AS_HANDLES

#ifndef PERL_FD_SETSIZE
#  define PERL_FD_SETSIZE 2048
#endif

#define PERL_BITS_PER_BYTE	8
#define	PERL_NFDBITS		(sizeof(Perl_fd_mask)*PERL_BITS_PER_BYTE)

typedef int			Perl_fd_mask;

typedef struct	Perl_fd_set {
    Perl_fd_mask bits[(PERL_FD_SETSIZE+PERL_NFDBITS-1)/PERL_NFDBITS];
}				Perl_fd_set;

#define PERL_FD_CLR(n,p) \
    ((p)->bits[(n)/PERL_NFDBITS] &= ~((unsigned)1 << ((n)%PERL_NFDBITS)))

#define PERL_FD_SET(n,p) \
    ((p)->bits[(n)/PERL_NFDBITS] |=  ((unsigned)1 << ((n)%PERL_NFDBITS)))

#define PERL_FD_ZERO(p) memset((char *)(p),0,sizeof(*(p)))

#define PERL_FD_ISSET(n,p) \
    ((p)->bits[(n)/PERL_NFDBITS] &   ((unsigned)1 << ((n)%PERL_NFDBITS)))

#else	/* USE_SOCKETS_AS_HANDLES */

#define Perl_fd_set	fd_set
#define PERL_FD_SET(n,p)	FD_SET(n,p)
#define PERL_FD_CLR(n,p)	FD_CLR(n,p)
#define PERL_FD_ISSET(n,p)	FD_ISSET(n,p)
#define PERL_FD_ZERO(p)		FD_ZERO(p)

#endif	/* USE_SOCKETS_AS_HANDLES */

SOCKET win32_accept (SOCKET s, struct sockaddr *addr, int *addrlen);
int win32_bind (SOCKET s, const struct sockaddr *addr, int namelen);
int win32_closesocket (SOCKET s);
int win32_connect (SOCKET s, const struct sockaddr *name, int namelen);
int win32_ioctlsocket (SOCKET s, long cmd, u_long *argp);
int win32_getpeername (SOCKET s, struct sockaddr *name, int * namelen);
int win32_getsockname (SOCKET s, struct sockaddr *name, int * namelen);
int win32_getsockopt (SOCKET s, int level, int optname, char * optval, int *optlen);
u_long win32_htonl (u_long hostlong);
u_short win32_htons (u_short hostshort);
unsigned long win32_inet_addr (const char * cp);
char * win32_inet_ntoa (struct in_addr in);
int win32_listen (SOCKET s, int backlog);
u_long win32_ntohl (u_long netlong);
u_short win32_ntohs (u_short netshort);
int win32_recv (SOCKET s, char * buf, int len, int flags);
int win32_recvfrom (SOCKET s, char * buf, int len, int flags,
                         struct sockaddr *from, int * fromlen);
int win32_select (int nfds, Perl_fd_set *rfds, Perl_fd_set *wfds, Perl_fd_set *xfds,
		  const struct timeval *timeout);
int win32_send (SOCKET s, const char * buf, int len, int flags);
int win32_sendto (SOCKET s, const char * buf, int len, int flags,
                       const struct sockaddr *to, int tolen);
int win32_setsockopt (SOCKET s, int level, int optname,
                           const char * optval, int optlen);
SOCKET win32_socket (int af, int type, int protocol);
int win32_shutdown (SOCKET s, int how);

/* Database function prototypes */

struct hostent * win32_gethostbyaddr(const char * addr, int len, int type);
struct hostent * win32_gethostbyname(const char * name);
int win32_gethostname (char * name, int namelen);
struct servent * win32_getservbyport(int port, const char * proto);
struct servent * win32_getservbyname(const char * name, const char * proto);
struct protoent * win32_getprotobynumber(int proto);
struct protoent * win32_getprotobyname(const char * name);
struct protoent *win32_getprotoent(void);
struct servent *win32_getservent(void);
void win32_sethostent(int stayopen);
void win32_setnetent(int stayopen);
struct netent * win32_getnetent(void);
struct netent * win32_getnetbyname(char *name);
struct netent * win32_getnetbyaddr(long net, int type);
void win32_setprotoent(int stayopen);
void win32_setservent(int stayopen);
void win32_endhostent(void);
void win32_endnetent(void);
void win32_endprotoent(void);
void win32_endservent(void);

#ifndef WIN32SCK_IS_STDSCK

/* direct to our version */

#define htonl		win32_htonl
#define htons		win32_htons
#define ntohl		win32_ntohl
#define ntohs		win32_ntohs
#define inet_addr	win32_inet_addr
#define inet_ntoa	win32_inet_ntoa

#define socket		win32_socket
#define bind		win32_bind
#define listen		win32_listen
#define accept		win32_accept
#define connect		win32_connect
#define send		win32_send
#define sendto		win32_sendto
#define recv		win32_recv
#define recvfrom	win32_recvfrom
#define shutdown	win32_shutdown
#define closesocket	win32_closesocket
#define ioctlsocket	win32_ioctlsocket
#define setsockopt	win32_setsockopt
#define getsockopt	win32_getsockopt
#define getpeername	win32_getpeername
#define getsockname	win32_getsockname
#define gethostname	win32_gethostname
#define gethostbyname	win32_gethostbyname
#define gethostbyaddr	win32_gethostbyaddr
#define getprotobyname	win32_getprotobyname
#define getprotobynumber win32_getprotobynumber
#define getservbyname	win32_getservbyname
#define getservbyport	win32_getservbyport
#define select		win32_select
#define endhostent	win32_endhostent
#define endnetent	win32_endnetent
#define endprotoent	win32_endprotoent
#define endservent	win32_endservent
#define getnetent	win32_getnetent
#define getnetbyname	win32_getnetbyname
#define getnetbyaddr	win32_getnetbyaddr
#define getprotoent	win32_getprotoent
#define getservent	win32_getservent
#define sethostent	win32_sethostent
#define setnetent	win32_setnetent
#define setprotoent	win32_setprotoent
#define setservent	win32_setservent

#ifdef USE_SOCKETS_AS_HANDLES
#undef fd_set
#undef FD_SET
#undef FD_CLR
#undef FD_ISSET
#undef FD_ZERO
#define fd_set		Perl_fd_set
#define FD_SET(n,p)	PERL_FD_SET(n,p)
#define FD_CLR(n,p)	PERL_FD_CLR(n,p)
#define FD_ISSET(n,p)	PERL_FD_ISSET(n,p)
#define FD_ZERO(p)	PERL_FD_ZERO(p)
#endif	/* USE_SOCKETS_AS_HANDLES */

#endif	/* WIN32SCK_IS_STDSCK */

#ifdef __cplusplus
}
#endif

#endif	/* _INC_SYS_SOCKET */
