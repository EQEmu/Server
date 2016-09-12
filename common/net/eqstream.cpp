#include "eqstream.h"
#include <eqemu_logsys.h>

EQ::Net::EQStreamManager::EQStreamManager(EQStreamManagerOptions &options) : m_daybreak(options.daybreak_options)
{
	m_options = options;

	m_daybreak.OnNewConnection(std::bind(&EQStreamManager::DaybreakNewConnection, this, std::placeholders::_1));
	m_daybreak.OnConnectionStateChange(std::bind(&EQStreamManager::DaybreakConnectionStateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_daybreak.OnPacketRecv(std::bind(&EQStreamManager::DaybreakPacketRecv, this, std::placeholders::_1, std::placeholders::_2));
}

EQ::Net::EQStreamManager::~EQStreamManager()
{
}

void EQ::Net::EQStreamManager::DaybreakNewConnection(std::shared_ptr<DaybreakConnection> connection)
{
	std::shared_ptr<EQStream> stream(new EQStream(this, connection));
	m_streams.insert(std::make_pair(connection, stream));
	if (m_on_new_connection) {
		m_on_new_connection(stream);
	}
}

void EQ::Net::EQStreamManager::DaybreakConnectionStateChange(std::shared_ptr<DaybreakConnection> connection, DbProtocolStatus from, DbProtocolStatus to)
{
	auto iter = m_streams.find(connection);
	if (iter != m_streams.end()) {
		if (m_on_connection_state_change) {
			m_on_connection_state_change(iter->second, from, to);
		}

		if (to == EQ::Net::StatusDisconnected) {
			m_streams.erase(iter);
		}
	}
}

void EQ::Net::EQStreamManager::DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, Packet &p)
{
	auto iter = m_streams.find(connection);
	if (iter != m_streams.end()) {
		auto patch = iter->second->GetRegisteredPatch();
		if (patch == nullptr && m_possible_patches.size() > 0) {
			for (auto &pt : m_possible_patches) {
				auto match = pt->TryIdentityMatch(p);
				if (match == EQ::Patches::IdentityMatchSuccess) {
					iter->second->RegisterPatch(pt.get());
					patch = pt.get();
					Log.Out(Logs::General, Logs::Debug, "Identified patch with name %s", pt->GetName().c_str());
				}
			}
		}

		if (patch) {
			EmuOpcode opcode;
			EQ::Net::WritablePacket out;
			patch->Decode(&p, opcode, out);

			if (opcode == OP_Unknown) {
				Log.Out(Logs::General, Logs::Netcode, "Incoming packet was not handled because the opcode was not found.\n%s", p.ToString().c_str());
			}
			else {
				if (m_on_packet_recv) {
					m_on_packet_recv(iter->second, opcode, out);
				}
			}
		}
		else {
			Log.Out(Logs::General, Logs::Netcode, "Incoming packet was not handled because we don't have a patch set.\n%s", p.ToString().c_str());
		}
	}
}

EQ::Net::EQStream::EQStream(EQStreamManager *owner, std::shared_ptr<DaybreakConnection> connection)
{
	m_owner = owner;
	m_connection = connection;
	m_patch = nullptr;
}

EQ::Net::EQStream::~EQStream()
{
}

void EQ::Net::EQStream::QueuePacket(EmuOpcode type, const Packet &p)
{
	if (m_patch) {
		EQ::Net::WritablePacket trans;
		m_patch->Encode(m_connection, type, &p);
	}
}

void EQ::Net::EQStream::ResetStats()
{
	m_connection->ResetStats();
}

void EQ::Net::EQStream::Close()
{
}

void EQ::Net::EQStream::QueuePacket(const EQApplicationPacket *p)
{
	EQ::Net::ReadOnlyPacket out(p->pBuffer, p->size);
	QueuePacket(p->GetOpcode(), out);

}

void EQ::Net::EQStream::FastQueuePacket(const EQApplicationPacket **p)
{
	QueuePacket(*p);
	delete *p;
}