#include "router.h"

Router::Router()
{
}

Router::~Router()
{
}

void Router::AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	m_connections.push_back(connection);
	connection->OnMessage(ServerOP_RouteTo, std::bind(&Router::OnRouterMessage, this, std::placeholders::_1, std::placeholders::_2));
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

void Router::OnRouterMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	auto idx = 0;
	auto filter_length = p.GetInt32(idx); idx += sizeof(int32_t);
	auto filter = p.GetString(idx, filter_length); idx += filter_length;

	printf("Recv router msg of size %i\n", p.Length());

	for (auto &connection : m_connections) {
		auto identifier = connection->GetIdentifier();
		auto pos = identifier.find(filter);
		if (pos == 0) {
			connection->Send(opcode, p);
		}
	}
}
