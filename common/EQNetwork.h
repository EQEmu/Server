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
#ifndef EQNETWORK_H
#define EQNETWORK_H

#include "../common/debug.h"

//uncomment this to enable the packet profiler. Counts the number
//of each type of packet sent or received on a connection.
#ifdef ZONE
//#define PACKET_PROFILER 1
#endif

#include <string.h>
#include <map>
#include <list>
#include <queue>
using namespace std;

#include "../common/types.h"
#include "../common/timer.h"
#include "../common/linked_list.h"
#include "../common/queue.h"
#include "../common/Mutex.h"
#include "../common/packet_functions.h"
#include "EQStream.h"
#ifdef PACKET_PROFILER
#include "../common/rdtsc.h"
#endif

#define EQNC_TIMEOUT	60000
#define NAS_TIMER	100
#define KA_TIMER	400 /* keeps the lag bar constant */
#define MAX_HEADER_SIZE	39 // Quag: 39 is the max header + opcode + crc32 + unknowns size

class EQStreamServer;
class EQStream;
class EQStreamPacket;
class EQStreamFragmentGroupList;
class EQStreamFragmentGroup;
typedef EQStreamServer				EQNServer;
typedef EQStream			EQNConnection;
typedef EQStreamPacket				EQNPacket;
typedef EQStreamFragmentGroupList	EQNFragmentGroupList;
typedef EQStreamFragmentGroup		EQNFragmentGroup;

#define	FLAG_COMPRESSED 0x1000
#define	FLAG_COMBINED	0x2000
#define	FLAG_ENCRYPTED	0x4000
#define	FLAG_IMPLICIT	0x8000
#define FLAG_ALL		0xF000
#define StripFlags(x)	(x & ~FLAG_ALL)

// Optimistic compression, used for guessing pre-alloc size on debug output
#define BEST_COMPR_RATIO 300

enum eappCompressed { appNormal, appInflated, appDeflated };

#ifdef WIN32
	void EQStreamServerLoop(void* tmp);
	void EQStreamInLoop(void* tmp);
	void EQStreamOutLoop(void* tmp);
#else
	void* EQStreamServerLoop(void* tmp);
	void* EQStreamInLoop(void* tmp);
	void* EQStreamOutLoop(void* tmp);
#endif
class EQStreamServer {
public:
	EQStreamServer(uint16 iPort = 0);
	virtual ~EQStreamServer();

	bool	Open(uint16 iPort = 0);			// opens the port
	void	Close();	// closes the port
	void	KillAll();						// kills all clients
	inline uint16	GetPort()		{ return pPort; }

	EQStream* NewQueuePop();
protected:
#ifdef WIN32
	friend void EQStreamServerLoop(void* tmp);
#else
	friend void* EQStreamServerLoop(void* tmp);
#endif
	void		Process();
	bool		IsOpen();
	void		SetOpen(bool iOpen);
	bool		RunLoop;
	Mutex		MLoopRunning;
private:
	void		RecvData(uchar* data, uint32 size, uint32 irIP, uint16 irPort);
#ifdef WIN32
	SOCKET	sock;
#else
	int		sock;
#endif
	uint16	pPort;
	bool	pOpen;
	Mutex	MNewQueue;
	Mutex	MOpen;

	map<string,EQStream*> connection_list;
	queue<EQStream *>		NewQueue;
};

#endif
