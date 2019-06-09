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

void EQ::WorldConnection::RouteMessage(const std::string &filter, const EQ::Net::Packet &p)
{
	if (!m_connection->Connected()) {
		return;
	}

	RouteToHeader header;
	strn0cpy(header.filter, filter.data(), 64);
	strn0cpy(header.type, m_connection->GetIdentifier().data(), 128);

	EQ::Net::DynamicPacket out;
	out.PutSerialize(0, header);
	out.PutPacket(out.Length(), p);
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
		auto header = p.GetSerialize<RouteToHeader>(0);
		auto np = EQ::Net::StaticPacket((int8_t*)p.Data() + sizeof(RouteToHeader), p.Length() - sizeof(RouteToHeader));

		m_on_routed_message(header.filter, header.type, np);
	}
}
