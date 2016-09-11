#include "chat.h"

EQ::Patches::ChatPatch::ChatPatch()
{
	m_opcode_manager.reset(new RegularOpcodeManager());
	if (!m_opcode_manager->LoadOpcodes("mail_opcodes.conf")) {
		m_opcode_manager.release();
	}

	m_signature.match_message_opcode = 0x0;
	m_signature.match_message_size = 0;
	m_message_size = 1; 
}

EQ::Patches::ChatPatch::~ChatPatch()
{
}
