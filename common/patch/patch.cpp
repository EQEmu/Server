#include "patch.h"
#include "../eqemu_logsys.h"

EQ::Patches::IdentityMatchStatus EQ::Patches::BasePatch::TryIdentityMatch(const EQ::Net::Packet &p) const
{
	if (p.Length() < m_message_size) {
		return IdentityMatchFailure;
	}

	int raw_opcode = 0;
	switch (m_message_size) {
	case 1:
		raw_opcode = *(uint8_t*)p.Data();
		break;
	case 2:
		raw_opcode = *(uint16_t*)p.Data();
		break;
	default:
		return IdentityMatchFailure;
	}

	if (m_signature.match_message_opcode != 0 && m_signature.match_message_opcode != raw_opcode) {
		return IdentityMatchFailure;
	}

	if (m_signature.match_message_size > 0 && m_signature.match_message_size != p.Length() - m_message_size) {
		return IdentityMatchFailure;
	}

	return IdentityMatchSuccess;
}

void EQ::Patches::BasePatch::Decode(const EQ::Net::Packet *in, EmuOpcode &opcode, EQ::Net::WritablePacket &out)
{
	int raw_opcode = 0;
	switch (m_message_size) {
	case 1:
		raw_opcode = *(uint8_t*)in->Data();
		break;
	case 2:
		raw_opcode = *(uint16_t*)in->Data();
		break;
	default:
		opcode = OP_Unknown;
		return;
	}

	opcode = m_opcode_manager->EQToEmu(raw_opcode);
	if (opcode == OP_Unknown) {
		out.PutData(0, (uint8_t*)in->Data() + m_message_size, in->Length() - m_message_size);
		return;
	}

	auto decode_iter = m_decode.find(opcode);
	if (decode_iter != m_decode.end()) {
		EQ::Net::ReadOnlyPacket p((uint8_t*)in->Data() + m_message_size, in->Length() - m_message_size);
		decode_iter->second(&p, opcode, out);
	}
	else {
		out.PutData(0, (uint8_t*)in->Data() + m_message_size, in->Length() - m_message_size);
	}
}

void EQ::Patches::BasePatch::Encode(std::shared_ptr<EQ::Net::DaybreakConnection> connection, EmuOpcode opcode, const EQ::Net::Packet *in)
{
	auto encode_iter = m_encode.find(opcode);
	if (encode_iter != m_encode.end()) {
		encode_iter->second(connection, opcode, in);
	}
	else {
		SendPacket(connection, opcode, in);
	}
}

void EQ::Patches::BasePatch::RegisterDecode(int protocol_number, DecodeStructFunction f)
{
	m_decode.insert(std::make_pair(protocol_number, f));
}

void EQ::Patches::BasePatch::RegisterEncode(EmuOpcode opcode, EncodeStructFunction f)
{
	m_encode.insert(std::make_pair(opcode, f));
}

void EQ::Patches::BasePatch::SendPacket(std::shared_ptr<EQ::Net::DaybreakConnection> connection, EmuOpcode opcode, const EQ::Net::Packet *p)
{
	if (!m_opcode_manager) {
		return;
	}

	auto raw_opcode = m_opcode_manager->EmuToEQ(opcode);
	EQ::Net::WritablePacket out;
	switch (m_message_size) {
	case 1:
		out.PutUInt8(0, (uint8_t)raw_opcode);
		out.PutPacket(1, *p);
		break;
	case 2:
		out.PutUInt16(0, raw_opcode);
		out.PutPacket(2, *p);
		break;
	default:
		return;
	}

	connection->QueuePacket(out);
}
