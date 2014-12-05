/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/debug.h"
#include "misc_functions.h"
#include <string.h>
#include <time.h>
#include <math.h>
#ifndef WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <iostream>
#include <iomanip>
#ifdef _WINDOWS
	#include <io.h>
#endif
#include "../common/timer.h"
#include "../common/seperator.h"

#ifdef _WINDOWS
	#include <windows.h>

	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <stdlib.h>
	#include <ctype.h>
	#include <stdarg.h>
	#include <sys/types.h>
	#include <sys/time.h>
#ifdef FREEBSD //Timothy Whitman - January 7, 2003
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif
	#include <sys/stat.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <errno.h>
#endif

void CoutTimestamp(bool ms) {
	time_t rawtime;
	struct tm* gmt_t;
	time(&rawtime);
	gmt_t = gmtime(&rawtime);

	struct timeval read_time;
	gettimeofday(&read_time,0);

	std::cout << (gmt_t->tm_year + 1900) << "/" << std::setw(2) << std::setfill('0') << (gmt_t->tm_mon + 1) << "/" << std::setw(2) << std::setfill('0') << gmt_t->tm_mday << " " << std::setw(2) << std::setfill('0') << gmt_t->tm_hour << ":" << std::setw(2) << std::setfill('0') << gmt_t->tm_min << ":" << std::setw(2) << std::setfill('0') << gmt_t->tm_sec;
	if (ms)
		std::cout << "." << std::setw(3) << std::setfill('0') << (read_time.tv_usec / 1000);
	std::cout << " GMT";
}


int32 filesize(FILE* fp) {
#ifdef _WINDOWS
	return _filelength(_fileno(fp));
#else
	struct stat file_stat;
	fstat(fileno(fp), &file_stat);
	return (int32) file_stat.st_size;
/*	uint32 tmp = 0;
	while (!feof(fp)) {
		fseek(fp, tmp++, SEEK_SET);
	}
	return tmp;*/
#endif
}

uint32 ResolveIP(const char* hostname, char* errbuf) {
#ifdef _WINDOWS
	static InitWinsock ws;
#endif
	if (errbuf)
		errbuf[0] = 0;
	if (hostname == 0) {
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "ResolveIP(): hostname == 0");
		return 0;
	}
	struct sockaddr_in	server_sin;
#ifdef _WINDOWS
	PHOSTENT phostent = nullptr;
#else
	struct hostent *phostent = nullptr;
#endif
	server_sin.sin_family = AF_INET;
	if ((phostent = gethostbyname(hostname)) == nullptr) {
#ifdef _WINDOWS
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %i", WSAGetLastError());
#else
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %s", strerror(errno));
#endif
		return 0;
	}
#ifdef _WINDOWS
	memcpy ((char FAR *)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#else
	memcpy ((char*)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#endif
	return server_sin.sin_addr.s_addr;
}

bool ParseAddress(const char* iAddress, uint32* oIP, uint16* oPort, char* errbuf) {
	Seperator sep(iAddress, ':', 2, 250, false, 0, 0);
	if (sep.argnum == 1 && sep.IsNumber(1)) {
		*oIP = ResolveIP(sep.arg[0], errbuf);
		if (*oIP == 0)
			return false;
		if (oPort)
			*oPort = atoi(sep.arg[1]);
		return true;
	}
	return false;
}

#ifdef _WINDOWS
InitWinsock::InitWinsock() {
	WORD version = MAKEWORD (1,1);
	WSADATA wsadata;
	WSAStartup (version, &wsadata);
}

InitWinsock::~InitWinsock() {
	WSACleanup();
}

#endif


const char * itoa(int num) {
		static char temp[_ITOA_BUFLEN];
		memset(temp,0,_ITOA_BUFLEN);
		snprintf(temp,_ITOA_BUFLEN,"%d",num);
		return temp;
}

#ifndef WIN32
const char * itoa(int num, char* a,int b) {
		static char temp[_ITOA_BUFLEN];
		memset(temp,0,_ITOA_BUFLEN);
		snprintf(temp,_ITOA_BUFLEN,"%d",num);
		return temp;
		return temp;
}
#endif

uint32 rnd_hash( time_t t, clock_t c )
{
	// Get a uint32 from t and c
	// Better than uint32(x) in case x is floating point in [0,1]
	// Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

	static uint32 differ = 0; // guarantee time-based seeds will change

	uint32 h1 = 0;
	unsigned char *p = (unsigned char *) &t;
	for( size_t i = 0; i < sizeof(t); ++i )
	{
		h1 *= 255 + 2U;
		h1 += p[i];
	}
	uint32 h2 = 0;
	p = (unsigned char *) &c;
	for( size_t j = 0; j < sizeof(c); ++j )
	{
		h2 *= 255 + 2U;
		h2 += p[j];
	}
	return ( h1 + differ++ ) ^ h2;
}

float EQ13toFloat(int d)
{
	return ( float(d)/float(1<<2));
}

float NewEQ13toFloat(int d)
{
	return ( float(d)/float(1<<6));
}

float EQ19toFloat(int d)
{
	return ( float(d)/float(1<<3));
}

int FloatToEQ13(float d)
{
	return int(d*float(1<<2));
}

int NewFloatToEQ13(float d)
{
	return int(d*float(1<<6));
}

int FloatToEQ19(float d)
{
	return int(d*float(1<<3));
}

/*
	Heading of 0 points in the pure positive Y direction

*/
int FloatToEQH(float d)
{
	return(int((360.0f - d) * float(1<<11)) / 360);
}

float EQHtoFloat(int d)
{
	return(360.0f - float((d * 360) >> 11));
}

// returns a swapped-bit value for use in client translator and inventory functions
uint32 SwapBits21and22(uint32 mask)
{
	static const uint32 BIT21 = 1 << 21;
	static const uint32 BIT22 = 1 << 22;
	static const uint32 SWAPBITS = (BIT21 | BIT22);

	if ((bool)(mask & BIT21) != (bool)(mask & BIT22))
		mask ^= SWAPBITS;

	return mask;
}

// returns an unset bit 22 value for use in client translators
uint32 Catch22(uint32 mask)
{
	static const uint32 KEEPBITS = ~(1 << 22);

	mask &= KEEPBITS;

	return mask;
}
