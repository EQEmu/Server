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
/*
 * EQStream classes, by Quagmire
*/

#include "../common/debug.h"

#include <iostream>
#include <iomanip>
#ifdef WIN32
	#include <process.h>
#else
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <stdlib.h>
	#include <string.h>
	#include <pthread.h>
	#include "../common/unix.h"
	#define SOCKET_ERROR -1
#endif
#include "EQNetwork.h"
#include "EQStream.h"
#include "../common/packet_dump.h"
#include "../common/packet_dump_file.h"
#include "../common/packet_functions.h"
#include "../common/MiscFunctions.h"
#include "../common/crc32.h"
#include "../common/eq_packet_structs.h"

#define EQN_DEBUG			0
#define EQN_DEBUG_Error		0
#define EQN_DEBUG_Packet	0
#define EQN_DEBUG_Fragment	0
#define EQN_DEBUG_ACK		0
#define EQN_DEBUG_Unknown	0
#define EQN_DEBUG_NewStream	0
#define LOG_PACKETS			0
#define LOG_RAW_PACKETS_OUT	0
#define LOG_RAW_PACKETS_IN	0
//#define PRIORITYTEST

template <typename type>					// LO_BYTE
type LO_BYTE (type a) {return (a&=0xff);}
template <typename type>					// HI_BYTE
type HI_BYTE (type a) {return (a&=0xff00);}
template <typename type>					// LO_WORD
type LO_WORD (type a) {return (a&=0xffff);}
template <typename type>					// HI_WORD
type HI_WORD (type a) {return (a&=0xffff0000);}
template <typename type>					// HI_LOSWAPshort
type HI_LOSWAPshort (type a) {return (LO_BYTE(a)<<8) | (HI_BYTE(a)>>8);}
template <typename type>					// HI_LOSWAPlong
type HI_LOSWAPlong (type x) {return (LO_WORD(a)<<16) | (HIWORD(a)>>16);}

EQStreamServer::EQStreamServer(uint16 iPort) {
	RunLoop = false;
	pPort = iPort;
	pOpen = false;
#ifdef WIN32
	WORD version = MAKEWORD (1,1);
	WSADATA wsadata;
	WSAStartup (version, &wsadata);
#endif
	sock = 0;
}

EQStreamServer::~EQStreamServer() {
	Close();
	RunLoop = false;
	MLoopRunning.lock();
	MLoopRunning.unlock();
#ifdef WIN32
	WSACleanup();
#endif
	connection_list.clear();
	while (!NewQueue.empty())
		NewQueue.pop(); // they're deleted with the list, clear this queue so it doesnt try to delete them again
}

bool EQStreamServer::Open(uint16 iPort) {
	LockMutex lock(&MOpen);
	if (iPort && pPort != iPort) {
		if (pOpen)
			return false;
		pPort = iPort;
	}
	if (!RunLoop) {
		RunLoop = true;
#ifdef WIN32
		_beginthread(EQStreamServerLoop, 0, this);
#else
		pthread_t thread;
		pthread_create(&thread, NULL, &EQStreamServerLoop, this);
#endif
	}
	if (pOpen) {
		return true;
	}
	else {
		struct sockaddr_in address;
//		int reuse_addr = 1;
		int bufsize = 64 * 1024; // 64kbyte send/recieve buffers, up from default of 8k
#ifdef WIN32
		unsigned long nonblocking = 1;
#endif

		/* Setup internet address information.
		This is used with the bind() call */
		memset((char *) &address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(pPort);
		address.sin_addr.s_addr = htonl(INADDR_ANY);

		/* Setting up UDP port for new clients */
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock < 0) {
			return false;
		}

//#ifdef WIN32
//		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse_addr, sizeof(reuse_addr));
		setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &bufsize, sizeof(bufsize));
		setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*) &bufsize, sizeof(bufsize));
//#else
//		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
//		setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
//		setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
//#endif

		if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
#ifdef WIN32
			closesocket(sock);
#else
			close(sock);
#endif
			return false;
		}

#ifdef WIN32
		ioctlsocket (sock, FIONBIO, &nonblocking);
#else
		fcntl(sock, F_SETFL, O_NONBLOCK);
#endif
		pOpen = true;
		return true;
	}
}

void EQStreamServer::Close() {
	SetOpen(false);
	if (sock) {
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
	}
	sock = 0;
}

bool EQStreamServer::IsOpen() {
	MOpen.lock();
	bool ret = pOpen;
	MOpen.unlock();
	return ret;
}

void EQStreamServer::SetOpen(bool iOpen) {
	MOpen.lock();
	pOpen = iOpen;
	MOpen.unlock();
}

void EQStreamServer::Process() {
	_CP(EQStreamServer_Process);
	if (!IsOpen()) {
		if (sock) {
#ifdef WIN32
			closesocket(sock);
#else
			close(sock);
#endif
			sock = 0;
		}
		return;
	}

	uchar		buffer[1518];

	int			status;
	struct sockaddr_in	from;
	unsigned int	fromlen;

	from.sin_family = AF_INET;
	fromlen = sizeof(from);

	while (1) {
#ifdef WIN32
		status = recvfrom(sock, (char *) buffer, sizeof(buffer), 0,(struct sockaddr*) &from, (int *) &fromlen);
#else
		status = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr*) &from, &fromlen);
#endif
		if (status >= 1) {
			cout << "Got data from recvfrom" << endl;
			RecvData(buffer, status, from.sin_addr.s_addr, from.sin_port);
		}
		else {
			break;
		}
	}

	std::map <std::string, EQStream*>::iterator connection;
	for (connection = connection_list.begin(); connection != connection_list.end();) {
		if (!connection->second) {
			connection = connection_list.erase(connection);
			continue;
		}
		EQStream* eqs_data = connection->second;
		if (eqs_data->IsFree() && (!eqs_data->CheckNetActive())) {
			safe_delete(eqs_data);
			connection = connection_list.erase(connection);
		} else if (!eqs_data->RunLoop) {
			eqs_data->Process(sock);
			++connection;
		}
	}
}

void EQStreamServer::RecvData(uchar* data, uint32 size, uint32 irIP, uint16 irPort) {
/*
	CHANGE HISTORY

	Version		Author		Date		Comment
	1			Unknown		Unknown		Initial Revision
	2			Joolz		05-Jan-2003	Optimised
	3			Quagmire	05-Feb-2003	Changed so 2 connection objects wouldnt be created for the same ip/port pair, often happened
*/

	// Check for invalid data
	if (!data || size <= 4) return;
	//if (CRC32::Generate(data, size-4) != ntohl(*((uint32*) &data[size-4]))) {
#if EQN_DEBUG_Error >= 1
		//cout << "Incomming Packet failed checksum" << endl;
#endif
		//return;
	//}

	char temp[25];
	sprintf(temp,"%lu:%u",(unsigned long)irIP,irPort);
	cout << "Data from " << temp << endl;
	EQStream* tmp = NULL;
	std::map <std::string, EQStream*>::iterator connection;
	if ((connection=connection_list.find(temp))!=connection_list.end())
		tmp=connection->second;
	if(tmp != NULL && tmp->GetrPort() == irPort)
	{
		tmp->RecvData(data, size);
		return;
	}
	else if(tmp != NULL && tmp->GetrPort() != irPort)
	{
		printf("Conflicting IPs & Ports: IP %i and Port %i is conflicting with IP %i and Port %i\n",irIP,irPort,tmp->GetrIP(),tmp->GetrPort());
		return;
	}

	if (data[1]==0x01) {
		cout << "New EQStream Connection." << endl;
		EQStream* tmp = new EQStream(irIP, irPort);
		tmp->RecvData(data, size);
		connection_list[temp]=tmp;
		if (connection_list.find(temp)==connection_list.end()) {
			cerr <<"Could not find new connection we just added!" << endl;
		}
		MNewQueue.lock();
		NewQueue.push(tmp);
		MNewQueue.unlock();
		return;
	}
#if EQN_DEBUG >= 4
	struct in_addr	in;
	in.s_addr = irIP;
	cout << "WARNING: Stray packet? " << inet_ntoa(in) << ":" << irPort << endl;
#endif
}

EQStream* EQStreamServer::NewQueuePop() {
	EQStream* ret = 0;
	MNewQueue.lock();
	if (!NewQueue.empty()) {
		ret = NewQueue.front();
		NewQueue.pop();
	}
	MNewQueue.unlock();
	return ret;
}

#ifdef WIN32
	void EQStreamServerLoop(void* tmp)
#else
	void* EQStreamServerLoop(void* tmp)
#endif
{
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
	EQStreamServer* eqns = (EQStreamServer*) tmp;
	eqns->MLoopRunning.lock();
	while (eqns->RunLoop) {
		{
			_CP(EQStreamServerLoop);
			eqns->Process();
		}
		Sleep(1);
	}
	eqns->MLoopRunning.unlock();
#ifdef WIN32
	_endthread();
#else
	return 0;
#endif
}

#ifdef WIN32
	void EQStreamInLoop(void* tmp)
#else
	void* EQStreamInLoop(void* tmp)
#endif
{
	EQStream* eqs = (EQStream*) tmp;
#ifdef _DEBUG
	if (eqs->ConnectionType != Outgoing) {
		ThrowError("EQStreamInLoop: eqs->ConnectionType != Outgoing");
	}
#endif
	eqs->MLoopRunning.lock();
	Timer* tmp_timer = new Timer(100);
	tmp_timer->Start();
	while (eqs->RunLoop) {
		{
			_CP(EQStreamInLoop);
			if(tmp_timer->Check())
				eqs->DoRecvData();
		}
		Sleep(1);
	}
	safe_delete(tmp_timer);
	eqs->MLoopRunning.unlock();
#ifdef WIN32
	_endthread();
#else
	return 0;
#endif
}

#ifdef WIN32
	void EQStreamOutLoop(void* tmp)
#else
	void* EQStreamOutLoop(void* tmp)
#endif
{
	EQStream* eqs = (EQStream*) tmp;
#ifdef _DEBUG
	if (eqs->ConnectionType != Outgoing) {
		ThrowError("EQStreamOutLoop: eqs->ConnectionType != Outgoing");
	}
#endif
	eqs->MLoopRunning.lock();
	Timer* tmp_timer = new Timer(100);
	tmp_timer->Start();
	while (eqs->RunLoop) {
		{
			_CP(EQStreamOutLoop);
			if(tmp_timer->Check())
				eqs->Process(eqs->outsock);
		}
		Sleep(1);
	}
	safe_delete(tmp_timer);
	eqs->MLoopRunning.unlock();
#ifdef WIN32
	_endthread();
#else
	return 0;
#endif
}

