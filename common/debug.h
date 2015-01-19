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

// Debug Levels
#ifndef EQDEBUG
#define EQDEBUG 1
#else
////// File/Console options
// 0 <= Quiet mode Errors to file Status and Normal ignored
// 1 >= Status and Normal to console, Errors to file
// 2 >= Status, Normal, and Error to console and logfile
// 3 >= Lite debug
// 4 >= Medium debug
// 5 >= Debug release (Anything higher is not recommended for regular use)
// 6 == (Reserved for special builds) Login opcode debug All packets dumped
// 7 == (Reserved for special builds) Chat Opcode debug All packets dumped
// 8 == (Reserved for special builds) World opcode debug All packets dumped
// 9 == (Reserved for special builds) Zone Opcode debug All packets dumped
// 10 >= More than you ever wanted to know
//
/////
// Add more below to reserve for file's functions ect.
/////
// Any setup code based on defines should go here
//
#endif


#if defined(_DEBUG) && defined(WIN32)
	#ifndef _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>
	#endif
#endif

#ifndef EQDEBUG_H
#define EQDEBUG_H

#ifndef _WINDOWS
	#define DebugBreak()			if(0) {}
#endif

#define _WINSOCKAPI_	//stupid windows, trying to fix the winsock2 vs. winsock issues
#if defined(WIN32) && ( defined(PACKETCOLLECTOR) || defined(COLLECTOR) )
	// Packet Collector on win32 requires winsock.h due to latest pcap.h
	// winsock.h must come before windows.h
	#include <winsock.h>
#endif

#ifdef _WINDOWS
	#include <windows.h>
	#include <winsock2.h>
#endif



#include "../common/mutex.h"
#include <stdio.h>
#include <stdarg.h>


class EQEmuLog {
public:
	EQEmuLog();
	~EQEmuLog();
private:
};

extern EQEmuLog* LogFile;

#endif
