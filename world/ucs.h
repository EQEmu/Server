#ifndef UCS_H
#define UCS_H

#include "../common/types.h"
#include "../common/net/servertalk_server_connection.h"
#include "../common/servertalk.h"
#include <memory>

class UCSConnection
{
public:
	UCSConnection();
	void SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void ProcessPacket(uint16 opcode, EQ::Net::Packet &p);
	void SendPacket(ServerPacket* pack);
	void Disconnect() { if(Stream && Stream->Handle()) Stream->Handle()->Disconnect(); }
	void SendMessage(const char *From, const char *Message);
private:
	inline std::string GetIP() const { return (Stream && Stream->Handle()) ? Stream->Handle()->RemoteIP() : 0; }
	std::shared_ptr<EQ::Net::ServertalkServerConnection> Stream;
};

#endif /*UCS_H_*/
