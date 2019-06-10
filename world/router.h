#pragma once

#include "../common/net/servertalk_server_connection.h"
#include <memory>
#include <list>

class Router
{
public:
	Router();
	~Router();

	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
private:
	std::list<std::shared_ptr<EQ::Net::ServertalkServerConnection>> m_connections;

	void OnRouterMessage(uint16 opcode, const EQ::Net::Packet &p);
};
