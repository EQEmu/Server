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
#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H
/*
	Parent classes for interserver TCP Communication.
	-Quagmire
*/

#ifdef _WINDOWS
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp

	#include <process.h>
#else
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#include "unix.h"

#endif

#include "types.h"
#include "mutex.h"
#include "queue.h"
#include "misc_functions.h"


#define TCPConnection_ErrorBufferSize	1024
#define MaxTCPReceiveBuffferSize		524288


#ifndef DEF_eConnectionType
#define DEF_eConnectionType
enum eConnectionType {Incoming, Outgoing};
#endif


class TCPConnection {
protected:
	typedef enum {
		TCPS_Ready = 0,
		TCPS_Connecting = 1,
		TCPS_Connected = 100,
		TCPS_Disconnecting = 200,	//I do not know the difference between Disconnecting and Closing
		TCPS_Disconnected = 201,
		TCPS_Closing = 250,
		TCPS_Error = 255
	} State_t;

public:
	//socket created by a server (incoming)
	TCPConnection(uint32 ID, SOCKET iSock, uint32 irIP, uint16 irPort);
	//socket created to connect to a server (outgoing)
	TCPConnection();	// for outgoing connections

	virtual ~TCPConnection();

	// Functions for outgoing connections
	bool			Connect(const char* irAddress, uint16 irPort, char* errbuf = 0);
	virtual bool	ConnectIP(uint32 irIP, uint16 irPort, char* errbuf = 0);
	void			AsyncConnect(const char* irAddress, uint16 irPort);
	void			AsyncConnect(uint32 irIP, uint16 irPort);
	virtual void	Disconnect();

	bool			Send(const uchar* data, int32 size);

	char*			PopLine();		//returns ownership of allocated byte array
	inline uint32	GetrIP()	const		{ return rIP; }
	inline uint16	GetrPort()		const	{ return rPort; }
	virtual State_t	GetState() const;
	inline bool		Connected()	const	{ return (GetState() == TCPS_Connected); }
	bool		ConnectReady() const;
	void			Free();		// Inform TCPServer that this connection object is no longer referanced

	inline uint32	GetID()	const		{ return id; }

	bool			GetEcho();
	void			SetEcho(bool iValue);
	bool GetSockName(char *host, uint16 *port);

	//should only be used by TCPServer<T>:
	bool			CheckNetActive();
	inline bool		IsFree() const { return pFree; }
	virtual bool	Process();

protected:
	friend class BaseTCPServer;
	void			SetState(State_t iState);

	static ThreadReturnType TCPConnectionLoop(void* tmp);
//	SOCKET			sock;
	bool			RunLoop();
	Mutex			MLoopRunning;
	Mutex	MAsyncConnect;
	bool	GetAsyncConnect();
	bool	SetAsyncConnect(bool iValue);
	char*	charAsyncConnect;
	bool	pAsyncConnect;	//this flag should really be turned into a state instead.

	virtual bool ProcessReceivedData(char* errbuf = 0);
	virtual bool SendData(bool &sent_something, char* errbuf = 0);
	virtual bool RecvData(char* errbuf = 0);

	virtual void ClearBuffers();


	bool m_previousLineEnd;

	eConnectionType	ConnectionType;
	Mutex	MRunLoop;
	bool	pRunLoop;

	SOCKET	connection_socket;
	uint32	id;
	uint32	rIP;
	uint16	rPort; // host byte order
	bool	pFree;

	mutable Mutex	MState;
	State_t	pState;

	//text based line out queue.
	Mutex MLineOutQueue;
	virtual bool	LineOutQueuePush(char* line);	//this is really kinda a hack for the transition to packet mode. Returns true to stop processing the output.
	MyQueue<char> LineOutQueue;

	uchar*	recvbuf;
	int32	recvbuf_size;
	int32	recvbuf_used;

	int32	recvbuf_echo;
	volatile bool	pEcho;

	Mutex	MSendQueue;
	uchar*	sendbuf;
	int32	sendbuf_size;
	int32	sendbuf_used;
	bool	ServerSendQueuePop(uchar** data, int32* size);
	bool	ServerSendQueuePopForce(uchar** data, int32* size);		//does a lock() instead of a trylock()
	void	ServerSendQueuePushEnd(const uchar* data, int32 size);
	void	ServerSendQueuePushEnd(uchar** data, int32 size);
	void	ServerSendQueuePushFront(uchar* data, int32 size);

private:
	void FinishDisconnect();
};


#endif

