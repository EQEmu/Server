#ifndef __OPENEQ_GLOBALS__
#define __OPENEQ_GLOBALS__

#ifdef WIN32
//#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#ifndef NULL
#define NULL 0
#endif

typedef unsigned long uint32;
typedef long int32;

typedef unsigned short uint16;
typedef short int16;

typedef unsigned char uint8;
typedef unsigned char uchar;
typedef char int8;

#endif
