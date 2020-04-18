#ifndef UCS_H
#define UCS_H

#include "../common/types.h"
#include "../common/net/servertalk_server_connection.h"
#include "../common/servertalk.h"
#include "../common/event/timer.h"
#include <memory>

class UCSConnection
{
public:
	UCSConnection();
	void SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void ProcessPacket(uint16 opcode, EQ::Net::Packet &p);
	void SendPacket(ServerPacket* pack);
	void Disconnect() { if(connection && connection->Handle()) connection->Handle()->Disconnect(); }
	void SendMessage(const char *From, const char *Message);
private:
	inline std::string GetIP() const { return (connection && connection->Handle()) ? connection->Handle()->RemoteIP() : 0; }
	std::shared_ptr<EQ::Net::ServertalkServerConnection> connection;

	/**
	 * Keepalive
	 */
	std::unique_ptr<EQ::Timer> m_keepalive;
	void OnKeepAlive(EQ::Timer *t);
};

#endif /*UCS_H_*/
