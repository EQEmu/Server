/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../global_define.h"
#include "../eqemu_config.h"
#include "../eqemu_logsys.h"
#include "larion.h"
#include "../opcodemgr.h"

#include "../eq_stream_ident.h"
#include "../crc32.h"

#include "../eq_packet_structs.h"
#include "../misc_functions.h"
#include "../strings.h"
#include "../inventory_profile.h"
#include "larion_structs.h"
#include "../rulesys.h"
#include "../path_manager.h"
#include "../classes.h"
#include "../races.h"
#include "../raid.h"

#include <iostream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cinttypes>

namespace Larion
{
	static const char* name = "Larion";
	static OpcodeManager* opcodes = nullptr;
	static Strategy struct_strategy;

	void Register(EQStreamIdentifier& into)
	{
		//create our opcode manager if we havent already
		if (opcodes == nullptr) {

			std::string opfile = fmt::format("{}/patch_{}.conf", path.GetPatchPath(), name);

			//load up the opcode manager.
			//TODO: figure out how to support shared memory with multiple patches...
			opcodes = new RegularOpcodeManager();
			if (!opcodes->LoadOpcodes(opfile.c_str())) {
				LogNetcode("[OPCODES] Error loading opcodes file [{}]. Not registering patch [{}]", opfile.c_str(), name);
				return;
			}
		}

		//ok, now we have what we need to register.

		EQStreamInterface::Signature signature;
		std::string pname;

		//register our world signature.
		pname = std::string(name) + "_world";
		signature.ignore_eq_opcode = 0;
		signature.first_length = sizeof(structs::LoginInfo_Struct);
		signature.first_eq_opcode = opcodes->EmuToEQ(OP_SendLoginInfo);
		into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);

		//register our zone signature.
		pname = std::string(name) + "_zone";
		signature.ignore_eq_opcode = opcodes->EmuToEQ(OP_AckPacket);
		signature.first_length = sizeof(structs::ClientZoneEntry_Struct);
		signature.first_eq_opcode = opcodes->EmuToEQ(OP_ZoneEntry);
		into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);
		LogNetcode("[StreamIdentify] Registered patch [{}]", name);
	}

	void Reload()
	{
		//we have a big problem to solve here when we switch back to shared memory
		//opcode managers because we need to change the manager pointer, which means
		//we need to go to every stream and replace it's manager.

		if (opcodes != nullptr) {
			std::string opfile = fmt::format("{}/patch_{}.conf", path.GetPatchPath(), name);
			if (!opcodes->ReloadOpcodes(opfile.c_str())) {
				LogNetcode("[OPCODES] Error reloading opcodes file [{}] for patch [{}]", opfile.c_str(), name);
				return;
			}
			LogNetcode("[OPCODES] Reloaded opcodes for patch [{}]", name);
		}
	}

	Strategy::Strategy() : StructStrategy()
	{
		//all opcodes default to passthrough.
#include "ss_register.h"
#include "larion_ops.h"
	}

	std::string Strategy::Describe() const
	{
		std::string r;
		r += "Patch ";
		r += name;
		return(r);
	}

	const EQ::versions::ClientVersion Strategy::ClientVersion() const
	{
		return EQ::versions::ClientVersion::Larion;
	}

#include "ss_define.h"

	// ENCODE methods
	ENCODE(OP_LogServer) {
		EQApplicationPacket* in = *p;
		*p = nullptr;
		LogServer_Struct* emu = (LogServer_Struct*)in->pBuffer;

		auto outapp = new EQApplicationPacket(OP_LogServer, 1840);
		auto buffer = outapp->pBuffer;

		//pvp
		if (emu->enable_pvp) {
			*(char*)&buffer[0x04] = 1;
		}

		if (emu->enable_FV) {
			//FV sets these both to 1
			//one appears to enable the no drop flag the other just marks the server as special?
			*(char*)&buffer[0x08] = 1;
			*(char*)&buffer[0x0a] = 1;
		}

		//This has something to do with heirloom and prestige items but im not sure what it does
		//Seems to sit at 0
		*(char*)&buffer[0x71d] = 0;

		//not sure what this does, something to do with server select
		*(char*)&buffer[0x09] = 0;

		//this appears to have some effect on the tradeskill system; disabling made by tags perhaps?
		*(char*)&buffer[0x0b] = 0;

		//not sure, setting it to the value ive seen
		*(char*)&buffer[0x0c] = 1;

		//Something to do with languages
		*(char*)&buffer[0x0d] = 1;

		//These seem to affect if server has betabuff enabled
		*(char*)&buffer[0x5c0] = 0;
		*(char*)&buffer[0x5c1] = 0;
		//This is set on test so it's probably indicating this is a test server
		*(char*)&buffer[0x5c2] = 0;

		//not sure, but it's grouped with the beta and test stuff
		*(char*)&buffer[0x5c3] = 0;

		//world short name
		strncpy((char*)&buffer[0x15], emu->worldshortname, 32);

		//not sure, affects some player calculation but didn't care to look more
		*(char*)&buffer[0x5c2] = 0;

		//Looks right
		if (emu->enablemail) {
			*(char*)&buffer[0x5b5] = 1;
		}

		//Looks right
		if (emu->enablevoicemacros) {
			*(char*)&buffer[0x5b4] = 1;
		}

		//Not sure, sending what we've seen
		*(char*)&buffer[0x5b6] = 0;

		//Not sure sending what we've seen
		*(char*)&buffer[0x5b8] = 1;

		//Not sure sending what we've seen
		*(int32_t*)&buffer[0x5fc] = -1;

		//Test sets this to 1, everyone else seems to set it to 0
		*(int32_t*)&buffer[0x600] = 0;

		//Disassembly puts it next to code dealing with commands, ive not seen anyone send anything but 0
		*(char*)&buffer[0x705] = 0;

		//Something about item restrictions, seems to always be set to 1
		*(char*)&buffer[0x710] = 0;

		//This and 0x724 are often multiplied together in guild favor calcs, live and test send 1.0f
		*(float*)&buffer[0x720] = 1.0f;
		*(float*)&buffer[0x724] = 1.0f;

		//This and 0x72c are often multiplied together in non-guild favor calcs, live and test send 1.0f
		*(float*)&buffer[0x728] = 1.0f;
		*(float*)&buffer[0x72c] = 1.0f;

		dest->FastQueuePacket(&outapp);
		safe_delete(in);
	}

	// DECODE methods
	
} /*Larion*/
