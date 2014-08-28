#ifndef QueryServ_H
#define QueryServ_H

#include "../common/types.h"
#include "../common/emu_tcp_connection.h"
#include "../common/servertalk.h"

class QueryServConnection
{
public:
	QueryServConnection();
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

#endif /*QueryServ_H_*/
