#include "world_connection.h"
#include "eqemu_config.h"
#include "string_util.h"

EQ::WorldConnection::WorldConnection(const std::string &type) {
	auto config = EQEmuConfig::get();

	m_connection.reset(new EQ::Net::ServertalkClient(config->WorldIP, config->WorldTCPPort, false, type, config->SharedKey));
	m_connection->OnConnect([this](EQ::Net::ServertalkClient *client) {
		if (m_on_connected) {
			m_on_connected();
		}
	});
	
	m_connection->OnMessage(std::bind(&WorldConnection::_HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_connection->OnMessage(ServerOP_RouteTo, std::bind(&WorldConnection::_HandleRoutedMessage, this, std::placeholders::_1, std::placeholders::_2));
}

EQ::WorldConnection::~WorldConnection() {

}

void EQ::WorldConnection::SendPacket(ServerPacket* pack) {
	m_connection->SendPacket(pack);
}

std::string EQ::WorldConnection::GetIP() const {
	return m_connection->Handle()->RemoteIP();
}

uint16 EQ::WorldConnection::GetPort() const {
	return m_connection->Handle()->RemotePort();
}

bool EQ::WorldConnection::Connected() const {
	return m_connection->Connected();
}

void EQ::WorldConnection::RouteMessage(const std::string &filter, const std::string &id, const EQ::Net::Packet& payload)
{
	if (!m_connection->Connected()) {
		return;
	}

	auto identifier = m_connection->GetIdentifier();

	RouteToMessage msg;
	msg.filter = filter;
	msg.identifier = identifier;
	msg.id = id;
	msg.payload_size = payload.Length();

	EQ::Net::DynamicPacket out;
	out.PutSerialize(0, msg);
	out.PutPacket(out.Length(), payload);

	m_connection->Send(ServerOP_RouteTo, out);
}

void EQ::WorldConnection::_HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	if (m_on_message) {
		m_on_message(opcode, p);
	}
}

void EQ::WorldConnection::_HandleRoutedMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	if (m_on_routed_message) {
		auto msg = p.GetSerialize<RouteToMessage>(0);
		auto payload_offset = p.Length() - msg.payload_size;
		auto payload = p.GetPacket(payload_offset, msg.payload_size);

		m_on_routed_message(msg.filter, msg.identifier, msg.id, payload);
	}
}
