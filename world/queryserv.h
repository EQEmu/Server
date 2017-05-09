#ifndef QueryServ_H
#define QueryServ_H

#include "../common/types.h"
#include "../common/net/servertalk_server.h"
#include "../common/servertalk.h"

class QueryServConnection
{
public:
	QueryServConnection();
	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void HandleGenericMessage(uint16_t opcode, EQ::Net::Packet &p);
	void HandleLFGuildUpdateMessage(uint16_t opcode, EQ::Net::Packet &p);
	bool SendPacket(ServerPacket* pack);
private:
	std::map<std::string, std::shared_ptr<EQ::Net::ServertalkServerConnection>> m_streams;
};

#endif /*QueryServ_H_*/
