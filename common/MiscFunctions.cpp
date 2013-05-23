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
#include "MiscFunctions.h"
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

static bool WELLRNG_init = false;
static int state_i = 0;
static unsigned int STATE[R];
static unsigned int z0, z1, z2;
unsigned int (*WELLRNG19937)(void);
static unsigned int case_1 (void);
static unsigned int case_2 (void);
static unsigned int case_3 (void);
static unsigned int case_4 (void);
static unsigned int case_5 (void);
static unsigned int case_6 (void);
uint32 rnd_hash(time_t t, clock_t c);
void oneseed(const uint32 seed);

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

/*
 * generate a random integer in the range low-high this
 * should be used instead of the rand()%limit method
 */
int MakeRandomInt(int low, int high)
{
	_CP(MakeRandomInt);
	if(low >= high)
		return(low);

	//return (rand()%(high-low+1) + (low));
		if(!WELLRNG_init) {
		WELLRNG_init = true;
		oneseed( rnd_hash( time(nullptr), clock() ) );
		WELLRNG19937 = case_1;
	}
	unsigned int randomnum = ((WELLRNG19937)());
	if(randomnum == 0xffffffffUL)
		return high;
	return int ((randomnum / (double)0xffffffffUL) * (high - low + 1) + low);
}

double MakeRandomFloat(double low, double high)
{
	_CP(MakeRandomFloat);
	if(low >= high)
		return(low);

	//return (rand() / (double)RAND_MAX * (high - low) + low);
		if(!WELLRNG_init) {
		WELLRNG_init = true;
		oneseed( rnd_hash( time(nullptr), clock() ) );
		WELLRNG19937 = case_1;
	}
	return ((WELLRNG19937)() / (double)0xffffffffUL * (high - low) + low);
}

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

void oneseed( const uint32 seed )
{
	// Initialize generator state with seed
	// See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
	// In previous versions, most significant bits (MSBs) of the seed affect
	// only MSBs of the state array. Modified 9 Jan 2002 by Makoto Matsumoto.
	register int j = 0;
	STATE[j] = seed & 0xffffffffUL;
	for (j = 1; j < R; j++)
	{
		STATE[j] = ( 1812433253UL * ( STATE[j-1] ^ (STATE[j-1] >> 30) ) + j ) & 0xffffffffUL;
	}
}

// WELL RNG code

/* ***************************************************************************** */
/* Copyright:      Francois Panneton and Pierre L'Ecuyer, University of Montreal */
/*                 Makoto Matsumoto, Hiroshima University                        */
/* Notice:         This code can be used freely for personal, academic,          */
/*                 or non-commercial purposes. For commercial purposes,          */
/*                 please contact P. L'Ecuyer at: lecuyer@iro.UMontreal.ca       */
/*                 A modified "maximally equidistributed" implementation         */
/*                 by Shin Harase, Hiroshima University.                         */
/* ***************************************************************************** */

unsigned int case_1 (void){
	// state_i == 0
	z0 = (VRm1Under & MASKL) | (VRm2Under & MASKU);
	z1 = MAT0NEG (-25, V0) ^ MAT0POS (27, VM1);
	z2 = MAT3POS (9, VM2) ^ MAT0POS (1, VM3);
	newV1 = z1 ^ z2;
	newV0Under = MAT1 (z0) ^ MAT0NEG (-9, z1) ^ MAT0NEG (-21, z2) ^ MAT0POS (21, newV1);
	state_i = R - 1;
	WELLRNG19937 = case_3;
	return (STATE[state_i] ^ (newVM2Over & BITMASK));
}

static unsigned int case_2 (void){
	// state_i == 1
	z0 = (VRm1 & MASKL) | (VRm2Under & MASKU);
	z1 = MAT0NEG (-25, V0) ^ MAT0POS (27, VM1);
	z2 = MAT3POS (9, VM2) ^ MAT0POS (1, VM3);
	newV1 = z1 ^ z2;
	newV0 = MAT1 (z0) ^ MAT0NEG (-9, z1) ^ MAT0NEG (-21, z2) ^ MAT0POS (21, newV1);
	state_i = 0;
	WELLRNG19937 = case_1;
	return (STATE[state_i] ^ (newVM2 & BITMASK));
}

static unsigned int case_3 (void){
	// state_i+M1 >= R
	z0 = (VRm1 & MASKL) | (VRm2 & MASKU);
	z1 = MAT0NEG (-25, V0) ^ MAT0POS (27, VM1Over);
	z2 = MAT3POS (9, VM2Over) ^ MAT0POS (1, VM3Over);
	newV1 = z1 ^ z2;
	newV0 = MAT1 (z0) ^ MAT0NEG (-9, z1) ^ MAT0NEG (-21, z2) ^ MAT0POS (21, newV1);
	state_i--;
	if (state_i + M1 < R)
		WELLRNG19937 = case_5;
	return (STATE[state_i] ^ (newVM2Over & BITMASK));
}

static unsigned int case_4 (void){
	// state_i+M3 >= R
	z0 = (VRm1 & MASKL) | (VRm2 & MASKU);
	z1 = MAT0NEG (-25, V0) ^ MAT0POS (27, VM1);
	z2 = MAT3POS (9, VM2) ^ MAT0POS (1, VM3Over);
	newV1 = z1 ^ z2;
	newV0 = MAT1 (z0) ^ MAT0NEG (-9, z1) ^ MAT0NEG (-21, z2) ^ MAT0POS (21, newV1);
	state_i--;
	if (state_i + M3 < R)
		WELLRNG19937 = case_6;
	return (STATE[state_i] ^ (newVM2 & BITMASK));
}

static unsigned int case_5 (void){
	// state_i+M2 >= R
	z0 = (VRm1 & MASKL) | (VRm2 & MASKU);
	z1 = MAT0NEG (-25, V0) ^ MAT0POS (27, VM1);
	z2 = MAT3POS (9, VM2Over) ^ MAT0POS (1, VM3Over);
	newV1 = z1 ^ z2;
	newV0 = MAT1 (z0) ^ MAT0NEG (-9, z1) ^ MAT0NEG (-21, z2) ^ MAT0POS (21, newV1);
	state_i--;
	if (state_i + M2 < R)
		WELLRNG19937 = case_4;
	return (STATE[state_i] ^ (newVM2Over & BITMASK));
}

static unsigned int case_6 (void){
	// 2 <= state_i <= (R - M3 - 1)
	z0 = (VRm1 & MASKL) | (VRm2 & MASKU);
	z1 = MAT0NEG (-25, V0) ^ MAT0POS (27, VM1);
	z2 = MAT3POS (9, VM2) ^ MAT0POS (1, VM3);
	newV1 = z1 ^ z2;
	newV0 = MAT1 (z0) ^ MAT0NEG (-9, z1) ^ MAT0NEG (-21, z2) ^ MAT0POS (21, newV1);
	state_i--;
	if (state_i == 1)
		WELLRNG19937 = case_2;
	return (STATE[state_i] ^ (newVM2 & BITMASK));
}

// end WELL RNG code


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
