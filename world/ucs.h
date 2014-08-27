#ifndef UCS_H
#define UCS_H

#include "../common/types.h"
#include "../common/emu_tcp_connection.h"
#include "../common/servertalk.h"

class UCSConnection
{
public:
	UCSConnection();
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

#endif /*UCS_H_*/
