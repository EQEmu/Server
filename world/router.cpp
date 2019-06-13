#include "router.h"
#include "../common/string_util.h"

Router::Router()
{
}

Router::~Router()
{
}

void Router::AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	m_connections.push_back(connection);
	connection->OnMessage(ServerOP_RouteTo, std::bind(&Router::OnRouterMessage, this, connection, std::placeholders::_1, std::placeholders::_2));
}

void Router::RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	auto iter = m_connections.begin();
	while (iter != m_connections.end()) {
		if ((*iter) == connection) {
			m_connections.erase(iter);
			return;
		}

		iter++;
	}
}

void Router::OnRouterMessage(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection, uint16 opcode, const EQ::Net::Packet &p)
{
	auto msg = (RouteToMessage*)p.Data();
	char to_id[32];
	strn0cpy(to_id, msg->id, 32);

	strn0cpy(msg->id, connection->GetUUID().c_str(), 32);


	if (to_id[0] != '\0' && msg->filter[0] != '\0') {
		for (auto &connection : m_connections) {
			auto id = connection->GetUUID();
			auto identifier = connection->GetIdentifier();
			if (strcmp(to_id, id.c_str()) == 0) {
				connection->Send(ServerOP_RouteTo, p);
			}
			else if (strcmp(msg->filter, identifier.c_str()) == 0) {
				connection->Send(ServerOP_RouteTo, p);
			}
		}
	}
	else if (msg->filter[0] != '\0') {
		for (auto &connection : m_connections) {
			auto identifier = connection->GetIdentifier();
			if (strcmp(msg->filter, identifier.c_str()) == 0) {
				connection->Send(ServerOP_RouteTo, p);
			}
		}
	}
	else if (to_id[0] != '\0') {
		for (auto &connection : m_connections) {
			auto id = connection->GetUUID();
			if (strcmp(to_id, id.c_str()) == 0) {
				connection->Send(ServerOP_RouteTo, p);
			}
		}
	}
	else {
		for (auto &connection : m_connections) {
			connection->Send(ServerOP_RouteTo, p);
		}
	}
}
