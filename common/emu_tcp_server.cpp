#include "global_define.h"
#include "emu_tcp_server.h"
#include "emu_tcp_connection.h"

EmuTCPServer::EmuTCPServer(uint16 iPort, bool iOldFormat)
:	TCPServer<EmuTCPConnection>(iPort),
	pOldFormat(iOldFormat)
{
}

EmuTCPServer::~EmuTCPServer() {
	_eqp
	MInQueue.lock();
	while(!m_InQueue.empty()) {
		delete m_InQueue.front();
		m_InQueue.pop();
	}
	MInQueue.unlock();
}

void EmuTCPServer::Process() {
	_eqp
	CheckInQueue();
	TCPServer<EmuTCPConnection>::Process();
}

void EmuTCPServer::CreateNewConnection(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort)
{
	_eqp
	EmuTCPConnection *conn = new EmuTCPConnection(ID, this, in_socket, irIP, irPort, pOldFormat);
	AddConnection(conn);
}


void EmuTCPServer::SendPacket(ServerPacket* pack) {
	_eqp
	EmuTCPNetPacket_Struct* tnps = EmuTCPConnection::MakePacket(pack);
	SendPacket(&tnps);
}

void EmuTCPServer::SendPacket(EmuTCPNetPacket_Struct** tnps) {
	_eqp
	MInQueue.lock();
	m_InQueue.push(*tnps);
	MInQueue.unlock();
	tnps = nullptr;
}

void EmuTCPServer::CheckInQueue() {
	_eqp
	EmuTCPNetPacket_Struct* tnps = 0;

	while (( tnps = InQueuePop() )) {
		vitr cur, end;
		cur = m_list.begin();
		end = m_list.end();
		for(; cur != end; cur++) {
			if ((*cur)->GetMode() != EmuTCPConnection::modeConsole && (*cur)->GetRemoteID() == 0)
				(*cur)->SendPacket(tnps);
		}
		safe_delete(tnps);
	}
}

EmuTCPNetPacket_Struct* EmuTCPServer::InQueuePop() {
	_eqp
	EmuTCPNetPacket_Struct* ret = nullptr;
	MInQueue.lock();
	if(!m_InQueue.empty()) {
		ret = m_InQueue.front();
		m_InQueue.pop();
	}
	MInQueue.unlock();
	return ret;
}


EmuTCPConnection *EmuTCPServer::FindConnection(uint32 iID) {
	_eqp
	vitr cur, end;
	cur = m_list.begin();
	end = m_list.end();
	for(; cur != end; cur++) {
		if ((*cur)->GetID() == iID)
			return *cur;
	}
	return(nullptr);
}

