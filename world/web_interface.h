#ifndef WORLD_WEB_INTERFACE_H
#define WORLD_WEB_INTERFACE_H

#include "../common/types.h"
#include "../common/emu_tcp_connection.h"
#include "../common/servertalk.h"

class WebInterfaceConnection
{
public:
	WebInterfaceConnection();
	void SetConnection(EmuTCPConnection *inStream);
	bool Process();
	bool SendPacket(ServerPacket* pack);
	void Disconnect() { if(stream) stream->Disconnect(); }
	void SendMessage(const char *From, const char *Message);
private:
	inline uint32 GetIP() const { return stream ? stream->GetrIP() : 0; }
	EmuTCPConnection *stream;
	bool authenticated;
};

#endif /*WORLD_WEB_INTERFACE_H*/
