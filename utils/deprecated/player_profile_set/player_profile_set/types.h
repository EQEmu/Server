/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef TYPES_H
#define TYPES_H

// TODO: If we require signed or unsigned we should the s and u types..

typedef unsigned char		int8;
typedef unsigned char		byte;
typedef unsigned short		int16;
typedef unsigned int		int32;

typedef unsigned char		uint8;
typedef  signed  char		sint8;
typedef unsigned short		uint16;
typedef  signed  short		sint16;
typedef unsigned int		uint32;
typedef  signed  int		sint32;

#ifdef WIN32
	#if defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64
		typedef unsigned __int64	int64;
		typedef unsigned __int64	uint64;
		typedef signed __int64		sint64;
	#else
		#error __int64 not supported
	#endif
#else
typedef unsigned long long	int64;
typedef unsigned long long	uint64;
typedef signed long long	sint64;
//typedef __u64				int64;
//typedef __u64				uint64;
//typedef __s64				sint64;
#endif

#ifndef __cplusplus
typedef enum { true, false } bool;
#endif

typedef unsigned long		ulong;
typedef unsigned short		ushort;
typedef unsigned char		uchar;

typedef const char Const_char;	//for perl XS

#ifdef WIN32
	#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
	typedef void ThreadReturnType;
//	#define THREAD_RETURN(x) return;
	#define THREAD_RETURN(x) _endthread(); return;
#else
	typedef void* ThreadReturnType;
//	typedef int SOCKET;
	#define THREAD_RETURN(x) return(x);
#endif

#define safe_delete(d) if(d) { delete d; d=0; }
#define safe_delete_array(d) if(d) { delete[] d; d=0; }
#define L32(i)	((int32) i)
#define H32(i)	((int32) (i >> 32))
#define L16(i)	((int16) i)

#ifndef WIN32
// More WIN32 compatability
	typedef unsigned long DWORD;
	typedef unsigned char BYTE;
	typedef char CHAR;
	typedef unsigned short WORD;
	typedef float FLOAT;
	typedef FLOAT *PFLOAT;
	typedef BYTE *PBYTE,*LPBYTE;
	typedef int *PINT,*LPINT;
	typedef WORD *PWORD,*LPWORD;
	typedef long *LPLONG, LONG;
	typedef DWORD *PDWORD,*LPDWORD;
	typedef int INT;
	typedef unsigned int UINT,*PUINT,*LPUINT;
#endif


#ifdef WIN32
#define DLLFUNC extern "C" __declspec(dllexport)
#else
#define DLLFUNC extern "C"
#endif



#endif
