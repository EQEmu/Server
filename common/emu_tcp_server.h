#ifndef EmuTCPSERVER_H_
#define EmuTCPSERVER_H_

#include "TCPServer.h"

class EmuTCPConnection;
struct EmuTCPNetPacket_Struct;
class ServerPacket;

class EmuTCPServer : public TCPServer<EmuTCPConnection> {
public:
	EmuTCPServer(uint16 iPort = 0, bool iOldFormat = false);
	virtual ~EmuTCPServer();

	//packet broadcast routines.
	void	SendPacket(ServerPacket* pack);
	void	SendPacket(EmuTCPNetPacket_Struct** tnps);

	//special crap for relay management
	EmuTCPConnection *FindConnection(uint32 iID);

	//exposed for some crap we pull. Do not call from outside this object.
	using TCPServer<EmuTCPConnection>::AddConnection;

protected:
	virtual void	Process();

	virtual void CreateNewConnection(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort);

	bool pOldFormat;

	//broadcast packet queue..
	void	CheckInQueue();
	Mutex	MInQueue;
	EmuTCPNetPacket_Struct*	InQueuePop();	//returns ownership
	std::queue<EmuTCPNetPacket_Struct *> m_InQueue;
};
#endif /*EmuTCPSERVER_H_*/
