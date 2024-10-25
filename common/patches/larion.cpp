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
		SETUP_VAR_ENCODE(LogServer_Struct);
		ALLOC_LEN_ENCODE(1840);

		//pvp
		if (emu->enable_pvp) {
			*(char*)&__packet->pBuffer[0x04] = 1;
		}

		if (emu->enable_FV) {
			//FV sets these both to 1
			//one appears to enable the no drop flag the other just marks the server as special?
			*(char*)&__packet->pBuffer[0x08] = 1;
			*(char*)&__packet->pBuffer[0x0a] = 1;
		}

		//This has something to do with heirloom and prestige items but im not sure what it does
		//Seems to sit at 0
		*(char*)&__packet->pBuffer[0x71d] = 0;

		//not sure what this does, something to do with server select
		*(char*)&__packet->pBuffer[0x09] = 0;

		//this appears to have some effect on the tradeskill system; disabling made by tags perhaps?
		*(char*)&__packet->pBuffer[0x0b] = 0;

		//not sure, setting it to the value ive seen
		*(char*)&__packet->pBuffer[0x0c] = 1;

		//Something to do with languages
		*(char*)&__packet->pBuffer[0x0d] = 1;

		//These seem to affect if server has betabuff enabled
		*(char*)&__packet->pBuffer[0x5c0] = 0;
		*(char*)&__packet->pBuffer[0x5c1] = 0;
		//This is set on test so it's probably indicating this is a test server
		*(char*)&__packet->pBuffer[0x5c2] = 0;

		//not sure, but it's grouped with the beta and test stuff
		*(char*)&__packet->pBuffer[0x5c3] = 0;

		//world short name
		strncpy((char*)&__packet->pBuffer[0x15], emu->worldshortname, 32);

		//not sure, affects some player calculation but didn't care to look more
		*(char*)&__packet->pBuffer[0x5c2] = 0;

		//Looks right
		if (emu->enablemail) {
			*(char*)&__packet->pBuffer[0x5b5] = 1;
		}

		//Looks right
		if (emu->enablevoicemacros) {
			*(char*)&__packet->pBuffer[0x5b4] = 1;
		}

		//Not sure, sending what we've seen
		*(char*)&__packet->pBuffer[0x5b6] = 0;

		//Not sure sending what we've seen
		*(char*)&__packet->pBuffer[0x5b8] = 1;

		//Not sure sending what we've seen
		*(int32_t*)&__packet->pBuffer[0x5fc] = -1;

		//Test sets this to 1, everyone else seems to set it to 0
		*(int32_t*)&__packet->pBuffer[0x600] = 0;

		//Disassembly puts it next to code dealing with commands, ive not seen anyone send anything but 0
		*(char*)&__packet->pBuffer[0x705] = 0;

		//Something about item restrictions, seems to always be set to 1
		*(char*)&__packet->pBuffer[0x710] = 0;

		//This and 0x724 are often multiplied together in guild favor calcs, live and test send 1.0f
		*(float*)&__packet->pBuffer[0x720] = 1.0f;
		*(float*)&__packet->pBuffer[0x724] = 1.0f;

		//This and 0x72c are often multiplied together in non-guild favor calcs, live and test send 1.0f
		*(float*)&__packet->pBuffer[0x728] = 1.0f;
		*(float*)&__packet->pBuffer[0x72c] = 1.0f;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMembership) {
		ENCODE_LENGTH_EXACT(Membership_Struct);
		SETUP_DIRECT_ENCODE(Membership_Struct, structs::Membership_Struct);

		eq->membership = emu->membership;
		eq->races = emu->races;
		eq->classes = emu->classes;
		eq->entrysize = 33;
		eq->entries[0] = -1;  // Max AA Restriction
		eq->entries[1] = -1;  // Max Level Restriction
		eq->entries[2] = -1;  // Max Char Slots per Account (not used by client?)
		eq->entries[3] = -1;  // 1 for Silver
		eq->entries[4] = -1;  // Main Inventory Size
		eq->entries[5] = -1;  // Max Platinum per level
		eq->entries[6] = 1;   // Send Mail
		eq->entries[7] = 1;   // Use Parcels?
		eq->entries[8] = 1;   // Voice Chat
		eq->entries[9] = -1;  // Merc Tiers
		eq->entries[10] = 1;  // Create Guilds
		eq->entries[11] = -1; // Shared Bank Slots
		eq->entries[12] = -1; // Max Journal Quests
		eq->entries[13] = 1;  // Housing Enabled
		eq->entries[14] = 1;  // Prestiege
		eq->entries[15] = 1;  // Broker System
		eq->entries[16] = 1;  // Chat
		eq->entries[17] = 1;  // Progression Server Access
		eq->entries[18] = 1;  // Customer Support
		eq->entries[19] = -1; // Popup reminders?
		eq->entries[20] = -1; // Exit Popup?
		eq->entries[21] = 0;
		eq->entries[22] = 0;
		eq->entries[23] = 0;  // This is the highest we actually see in detail entries
		eq->entries[24] = 0;  
		eq->entries[25] = 0;
		eq->entries[26] = 0;
		eq->entries[27] = 0;
		eq->entries[28] = 0;
		eq->entries[29] = 0;
		eq->entries[30] = 0;
		eq->entries[31] = 0;
		eq->entries[32] = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMembershipDetails) {
		SETUP_VAR_ENCODE(Membership_Details_Struct);

		SerializeBuffer buffer;
		// Generated via script; we should put this in a better format later
		// count
		buffer.WriteUInt32(96);

		// settings
		buffer.WriteUInt8(0);
		buffer.WriteUInt32(0);
		buffer.WriteInt32(250);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(0);
		buffer.WriteInt32(1000);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(1);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(1);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(2);
		buffer.WriteInt32(2);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(0);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(0);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(2);
		buffer.WriteInt32(4);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(3);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(1);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(1);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(3);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(4);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(2);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(2);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(4);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(5);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(3);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(3);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(5);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(6);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(4);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(4);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(6);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(7);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(5);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(5);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(7);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(8);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(6);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(6);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(8);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(9);
		buffer.WriteInt32(5);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(7);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(7);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(9);
		buffer.WriteInt32(5);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(10);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(8);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(8);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(11);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(10);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(9);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(9);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(12);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(11);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(10);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(10);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(13);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(12);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(11);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(11);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(14);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(13);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(12);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(12);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(15);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(14);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(13);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(13);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(16);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(15);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(14);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(14);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(17);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(16);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(15);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(15);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(18);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(17);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(16);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(16);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(19);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(18);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(17);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(17);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(20);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(19);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(18);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(18);
		buffer.WriteInt32(1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(21);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(20);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(19);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(19);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(22);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(21);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(20);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(20);
		buffer.WriteInt32(-1);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(21);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(0);
		buffer.WriteUInt32(23);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(22);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(21);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(22);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(1);
		buffer.WriteUInt32(23);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(22);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(2);
		buffer.WriteUInt32(23);
		buffer.WriteInt32(0);

		buffer.WriteUInt8(3);
		buffer.WriteUInt32(23);
		buffer.WriteInt32(0);

		// race count
		buffer.WriteUInt32(17);

		// races
		buffer.WriteUInt32(1);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(333);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(90287);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(90289);
		buffer.WriteUInt32(16);

		buffer.WriteUInt32(90290);
		buffer.WriteUInt32(32);

		buffer.WriteUInt32(90291);
		buffer.WriteUInt32(64);

		buffer.WriteUInt32(90292);
		buffer.WriteUInt32(128);

		buffer.WriteUInt32(90293);
		buffer.WriteUInt32(256);

		buffer.WriteUInt32(90294);
		buffer.WriteUInt32(512);

		buffer.WriteUInt32(90295);
		buffer.WriteUInt32(1024);

		buffer.WriteUInt32(90296);
		buffer.WriteUInt32(2048);

		buffer.WriteUInt32(90297);
		buffer.WriteUInt32(8192);

		buffer.WriteUInt32(90298);
		buffer.WriteUInt32(16384);

		buffer.WriteUInt32(90299);
		buffer.WriteUInt32(32768);

		buffer.WriteUInt32(90300);
		buffer.WriteUInt32(65536);

		buffer.WriteUInt32(2012271);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(2012277);
		buffer.WriteUInt32(131071);

		// classes count
		buffer.WriteUInt32(17);

		// classes
		buffer.WriteUInt32(1);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(333);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(90287);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(90301);
		buffer.WriteUInt32(8);

		buffer.WriteUInt32(90302);
		buffer.WriteUInt32(16);

		buffer.WriteUInt32(90303);
		buffer.WriteUInt32(32);

		buffer.WriteUInt32(90304);
		buffer.WriteUInt32(64);

		buffer.WriteUInt32(90305);
		buffer.WriteUInt32(128);

		buffer.WriteUInt32(90306);
		buffer.WriteUInt32(256);

		buffer.WriteUInt32(90307);
		buffer.WriteUInt32(1024);

		buffer.WriteUInt32(90308);
		buffer.WriteUInt32(2048);

		buffer.WriteUInt32(90309);
		buffer.WriteUInt32(8192);

		buffer.WriteUInt32(90310);
		buffer.WriteUInt32(16384);

		buffer.WriteUInt32(90311);
		buffer.WriteUInt32(32768);

		buffer.WriteUInt32(90312);
		buffer.WriteUInt32(65536);

		buffer.WriteUInt32(2012271);
		buffer.WriteUInt32(131071);

		buffer.WriteUInt32(2012277);
		buffer.WriteUInt32(131071);

		// exit string length
		buffer.WriteUInt32(0);

		__packet->size = buffer.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buffer.buffer(), __packet->size);
		FINISH_ENCODE();
	}

	ENCODE(OP_SendMaxCharacters) {
		ENCODE_LENGTH_EXACT(MaxCharacters_Struct);
		SETUP_DIRECT_ENCODE(MaxCharacters_Struct, structs::MaxCharacters_Struct);

		//OUT(max_chars);
		eq->max_chars = 8; //needs to be fixed
		eq->marketplace_chars = 0;
		eq->unknown008 = -1;
		eq->unknown00c = 196608;
		eq->unknown010 = 0;
		eq->unknown014 = 0;
		eq->unknown018 = 0;
		eq->unknown01c = 0;
		eq->unknown020 = -1;
		eq->unknown024 = 0;
		eq->unknown028 = 0;
		eq->unknown02c = 0;
		eq->unknown030 = 0;
		eq->unknown034 = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendCharInfo) {
		SETUP_VAR_ENCODE(CharacterSelect_Struct);

		SerializeBuffer buffer;

		buffer.WriteUInt32(emu->CharCount);

		for (int i = 0; i < emu->CharCount; ++i) {
			auto *char_info = &emu->Entries[i];
			buffer.WriteString(char_info->Name);
			buffer.WriteUInt32(char_info->Class);
			buffer.WriteUInt32(char_info->Race);
			buffer.WriteUInt8(1); //not sure seen 1
			buffer.WriteUInt32(char_info->ShroudRace);
			buffer.WriteUInt32(char_info->ShroudClass);
			buffer.WriteUInt16(char_info->Zone);
			buffer.WriteUInt16(char_info->Instance);
			buffer.WriteUInt8(char_info->Gender);
			buffer.WriteUInt8(char_info->Face);

			for (int j = 0; j < 9; ++j) {
				buffer.WriteUInt32(char_info->Equip[j].Material);
				buffer.WriteUInt32(char_info->Equip[j].Unknown1);
				buffer.WriteUInt32(char_info->Equip[j].EliteModel);
				buffer.WriteUInt32(char_info->Equip[j].HerosForgeModel);
				buffer.WriteUInt32(char_info->Equip[j].Unknown2);
				buffer.WriteUInt32(char_info->Equip[j].Color);
			}

			buffer.WriteUInt8(255); //seen 255
			buffer.WriteUInt8(0); //seen 0
			buffer.WriteUInt32(char_info->DrakkinTattoo);
			buffer.WriteUInt32(char_info->DrakkinDetails);
			buffer.WriteUInt32(char_info->Deity);
			buffer.WriteUInt32(char_info->PrimaryIDFile);
			buffer.WriteUInt32(char_info->SecondaryIDFile);
			buffer.WriteUInt8(char_info->HairColor);
			buffer.WriteUInt8(char_info->BeardColor);
			buffer.WriteUInt8(char_info->EyeColor1);
			buffer.WriteUInt8(char_info->EyeColor2);
			buffer.WriteUInt8(char_info->HairStyle);
			buffer.WriteUInt8(char_info->Beard);
			buffer.WriteUInt8(char_info->GoHome);
			buffer.WriteUInt8(char_info->Tutorial);
			buffer.WriteUInt32(char_info->DrakkinHeritage);
			buffer.WriteUInt8(0); //seen 0
			buffer.WriteUInt8(0); //seen 0
			buffer.WriteUInt32(char_info->LastLogin);
			buffer.WriteUInt32(0); //last login might just be 64bit now
			buffer.WriteUInt32(2590000 + i); //unique character id?
			buffer.WriteUInt32(104);
		}

		buffer.WriteUInt8(0);

		__packet->size = buffer.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buffer.buffer(), __packet->size);
		FINISH_ENCODE();
	}

	// DECODE methods
	
} /*Larion*/
