#include "eqstream.h"
#include "../eqemu_logsys.h"

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

void EQ::Net::EQStreamManager::DaybreakPacketRecv(std::shared_ptr<DaybreakConnection> connection, const Packet &p)
{
	auto iter = m_streams.find(connection);
	if (iter != m_streams.end()) {
		auto &stream = iter->second;
		std::unique_ptr<EQ::Net::Packet> t(new EQ::Net::DynamicPacket());
		t->PutPacket(0, p);
		stream->m_packet_queue.push_back(std::move(t));
	}
}

EQ::Net::EQStream::EQStream(EQStreamManager *owner, std::shared_ptr<DaybreakConnection> connection)
{
	m_owner = owner;
	m_connection = connection;
	m_opcode_manager = nullptr;
}

EQ::Net::EQStream::~EQStream()
{
}

void EQ::Net::EQStream::QueuePacket(const EQApplicationPacket *p, bool ack_req) {
	if (m_opcode_manager && *m_opcode_manager) {
		uint16 opcode = 0;
		if (p->GetOpcodeBypass() != 0) {
			opcode = p->GetOpcodeBypass();
		}
		else {
			opcode = (*m_opcode_manager)->EmuToEQ(p->GetOpcode());
		}

		EQ::Net::DynamicPacket out;
		switch (m_owner->m_options.opcode_size) {
		case 1:
			out.PutUInt8(0, opcode);
			out.PutData(1, p->pBuffer, p->size);
			break;
		case 2:
			out.PutUInt16(0, opcode);
			out.PutData(2, p->pBuffer, p->size);
			break;
		}

		if (ack_req) {
			m_connection->QueuePacket(out);
		}
		else {
			m_connection->QueuePacket(out, 0, false);
		}
	}
}

void EQ::Net::EQStream::FastQueuePacket(EQApplicationPacket **p, bool ack_req) {
	QueuePacket(*p, ack_req);
	delete *p;
	*p = nullptr;
}

EQApplicationPacket *EQ::Net::EQStream::PopPacket() {
	if (m_packet_queue.empty()) {
		return nullptr;
	}

	if (m_opcode_manager != nullptr && *m_opcode_manager != nullptr) {
		auto &p = m_packet_queue.front();

		uint16 opcode = 0;
		switch (m_owner->m_options.opcode_size) {
		case 1:
			opcode = p->GetUInt8(0);
			break;
		case 2:
			opcode = p->GetUInt16(0);
			break;
		}

		EmuOpcode emu_op = (*m_opcode_manager)->EQToEmu(opcode);
		EQApplicationPacket *ret = new EQApplicationPacket(emu_op, (unsigned char*)p->Data() + m_owner->m_options.opcode_size, p->Length() - m_owner->m_options.opcode_size);
		ret->SetProtocolOpcode(opcode);
		m_packet_queue.pop_front();
		return ret;
	}

	return nullptr;
}

void EQ::Net::EQStream::Close() {
	m_connection->Close();
}

std::string EQ::Net::EQStream::GetRemoteAddr() const
{
	return RemoteEndpoint();
}

uint32 EQ::Net::EQStream::GetRemoteIP() const {
	return inet_addr(RemoteEndpoint().c_str());
}

bool EQ::Net::EQStream::CheckState(EQStreamState state) {
	return GetState() == state;
}

EQStreamInterface::MatchState EQ::Net::EQStream::CheckSignature(const Signature *sig) {
	if (!m_packet_queue.empty()) {
		auto p = m_packet_queue.front().get();
		uint16 opcode = 0;
		size_t length = p->Length() - m_owner->m_options.opcode_size;
		switch (m_owner->m_options.opcode_size) {
		case 1:
			opcode = p->GetUInt8(0);
			break;
		case 2:
			opcode = p->GetUInt16(0);
			break;
		}

		if (sig->ignore_eq_opcode != 0 && opcode == sig->ignore_eq_opcode) {
			if (m_packet_queue.size() > 1) {
				p = m_packet_queue[1].get();
				opcode = 0;
				length = p->Length() - m_owner->m_options.opcode_size;
				switch (m_owner->m_options.opcode_size) {
				case 1:
					opcode = p->GetUInt8(0);
					break;
				case 2:
					opcode = p->GetUInt16(0);
					break;
				}
			}
			else {
				return MatchNotReady;
			}
		}

		if (opcode == sig->first_eq_opcode) {
			if (length == sig->first_length) {
				LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode matched {2:#x} and length matched {3}",
					RemoteEndpoint(), m_connection->RemotePort(), sig->first_eq_opcode, length);
				return MatchSuccessful;
			}
			else if (length == 0) {
				LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode matched {2:#x} and length is ignored.",
					RemoteEndpoint(), m_connection->RemotePort(), sig->first_eq_opcode);
				return MatchSuccessful;
			}
			else {
				LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode matched {2:#x} but length {3} did not match expected {4}",
					RemoteEndpoint(), m_connection->RemotePort(), sig->first_eq_opcode, length, sig->first_length);
				return MatchFailed;
			}
		}
		else {
			LogF(Logs::General, Logs::Netcode, "[IDENT_TRACE] {0}:{1}: First opcode {1:#x} did not match expected {2:#x}",
				RemoteEndpoint(), m_connection->RemotePort(), opcode, sig->first_eq_opcode);
			return MatchFailed;
		}
	}

	return MatchNotReady;
}

EQStreamState EQ::Net::EQStream::GetState() {
	auto status = m_connection->GetStatus();
	switch (status) {
	case StatusConnecting:
		return UNESTABLISHED;
	case StatusConnected:
		return ESTABLISHED;
	case StatusDisconnecting:
		return DISCONNECTING;
	default:
		return CLOSED;
	}
}