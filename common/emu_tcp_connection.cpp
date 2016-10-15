/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
There are really two or three different objects shoe-hored into this
connection object. Sombody really needs to factor out the relay link
crap into its own subclass of this object, it will clean things up
tremendously.
*/

#include "../common/global_define.h"

#include <iostream>
#include <string.h>

#include "emu_tcp_connection.h"
#include "emu_tcp_server.h"
#include "../common/servertalk.h"

#ifdef FREEBSD //Timothy Whitman - January 7, 2003
	#define MSG_NOSIGNAL 0
#endif

#define TCPN_DEBUG				0
#define TCPN_DEBUG_Console		0
#define TCPN_DEBUG_Memory		0
#define TCPN_LOG_PACKETS		0
#define TCPN_LOG_RAW_DATA_OUT	0
#define TCPN_LOG_RAW_DATA_IN	0


//server side case
EmuTCPConnection::EmuTCPConnection(uint32 ID, EmuTCPServer* iServer, SOCKET in_socket, uint32 irIP, uint16 irPort, bool iOldFormat)
:	TCPConnection(ID, in_socket, irIP, irPort),
	keepalive_timer(SERVER_TIMEOUT),
	timeout_timer(SERVER_TIMEOUT * 2)
{
	id = 0;
	Server = nullptr;
	pOldFormat = iOldFormat;
	#ifdef MINILOGIN
		TCPMode = modePacket;
		PacketMode = packetModeLogin;
	#else
		if (pOldFormat)
			TCPMode = modePacket;
		else
			TCPMode = modeConsole;
		PacketMode = packetModeZone;
	#endif
	RelayLink = 0;
	RelayServer = false;
	RelayCount = 0;
	RemoteID = 0;

}

//client outgoing connection case (and client side relay)
EmuTCPConnection::EmuTCPConnection(bool iOldFormat, EmuTCPServer* iRelayServer, eTCPMode iMode)
:	TCPConnection(),
	keepalive_timer(SERVER_TIMEOUT),
	timeout_timer(SERVER_TIMEOUT * 2)
{
	Server = iRelayServer;
	if (Server)
		RelayServer = true;
	else
		RelayServer = false;
	RelayLink = 0;
	RelayCount = 0;
	RemoteID = 0;
	pOldFormat = iOldFormat;
	TCPMode = iMode;
	PacketMode = packetModeZone;
#if TCPN_DEBUG_Memory >= 7
	std::cout << "Constructor #1 on outgoing TCP# " << GetID() << std::endl;
#endif
}

//server side relay case
EmuTCPConnection::EmuTCPConnection(uint32 ID, EmuTCPServer* iServer, EmuTCPConnection* iRelayLink, uint32 iRemoteID, uint32 irIP, uint16 irPort)
:	TCPConnection(ID, 0, irIP, irPort),
	keepalive_timer(SERVER_TIMEOUT),
	timeout_timer(SERVER_TIMEOUT * 2)
{
	Server = iServer;
	RelayLink = iRelayLink;
	RelayServer = true;
	RelayCount = 0;
	RemoteID = iRemoteID;
	pOldFormat = false;
	ConnectionType = Incoming;
	TCPMode = modePacket;
	PacketMode = packetModeZone;
#if TCPN_DEBUG_Memory >= 7
	std::cout << "Constructor #3 on outgoing TCP# " << GetID() << std::endl;
#endif
}

EmuTCPConnection::~EmuTCPConnection() {
	//the queues free their content right now I believe.
}

EmuTCPNetPacket_Struct* EmuTCPConnection::MakePacket(ServerPacket* pack, uint32 iDestination) {
	int32 size = sizeof(EmuTCPNetPacket_Struct) + pack->size;
	if (pack->compressed) {
		size += 4;
	}
	if (iDestination) {
		size += 4;
	}
	EmuTCPNetPacket_Struct* tnps = (EmuTCPNetPacket_Struct*) new uchar[size];
	tnps->size = size;
	tnps->opcode = pack->opcode;
	*((uint8*) &tnps->flags) = 0;
	uchar* buffer = tnps->buffer;
	if (pack->compressed) {
		tnps->flags.compressed = 1;
		*((int32*) buffer) = pack->InflatedSize;
		buffer += 4;
	}
	if (iDestination) {
		tnps->flags.destination = 1;
		*((int32*) buffer) = iDestination;
		buffer += 4;
	}
	memcpy(buffer, pack->pBuffer, pack->size);
	return tnps;
}

SPackSendQueue* EmuTCPConnection::MakeOldPacket(ServerPacket* pack) {
	SPackSendQueue* spsq = (SPackSendQueue*) new uchar[sizeof(SPackSendQueue) + pack->size + 4];
	if (pack->pBuffer != 0 && pack->size != 0)
		memcpy((char *) &spsq->buffer[4], (char *) pack->pBuffer, pack->size);
	memcpy((char *) &spsq->buffer[0], (char *) &pack->opcode, 2);
	spsq->size = pack->size+4;
	memcpy((char *) &spsq->buffer[2], (char *) &spsq->size, 2);
	return spsq;
}

bool EmuTCPConnection::SendPacket(ServerPacket* pack, uint32 iDestination) {
	if (!Connected())
		return false;
	eTCPMode tmp = GetMode();
	if (tmp != modePacket && tmp != modeTransition)
		return false;
	LockMutex lock(&MState);
	if (RemoteID)
		return RelayLink->SendPacket(pack, RemoteID);
	else if (pOldFormat) {
		#if TCPN_LOG_PACKETS >= 1
			if (pack && pack->opcode != 0) {
				struct in_addr	in;
				in.s_addr = GetrIP();
				CoutTimestamp(true);
				std::cout << ": Logging outgoing TCP OldPacket. OPCode: 0x" << std::hex << std::setw(4) << std::setfill('0') << pack->opcode << std::dec << ", size: " << std::setw(5) << std::setfill(' ') << pack->size << " " << inet_ntoa(in) << ":" << GetrPort() << std::endl;
				#if TCPN_LOG_PACKETS == 2
					if (pack->size >= 32)
						DumpPacket(pack->pBuffer, 32);
					else
						DumpPacket(pack);
				#endif
				#if TCPN_LOG_PACKETS >= 3
					DumpPacket(pack);
				#endif
			}
		#endif
		SPackSendQueue* spsq = MakeOldPacket(pack);
		ServerSendQueuePushEnd(spsq->buffer, spsq->size);
		safe_delete_array(spsq);
	}
	else {
		EmuTCPNetPacket_Struct* tnps = MakePacket(pack, iDestination);
		if (tmp == modeTransition) {
			InModeQueuePush(tnps);
		}
		else {
			#if TCPN_LOG_PACKETS >= 1
				if (pack && pack->opcode != 0) {
					struct in_addr	in;
					in.s_addr = GetrIP();
					CoutTimestamp(true);
					std::cout << ": Logging outgoing TCP packet. OPCode: 0x" << std::hex << std::setw(4) << std::setfill('0') << pack->opcode << std::dec << ", size: " << std::setw(5) << std::setfill(' ') << pack->size << " " << inet_ntoa(in) << ":" << GetrPort() << std::endl;
					#if TCPN_LOG_PACKETS == 2
						if (pack->size >= 32)
							DumpPacket(pack->pBuffer, 32);
						else
							DumpPacket(pack);
					#endif
					#if TCPN_LOG_PACKETS >= 3
						DumpPacket(pack);
					#endif
				}
			#endif
			ServerSendQueuePushEnd((uchar**) &tnps, tnps->size);
		}
	}
	return true;
}

bool EmuTCPConnection::SendPacket(EmuTCPNetPacket_Struct* tnps) {
	if (RemoteID)
		return false;
	if (!Connected())
		return false;
	if (GetMode() != modePacket)
		return false;

	LockMutex lock(&MState);
	eTCPMode tmp = GetMode();
	if (tmp == modeTransition) {
		EmuTCPNetPacket_Struct* tnps2 = (EmuTCPNetPacket_Struct*) new uchar[tnps->size];
		memcpy(tnps2, tnps, tnps->size);
		InModeQueuePush(tnps2);
		return true;
	}
	#if TCPN_LOG_PACKETS >= 1
		if (tnps && tnps->opcode != 0) {
			struct in_addr	in;
			in.s_addr = GetrIP();
			CoutTimestamp(true);
			std::cout << ": Logging outgoing TCP NetPacket. OPCode: 0x" << std::hex << std::setw(4) << std::setfill('0') << tnps->opcode << std::dec << ", size: " << std::setw(5) << std::setfill(' ') << tnps->size << " " << inet_ntoa(in) << ":" << GetrPort();
			if (pOldFormat)
				std::cout << " (OldFormat)";
			std::cout << std::endl;
			#if TCPN_LOG_PACKETS == 2
				if (tnps->size >= 32)
					DumpPacket((uchar*) tnps, 32);
				else
					DumpPacket((uchar*) tnps, tnps->size);
			#endif
			#if TCPN_LOG_PACKETS >= 3
				DumpPacket((uchar*) tnps, tnps->size);
			#endif
		}
	#endif
	ServerSendQueuePushEnd((const uchar*) tnps, tnps->size);
	return true;
}

ServerPacket* EmuTCPConnection::PopPacket() {
	ServerPacket* ret;
	if (!MOutQueueLock.trylock())
		return nullptr;
	ret = OutQueue.pop();
	MOutQueueLock.unlock();
	return ret;
}

void EmuTCPConnection::InModeQueuePush(EmuTCPNetPacket_Struct* tnps) {
	MSendQueue.lock();
	InModeQueue.push(tnps);
	MSendQueue.unlock();
}

void EmuTCPConnection::OutQueuePush(ServerPacket* pack) {
	MOutQueueLock.lock();
	OutQueue.push(pack);
	MOutQueueLock.unlock();
}


bool EmuTCPConnection::LineOutQueuePush(char* line) {
	#if defined(GOTFRAGS) && 0
		if (strcmp(line, "**CRASHME**") == 0) {
			int i = 0;
			std::cout << (5 / i) << std::endl;
		}
	#endif
	if(line[0] == '*') {
		if (strcmp(line, "**PACKETMODE**") == 0) {
			MSendQueue.lock();
			safe_delete_array(sendbuf);
			if (TCPMode == modeConsole)
				Send((const uchar*) "\0**PACKETMODE**\r", 16);
			TCPMode = modePacket;
			PacketMode = packetModeLogin;
			EmuTCPNetPacket_Struct* tnps = 0;
			while ((tnps = InModeQueue.pop())) {
				SendPacket(tnps);
				safe_delete_array(tnps);
			}
			MSendQueue.unlock();
			safe_delete_array(line);
			return(true);
		}
		if (strcmp(line, "**PACKETMODEZONE**") == 0) {
			MSendQueue.lock();
			safe_delete_array(sendbuf);
			if (TCPMode == modeConsole)
				Send((const uchar*) "\0**PACKETMODEZONE**\r", 20);
			TCPMode = modePacket;
			PacketMode = packetModeZone;
			EmuTCPNetPacket_Struct* tnps = 0;
			while ((tnps = InModeQueue.pop())) {
				SendPacket(tnps);
				safe_delete_array(tnps);
			}
			MSendQueue.unlock();
			safe_delete_array(line);
			return(true);
		}
		if (strcmp(line, "**PACKETMODELAUNCHER**") == 0) {
			MSendQueue.lock();
			safe_delete_array(sendbuf);
			if (TCPMode == modeConsole)
				Send((const uchar*) "\0**PACKETMODELAUNCHER**\r", 24);
			TCPMode = modePacket;
			PacketMode = packetModeLauncher;
			EmuTCPNetPacket_Struct* tnps = 0;
			while ((tnps = InModeQueue.pop())) {
				SendPacket(tnps);
				safe_delete_array(tnps);
			}
			MSendQueue.unlock();
			safe_delete_array(line);
			return(true);
		}
		if (strcmp(line, "**PACKETMODEUCS**") == 0) {
			MSendQueue.lock();
			safe_delete_array(sendbuf);
			if (TCPMode == modeConsole)
				Send((const uchar*) "\0**PACKETMODEUCS**\r", 19);
			TCPMode = modePacket;
			PacketMode = packetModeUCS;
			EmuTCPNetPacket_Struct* tnps = 0;
			while ((tnps = InModeQueue.pop())) {
				SendPacket(tnps);
				safe_delete_array(tnps);
			}
			MSendQueue.unlock();
			safe_delete_array(line);
			return(true);
		}
		if (strcmp(line, "**PACKETMODEQS**") == 0) {
			MSendQueue.lock();
			safe_delete_array(sendbuf);
			if (TCPMode == modeConsole)
				Send((const uchar*) "\0**PACKETMODEQS**\r", 18);
			TCPMode = modePacket;
			PacketMode = packetModeQueryServ;
			EmuTCPNetPacket_Struct* tnps = 0;
			while ((tnps = InModeQueue.pop())) {
				SendPacket(tnps);
				safe_delete_array(tnps);
			}
			MSendQueue.unlock();
			safe_delete_array(line);
			return(true);
		}
	}

	return(TCPConnection::LineOutQueuePush(line));
}

void EmuTCPConnection::Disconnect(bool iSendRelayDisconnect) {
	TCPConnection::Disconnect();

	if (RelayLink) {
		RelayLink->RemoveRelay(this, iSendRelayDisconnect);
		RelayLink = 0;
	}
}

bool EmuTCPConnection::ConnectIP(uint32 irIP, uint16 irPort, char* errbuf) {
	if(!TCPConnection::ConnectIP(irIP, irPort, errbuf))
		return(false);

	MSendQueue.lock();
	#ifdef MINILOGIN
		TCPMode = modePacket;
	#else
		if (pOldFormat) {
			TCPMode = modePacket;
		}
		else if (TCPMode == modePacket || TCPMode == modeTransition) {
			TCPMode = modeTransition;
			if(PacketMode == packetModeLauncher) {
				safe_delete_array(sendbuf);
				sendbuf_size = 24;
				sendbuf_used = sendbuf_size;
				sendbuf = new uchar[sendbuf_size];
				memcpy(sendbuf, "\0**PACKETMODELAUNCHER**\r", sendbuf_size);
			} else if(PacketMode == packetModeLogin) {
				safe_delete_array(sendbuf);
				sendbuf_size = 16;
				sendbuf_used = sendbuf_size;
				sendbuf = new uchar[sendbuf_size];
				memcpy(sendbuf, "\0**PACKETMODE**\r", sendbuf_size);
			} else if(PacketMode == packetModeUCS) {
				safe_delete_array(sendbuf);
				sendbuf_size = 19;
				sendbuf_used = sendbuf_size;
				sendbuf = new uchar[sendbuf_size];
				memcpy(sendbuf, "\0**PACKETMODEUCS**\r", sendbuf_size);
			}
			else if(PacketMode == packetModeQueryServ) {
				safe_delete_array(sendbuf);
				sendbuf_size = 18;
				sendbuf_used = sendbuf_size;
				sendbuf = new uchar[sendbuf_size];
				memcpy(sendbuf, "\0**PACKETMODEQS**\r", sendbuf_size);
			} 
			else {
				//default: packetModeZone
				safe_delete_array(sendbuf);
				sendbuf_size = 20;
				sendbuf_used = sendbuf_size;
				sendbuf = new uchar[sendbuf_size];
				memcpy(sendbuf, "\0**PACKETMODEZONE**\r", sendbuf_size);
			}
		}
	#endif
	MSendQueue.unlock();

	return(true);
}

void EmuTCPConnection::ClearBuffers() {
	TCPConnection::ClearBuffers();

	LockMutex lock2(&MOutQueueLock);
	ServerPacket* pack = 0;
	while ((pack = OutQueue.pop()))
		safe_delete(pack);

	EmuTCPNetPacket_Struct* tnps = 0;
	while ((tnps = InModeQueue.pop()))
		safe_delete(tnps);

	keepalive_timer.Start();
	timeout_timer.Start();
}

void EmuTCPConnection::SendNetErrorPacket(const char* reason) {
	#if TCPC_DEBUG >= 1
		struct in_addr	in;
		in.s_addr = GetrIP();
		std::cout "NetError: '";
		if (reason)
			std::cout << reason;
		std::cout << "': " << inet_ntoa(in) << ":" << GetPort() << std::endl;
	#endif
		auto pack = new ServerPacket(0);
		pack->size = 1;
		if (reason)
			pack->size += strlen(reason) + 1;
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		pack->pBuffer[0] = 255;
		strcpy((char *)&pack->pBuffer[1], reason);
		SendPacket(pack);
		safe_delete(pack);
}

void EmuTCPConnection::RemoveRelay(EmuTCPConnection* relay, bool iSendRelayDisconnect) {
	if (iSendRelayDisconnect) {
		auto pack = new ServerPacket(0, 5);
		pack->pBuffer[0] = 3;
		*((uint32*) &pack->pBuffer[1]) = relay->GetRemoteID();
		SendPacket(pack);
		safe_delete(pack);
	}
	RelayCount--;
}



bool EmuTCPConnection::ProcessReceivedData(char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	timeout_timer.Start();
	if (!recvbuf)
		return true;
	if (TCPMode == modePacket) {
		if (pOldFormat)
			return ProcessReceivedDataAsOldPackets(errbuf);
		else
			return ProcessReceivedDataAsPackets(errbuf);
	}
	//else, use the base class's text processing.
	bool ret = TCPConnection::ProcessReceivedData(errbuf);
	//see if we made the transition to packet mode...
	if(ret && TCPMode == modePacket) {
		return ProcessReceivedDataAsPackets(errbuf);
	}
	return(ret);
}



bool EmuTCPConnection::ProcessReceivedDataAsPackets(char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	int32 base = 0;
	int32 size = 7;
	uchar* buffer;
	ServerPacket* pack = 0;
	while ((recvbuf_used - base) >= size) {
		EmuTCPNetPacket_Struct* tnps = (EmuTCPNetPacket_Struct*) &recvbuf[base];
		buffer = tnps->buffer;
		size = tnps->size;
		if (size >= MaxTCPReceiveBuffferSize) {
#if TCPN_DEBUG_Memory >= 1
			std::cout << "TCPConnection[" << GetID() << "]::ProcessReceivedDataAsPackets(): size[" << size << "] >= MaxTCPReceiveBuffferSize" << std::endl;
			DumpPacket(&recvbuf[base], 16);
#endif
			if (errbuf)
				snprintf(errbuf, TCPConnection_ErrorBufferSize, "EmuTCPConnection::ProcessReceivedDataAsPackets(): size >= MaxTCPReceiveBuffferSize");
			return false;
		}
		if ((recvbuf_used - base) >= size) {
			// ok, we got enough data to make this packet!
			pack = new ServerPacket;
			pack->size = size - sizeof(EmuTCPNetPacket_Struct);
			// read headers
			pack->opcode = tnps->opcode;
			if (tnps->flags.compressed) {
				pack->compressed = true;
				pack->InflatedSize = *((int32*)buffer);
				pack->size -= 4;
				buffer += 4;
			}
			if (tnps->flags.destination) {
				pack->destination = *((int32*)buffer);
				pack->size -= 4;
				buffer += 4;
			}
			// end read headers
			if (pack->size > 0) {
				if (tnps->flags.compressed) {
					// Lets decompress the packet here
					pack->compressed = false;
					pack->pBuffer = new uchar[pack->InflatedSize];
					pack->size = InflatePacket(buffer, pack->size, pack->pBuffer, pack->InflatedSize);
				}
				else {
					pack->pBuffer = new uchar[pack->size];
					memcpy(pack->pBuffer, buffer, pack->size);
				}
			}
			if (pack->opcode == 0) {
				if (pack->size) {
					#if TCPN_DEBUG >= 2
						std::cout << "Received TCP Network layer packet" << std::endl;
					#endif
					ProcessNetworkLayerPacket(pack);
				}
				#if TCPN_DEBUG >= 5
					else {
						std::cout << "Received TCP keepalive packet. (opcode=0)" << std::endl;
					}
				#endif
				// keepalive, no need to process
				safe_delete(pack);
			}
			else {
				#if TCPN_LOG_PACKETS >= 1
					if (pack && pack->opcode != 0) {
						struct in_addr	in;
						in.s_addr = GetrIP();
						CoutTimestamp(true);
						std::cout << ": Logging incoming TCP packet. OPCode: 0x" << std::hex << std::setw(4) << std::setfill('0') << pack->opcode << std::dec << ", size: " << std::setw(5) << std::setfill(' ') << pack->size << " " << inet_ntoa(in) << ":" << GetrPort() << std::endl;
						#if TCPN_LOG_PACKETS == 2
							if (pack->size >= 32)
								DumpPacket(pack->pBuffer, 32);
							else
								DumpPacket(pack);
						#endif
						#if TCPN_LOG_PACKETS >= 3
							DumpPacket(pack);
						#endif
					}
				#endif
				if (RelayServer && Server && pack->destination) {
					EmuTCPConnection* con = Server->FindConnection(pack->destination);
					if (!con) {
						#if TCPN_DEBUG >= 1
							std::cout << "Error relaying packet: con = 0" << std::endl;
						#endif
						safe_delete(pack);
					}
					else
						con->OutQueuePush(pack);
				}
				else
					OutQueuePush(pack);
			}
			base += size;
			size = 7;
		}
	}
	if (base != 0) {
		if (base >= recvbuf_used) {
			safe_delete_array(recvbuf);
		} else {
			auto tmpbuf = new uchar[recvbuf_size - base];
			memcpy(tmpbuf, &recvbuf[base], recvbuf_used - base);
			safe_delete_array(recvbuf);
			recvbuf = tmpbuf;
			recvbuf_used -= base;
			recvbuf_size -= base;
		}
	}
	return true;
}

bool EmuTCPConnection::ProcessReceivedDataAsOldPackets(char* errbuf) {
	int32 base = 0;
	int32 size = 4;
	uchar* buffer;
	ServerPacket* pack = 0;
	while ((recvbuf_used - base) >= size) {
		buffer = &recvbuf[base];
		memcpy(&size, &buffer[2], 2);
		if (size >= MaxTCPReceiveBuffferSize) {
#if TCPN_DEBUG_Memory >= 1
			std::cout << "TCPConnection[" << GetID() << "]::ProcessReceivedDataAsPackets(): size[" << size << "] >= MaxTCPReceiveBuffferSize" << std::endl;
#endif
			if (errbuf)
				snprintf(errbuf, TCPConnection_ErrorBufferSize, "EmuTCPConnection::ProcessReceivedDataAsPackets(): size >= MaxTCPReceiveBuffferSize");
			return false;
		}
		if ((recvbuf_used - base) >= size) {
			// ok, we got enough data to make this packet!
			pack = new ServerPacket;
			memcpy(&pack->opcode, &buffer[0], 2);
			pack->size = size - 4;
/*			if () { // TODO: Checksum or size check or something similar
				// Datastream corruption, get the hell outta here!
				delete pack;
				return false;
			}*/
			if (pack->size > 0) {
				pack->pBuffer = new uchar[pack->size];
				memcpy(pack->pBuffer, &buffer[4], pack->size);
			}
			if (pack->opcode == 0) {
				// keepalive, no need to process
				safe_delete(pack);
			}
			else {
				#if TCPN_LOG_PACKETS >= 1
					if (pack && pack->opcode != 0) {
						struct in_addr	in;
						in.s_addr = GetrIP();
						CoutTimestamp(true);
						std::cout << ": Logging incoming TCP OldPacket. OPCode: 0x" << std::hex << std::setw(4) << std::setfill('0') << pack->opcode << std::dec << ", size: " << std::setw(5) << std::setfill(' ') << pack->size << " " << inet_ntoa(in) << ":" << GetrPort() << std::endl;
						#if TCPN_LOG_PACKETS == 2
							if (pack->size >= 32)
								DumpPacket(pack->pBuffer, 32);
							else
								DumpPacket(pack);
						#endif
						#if TCPN_LOG_PACKETS >= 3
							DumpPacket(pack);
						#endif
					}
				#endif
				OutQueuePush(pack);
			}
			base += size;
			size = 4;
		}
	}
	if (base != 0) {
		if (base >= recvbuf_used) {
			safe_delete_array(recvbuf);
		}
		else {
			auto tmpbuf = new uchar[recvbuf_size - base];
			memcpy(tmpbuf, &recvbuf[base], recvbuf_used - base);
			safe_delete_array(recvbuf);
			recvbuf = tmpbuf;
			recvbuf_used -= base;
			recvbuf_size -= base;
		}
	}
	return true;
}

void EmuTCPConnection::ProcessNetworkLayerPacket(ServerPacket* pack) {
	uint8 opcode = pack->pBuffer[0];
	uint8* data = &pack->pBuffer[1];
	switch (opcode) {
		case 0: {
			break;
		}
		case 1: { // Switch to RelayServer mode
			if (pack->size != 1) {
				SendNetErrorPacket("New RelayClient: wrong size, expected 1");
				break;
			}
			if (RelayServer) {
				SendNetErrorPacket("Switch to RelayServer mode when already in RelayServer mode");
				break;
			}
			if (RemoteID) {
				SendNetErrorPacket("Switch to RelayServer mode by a Relay Client");
				break;
			}
			if (ConnectionType != Incoming) {
				SendNetErrorPacket("Switch to RelayServer mode on outgoing connection");
				break;
			}
			#if TCPC_DEBUG >= 3
				struct in_addr	in;
				in.s_addr = GetrIP();
				std::cout << "Switching to RelayServer mode: " << inet_ntoa(in) << ":" << GetPort() << std::endl;
			#endif
			RelayServer = true;
			break;
		}
		case 2: { // New Relay Client
			if (!RelayServer) {
				SendNetErrorPacket("New RelayClient when not in RelayServer mode");
				break;
			}
			if (pack->size != 11) {
				SendNetErrorPacket("New RelayClient: wrong size, expected 11");
				break;
			}
			if (ConnectionType != Incoming) {
				SendNetErrorPacket("New RelayClient: illegal on outgoing connection");
				break;
			}
			auto con = new EmuTCPConnection(Server->GetNextID(), Server, this, *((uint32 *)data),
							*((uint32 *)&data[4]), *((uint16 *)&data[8]));
			Server->AddConnection(con);
			RelayCount++;
			break;
		}
		case 3: { // Delete Relay Client
			if (!RelayServer) {
				SendNetErrorPacket("Delete RelayClient when not in RelayServer mode");
				break;
			}
			if (pack->size != 5) {
				SendNetErrorPacket("Delete RelayClient: wrong size, expected 5");
				break;
			}
			EmuTCPConnection* con = Server->FindConnection(*((uint32*)data));
			if (con) {
				if (ConnectionType == Incoming) {
					if (con->GetRelayLink() != this) {
						SendNetErrorPacket("Delete RelayClient: RelayLink != this");
						break;
					}
				}
				con->Disconnect(false);
			}
			break;
		}
		case 255: {
			#if TCPC_DEBUG >= 1
				struct in_addr	in;
				in.s_addr = GetrIP();
				std::cout "Received NetError: '";
				if (pack->size > 1)
					std::cout << (char*) data;
				std::cout << "': " << inet_ntoa(in) << ":" << GetPort() << std::endl;
			#endif
			break;
		}
	}
}

bool EmuTCPConnection::SendData(bool &sent_something, char* errbuf) {
	sent_something = false;
	if(!TCPConnection::SendData(sent_something, errbuf))
		return(false);

	if(sent_something)
		keepalive_timer.Start();
	else if (TCPMode == modePacket && keepalive_timer.Check()) {
		auto pack = new ServerPacket(0, 0);
		SendPacket(pack);
		safe_delete(pack);
		#if TCPN_DEBUG >= 5
		std::cout << "Sending TCP keepalive packet. (timeout=" << timeout_timer.GetRemainingTime() << " remaining)" << std::endl;
		#endif
	}

	return(true);
}

bool EmuTCPConnection::RecvData(char* errbuf) {
	if(!TCPConnection::RecvData(errbuf)) {
		if (OutQueue.count())
			return(true);
		else
			return(false);
	}

	if ((TCPMode == modePacket || TCPMode == modeTransition) && timeout_timer.Check()) {
		if (errbuf)
			snprintf(errbuf, TCPConnection_ErrorBufferSize, "TCPConnection::RecvData(): Connection timeout");
		return false;
	}

	return(true);
}

