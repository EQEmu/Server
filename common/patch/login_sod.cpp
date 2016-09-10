#include "login_sod.h"

EQ::Patches::LoginSoDPatch::LoginSoDPatch()
{
	m_opcode_manager.reset(new RegularOpcodeManager());
	if (!m_opcode_manager->LoadOpcodes("login_opcodes_sod.conf")) {
		m_opcode_manager.release();
	}

	m_signature.match_message_opcode = 0x01;
	m_signature.match_message_size = 0;
	m_message_size = 2;
}

EQ::Patches::LoginSoDPatch::~LoginSoDPatch()
{
}
