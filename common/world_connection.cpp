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

void EQ::WorldConnection::RouteMessage(const std::string &filter, int type, const EQ::Net::Packet &p)
{
	if (!m_connection->Connected()) {
		return;
	}

	auto identifier = m_connection->GetIdentifier();

	EQ::Net::DynamicPacket out;
	out.PutUInt32(out.Length(), static_cast<uint32_t>(filter.length()));
	out.PutString(out.Length(), filter);
	out.PutUInt32(out.Length(), static_cast<uint32_t>(identifier.length()));
	out.PutString(out.Length(), identifier);
	out.PutInt32(out.Length(), type);
	out.PutInt32(out.Length(), static_cast<uint32_t>(p.Length()));
	out.PutPacket(out.Length(), p);

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
		auto idx = 0;
		auto filter_length = p.GetInt32(idx); idx += sizeof(int32_t);
		auto filter = p.GetString(idx, filter_length); idx += filter_length;
		auto identifier_length = p.GetInt32(idx); idx += sizeof(int32_t);
		auto identifier = p.GetString(idx, identifier_length); idx += identifier_length;
		auto type = p.GetInt32(idx); idx += sizeof(int32_t);
		auto packet_length = p.GetInt32(idx); idx += sizeof(int32_t);
		auto packet = EQ::Net::StaticPacket(
			(void*)((const uint8_t*)p.Data() + idx),
			static_cast<size_t>(packet_length));
	
		m_on_routed_message(filter, identifier, type, packet);
	}
}
