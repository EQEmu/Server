#ifndef __OPENEQ_GLOBALS__
#define __OPENEQ_GLOBALS__

#ifdef WIN32
#define Polygon Polygon_Win32
#include <winsock.h>
#undef Polygon
#else
#include <netinet/in.h>
#endif

typedef unsigned int uint32;
typedef int int32;

typedef unsigned short uint16;
typedef short int16;

typedef unsigned char uint8;
typedef unsigned char uchar;
typedef char int8;

uint16 GetUint16(uchar **buf);
uint32 GetUint32(uchar **buf);

#define uint16(buf) \
GetUint16((uchar **) &buf)
#define uint32(buf) \
GetUint32((uchar **) &buf)

#endif
