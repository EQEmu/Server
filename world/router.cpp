#include "router.h"
#include <regex>

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
	auto msg = p.GetSerialize<RouteToMessage>(0);
	auto payload_offset = p.Length() - msg.payload_size;
	auto payload = p.GetPacket(payload_offset, msg.payload_size);

	auto out_msg = msg;
	out_msg.identifier = connection->GetIdentifier();
	out_msg.id = connection->GetUUID();

	EQ::Net::DynamicPacket out;
	out.PutSerialize(0, out_msg);
	out.PutPacket(out.Length(), payload);

	if (!msg.id.empty() && !msg.filter.empty()) {
		auto id_regex = std::regex(msg.id);
		auto filter_regex = std::regex(msg.filter);

		for (auto &connection : m_connections) {
			auto id = connection->GetUUID();
			auto identifier = connection->GetIdentifier();
			if (std::regex_match(id, id_regex)) {
				connection->Send(ServerOP_RouteTo, out);
			}
			else if (std::regex_match(identifier, filter_regex)) {
				connection->Send(ServerOP_RouteTo, out);
			}
		}
	}
	else if (!msg.id.empty()) {
		auto id_regex = std::regex(msg.id);

		for (auto &connection : m_connections) {
			auto id = connection->GetUUID();
			if (std::regex_match(id, id_regex)) {
				connection->Send(ServerOP_RouteTo, out);
			}
		}
	} else if (!msg.filter.empty()) {
		auto filter_regex = std::regex(msg.filter);

		for (auto &connection : m_connections) {
			auto identifier = connection->GetIdentifier();
			if (std::regex_match(identifier, filter_regex)) {
				connection->Send(ServerOP_RouteTo, out);
			}
		}
	}
	else {
		for (auto &connection : m_connections) {
			connection->Send(ServerOP_RouteTo, out);
		}
	}
}
