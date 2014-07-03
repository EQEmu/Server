#ifndef Socket_Server__H
#define Socket_Server__H

#include "../common/types.h"
#include "../common/EmuTCPConnection.h"
#include "../common/servertalk.h"

class Socket_Server_Connection
{
public:
	Socket_Server_Connection();
	void SetConnection(EmuTCPConnection *inStream);
	bool Process();
	bool SendPacket(ServerPacket* pack);
	void Disconnect() { if(Stream) Stream->Disconnect(); }
	void SendMessage(const char *From, const char *Message);
private:
	inline uint32 GetIP() const { return Stream ? Stream->GetrIP() : 0; }
	EmuTCPConnection *Stream;
	bool authenticated;
};

#endif /*Socket_Server__H_*/
