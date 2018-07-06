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
#include "titanium.h"
#include "../opcodemgr.h"

#include "../eq_stream_ident.h"
#include "../crc32.h"
#include "../races.h"

#include "../eq_packet_structs.h"
#include "../misc_functions.h"
#include "../string_util.h"
#include "../item_instance.h"
#include "titanium_structs.h"

#include <sstream>


namespace Titanium
{
	static const char *name = "Titanium";
	static OpcodeManager *opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(EQEmu::OutBuffer& ob, const EQEmu::ItemInstance *inst, int16 slot_id_in, uint8 depth);

	// server to client inventory location converters
	static inline int16 ServerToTitaniumSlot(uint32 serverSlot);
	static inline int16 ServerToTitaniumCorpseSlot(uint32 serverCorpseSlot);

	// client to server inventory location converters
	static inline uint32 TitaniumToServerSlot(int16 titaniumSlot);
	static inline uint32 TitaniumToServerCorpseSlot(int16 titaniumCorpseSlot);

	// server to client say link converter
	static inline void ServerToTitaniumSayLink(std::string& titaniumSayLink, const std::string& serverSayLink);

	// client to server say link converter
	static inline void TitaniumToServerSayLink(std::string& serverSayLink, const std::string& titaniumSayLink);

	static inline CastingSlot ServerToTitaniumCastingSlot(EQEmu::CastingSlot slot);
	static inline EQEmu::CastingSlot TitaniumToServerCastingSlot(CastingSlot slot, uint32 itemlocation);

	static inline int ServerToTitaniumBuffSlot(int index);
	static inline int TitaniumToServerBuffSlot(int index);

	void Register(EQStreamIdentifier &into)
	{
		auto Config = EQEmuConfig::get();
		//create our opcode manager if we havent already
		if (opcodes == nullptr) {
			//TODO: get this file name from the config file
			std::string opfile = Config->PatchDir;
			opfile += "patch_";
			opfile += name;
			opfile += ".conf";
			//load up the opcode manager.
			//TODO: figure out how to support shared memory with multiple patches...
			opcodes = new RegularOpcodeManager();
			if (!opcodes->LoadOpcodes(opfile.c_str())) {
				Log(Logs::General, Logs::Netcode, "[OPCODES] Error loading opcodes file %s. Not registering patch %s.", opfile.c_str(), name);
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



		Log(Logs::General, Logs::Netcode, "[IDENTIFY] Registered patch %s", name);
	}

	void Reload()
	{
		//we have a big problem to solve here when we switch back to shared memory
		//opcode managers because we need to change the manager pointer, which means
		//we need to go to every stream and replace it's manager.

		if (opcodes != nullptr) {
			//TODO: get this file name from the config file
			auto Config = EQEmuConfig::get();
			std::string opfile = Config->PatchDir;
			opfile += "patch_";
			opfile += name;
			opfile += ".conf";
			if (!opcodes->ReloadOpcodes(opfile.c_str())) {
				Log(Logs::General, Logs::Netcode, "[OPCODES] Error reloading opcodes file %s for patch %s.", opfile.c_str(), name);
				return;
			}
			Log(Logs::General, Logs::Netcode, "[OPCODES] Reloaded opcodes for patch %s", name);
		}
	}

	Strategy::Strategy() : StructStrategy()
	{
		//all opcodes default to passthrough.
#include "ss_register.h"
#include "titanium_ops.h"
	}

	std::string Strategy::Describe() const
	{
		std::string r;
		r += "Patch ";
		r += name;
		return(r);
	}

	const EQEmu::versions::ClientVersion Strategy::ClientVersion() const
	{
		return EQEmu::versions::ClientVersion::Titanium;
	}

#include "ss_define.h"

// ENCODE methods
	EAT_ENCODE(OP_GuildMemberLevelUpdate); // added ;

	EAT_ENCODE(OP_ZoneServerReady); // added ;

	ENCODE(OP_Action)
	{
		ENCODE_LENGTH_EXACT(Action_Struct);
		SETUP_DIRECT_ENCODE(Action_Struct, structs::Action_Struct);

		OUT(target);
		OUT(source);
		OUT(level);
		OUT(instrument_mod);
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);
		OUT(type);
		//OUT(damage);
		OUT(spell);
		OUT(spell_level);
		OUT(effect_flag); // if this is 4, a buff icon is made

		FINISH_ENCODE();
	}

	ENCODE(OP_AdventureMerchantSell)
	{
		ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
		SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		eq->unknown000 = 1;
		OUT(npcid);
		eq->slot = ServerToTitaniumSlot(emu->slot);
		OUT(charges);
		OUT(sell_price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ApplyPoison)
	{
		ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
		SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		eq->inventorySlot = ServerToTitaniumSlot(emu->inventorySlot);
		OUT(success);

		FINISH_ENCODE();
	}

	ENCODE(OP_BazaarSearch)
	{
		if (((*p)->size == sizeof(BazaarReturnDone_Struct)) || ((*p)->size == sizeof(BazaarWelcome_Struct))) {

			EQApplicationPacket *in = *p;
			*p = nullptr;
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		BazaarSearchResults_Struct *emu = (BazaarSearchResults_Struct *)__emu_buffer;

		//determine and verify length
		int entrycount = in->size / sizeof(BazaarSearchResults_Struct);
		if (entrycount == 0 || (in->size % sizeof(BazaarSearchResults_Struct)) != 0) {
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(BazaarSearchResults_Struct));
			delete in;
			return;
		}

		//make the EQ struct.
		in->size = sizeof(structs::BazaarSearchResults_Struct)*entrycount;
		in->pBuffer = new unsigned char[in->size];
		structs::BazaarSearchResults_Struct *eq = (structs::BazaarSearchResults_Struct *) in->pBuffer;

		//zero out the packet. We could avoid this memset by setting all fields (including unknowns) in the loop.
		memset(in->pBuffer, 0, in->size);

		for (int i = 0; i < entrycount; i++, eq++, emu++) {
			eq->Beginning.Action = emu->Beginning.Action;
			eq->Beginning.Unknown001 = emu->Beginning.Unknown001;
			eq->Beginning.Unknown002 = emu->Beginning.Unknown002;
			eq->NumItems = emu->NumItems;
			eq->SerialNumber = emu->SerialNumber;
			eq->SellerID = emu->SellerID;
			eq->Cost = emu->Cost;
			eq->ItemStat = emu->ItemStat;
			strcpy(eq->ItemName, emu->ItemName);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_BecomeTrader)
	{
		ENCODE_LENGTH_EXACT(BecomeTrader_Struct);
		SETUP_DIRECT_ENCODE(BecomeTrader_Struct, structs::BecomeTrader_Struct);

		OUT(ID);
		OUT(Code);

		FINISH_ENCODE();
	}

	ENCODE(OP_Buff)
	{
		ENCODE_LENGTH_EXACT(SpellBuffPacket_Struct);
		SETUP_DIRECT_ENCODE(SpellBuffPacket_Struct, structs::SpellBuffPacket_Struct);

		OUT(entityid);
		OUT(buff.effect_type);
		OUT(buff.level);
		OUT(buff.bard_modifier);
		OUT(buff.spellid);
		OUT(buff.duration);
		OUT(buff.counters);
		OUT(buff.player_id);
		eq->slotid = ServerToTitaniumBuffSlot(emu->slotid);
		OUT(bufffade);

		FINISH_ENCODE();
	}

	ENCODE(OP_ChannelMessage)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		ChannelMessage_Struct *emu = (ChannelMessage_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		std::string old_message = emu->message;
		std::string new_message;
		ServerToTitaniumSayLink(new_message, old_message);

		in->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		memcpy(OutBuffer, __emu_buffer, sizeof(ChannelMessage_Struct));

		OutBuffer += sizeof(ChannelMessage_Struct);

		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_CharInventory)
	{
		//consume the packet
		EQApplicationPacket* in = *p;
		*p = nullptr;

		//store away the emu struct
		uchar* __emu_buffer = in->pBuffer;

		int itemcount = in->size / sizeof(EQEmu::InternalSerializedItem_Struct);
		if (itemcount == 0 || (in->size % sizeof(EQEmu::InternalSerializedItem_Struct)) != 0) {
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(EQEmu::InternalSerializedItem_Struct));
			delete in;
			return;
		}

		EQEmu::InternalSerializedItem_Struct* eq = (EQEmu::InternalSerializedItem_Struct*)in->pBuffer;

		//do the transform...
		EQEmu::OutBuffer ob;
		EQEmu::OutBuffer::pos_type last_pos = ob.tellp();

		for (int r = 0; r < itemcount; r++, eq++) {
			SerializeItem(ob, (const EQEmu::ItemInstance*)eq->inst, eq->slot_id, 0);
			if (ob.tellp() == last_pos)
				Log(Logs::General, Logs::Netcode, "[STRUCTS] Serialization failed on item slot %d during OP_CharInventory.  Item skipped.", eq->slot_id);
			
			last_pos = ob.tellp();
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();
		
		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Damage)
	{
		ENCODE_LENGTH_EXACT(CombatDamage_Struct);
		SETUP_DIRECT_ENCODE(CombatDamage_Struct, structs::CombatDamage_Struct);

		OUT(target);
		OUT(source);
		OUT(type);
		OUT(spellid);
		OUT(damage);
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteCharge) { ENCODE_FORWARD(OP_MoveItem); }

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToTitaniumSlot(emu->from_slot);
		eq->to_slot = ServerToTitaniumSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteSpawn)
	{
		SETUP_DIRECT_ENCODE(DeleteSpawn_Struct, structs::DeleteSpawn_Struct);

		OUT(spawn_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzCompass)
	{
		SETUP_VAR_ENCODE(ExpeditionCompass_Struct);
		ALLOC_VAR_ENCODE(structs::ExpeditionCompass_Struct, sizeof(structs::ExpeditionInfo_Struct) + sizeof(structs::ExpeditionCompassEntry_Struct) * emu->count);

		OUT(count);

		for (uint32 i = 0; i < emu->count; ++i)
		{
			OUT(entries[i].x);
			OUT(entries[i].y);
			OUT(entries[i].z);
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionEndsWarning)
	{
		ENCODE_LENGTH_EXACT(ExpeditionExpireWarning);
		SETUP_DIRECT_ENCODE(ExpeditionExpireWarning, structs::ExpeditionExpireWarning);

		OUT(minutes_remaining);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionInfo)
	{
		ENCODE_LENGTH_EXACT(ExpeditionInfo_Struct);
		SETUP_DIRECT_ENCODE(ExpeditionInfo_Struct, structs::ExpeditionInfo_Struct);

		OUT(max_players);
		eq->enabled_max = 1;
		strcpy(eq->expedition_name, emu->expedition_name);
		strcpy(eq->leader_name, emu->leader_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionList)
	{
		SETUP_VAR_ENCODE(ExpeditionLockoutList_Struct);

		std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
		uint32 client_id = 0;
		uint8 null_term = 0;
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&emu->count, sizeof(uint32));
		for (uint32 i = 0; i < emu->count; ++i)
		{
			ss.write(emu->entries[i].expedition, strlen(emu->entries[i].expedition));
			ss.write((const char*)&null_term, sizeof(char));
			ss.write((const char*)&emu->entries[i].time_left, sizeof(uint32));
			ss.write((const char*)&client_id, sizeof(uint32));
			ss.write(emu->entries[i].expedition_event, strlen(emu->entries[i].expedition_event));
			ss.write((const char*)&null_term, sizeof(char));
		}

		__packet->size = ss.str().length();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, ss.str().c_str(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzJoinExpeditionConfirm)
	{
		ENCODE_LENGTH_EXACT(ExpeditionJoinPrompt_Struct);
		SETUP_DIRECT_ENCODE(ExpeditionJoinPrompt_Struct, structs::ExpeditionJoinPrompt_Struct);

		strcpy(eq->expedition_name, emu->expedition_name);
		strcpy(eq->player_name, emu->player_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzLeaderStatus)
	{
		SETUP_VAR_ENCODE(ExpeditionLeaderSet_Struct);

		std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
		uint32 client_id = 0;
		uint8 null_term = 0;

		ss.write((const char*)&client_id, sizeof(uint32));
		//ss.write((const char*)&client_id, sizeof(uint32));
		ss.write(emu->leader_name, strlen(emu->leader_name));
		ss.write((const char*)&null_term, sizeof(char));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));//0xffffffff
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&client_id, sizeof(uint32));//1
		ss.write((const char*)&client_id, sizeof(uint32));

		__packet->size = ss.str().length();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, ss.str().c_str(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberList)
	{
		SETUP_VAR_ENCODE(ExpeditionMemberList_Struct);

		std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

		uint32 client_id = 0;
		uint8 null_term = 0;
		ss.write((const char*)&client_id, sizeof(uint32));
		ss.write((const char*)&emu->count, sizeof(uint32));
		for (uint32 i = 0; i < emu->count; ++i)
		{
			ss.write(emu->entries[i].name, strlen(emu->entries[i].name));
			ss.write((const char*)&null_term, sizeof(char));
			ss.write((const char*)&emu->entries[i].status, sizeof(char));
		}

		__packet->size = ss.str().length();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, ss.str().c_str(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_Emote)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		Emote_Struct *emu = (Emote_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		std::string old_message = emu->message;
		std::string new_message;
		ServerToTitaniumSayLink(new_message, old_message);

		//if (new_message.length() > 512) // length restricted in packet building function due vari-length name size (no nullterm)
		//	new_message = new_message.substr(0, 512);

		in->size = new_message.length() + 5;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->type);
		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_FormattedMessage)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		FormattedMessage_Struct *emu = (FormattedMessage_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		char *old_message_ptr = (char *)in->pBuffer;
		old_message_ptr += sizeof(FormattedMessage_Struct);

		std::string old_message_array[9];

		for (int i = 0; i < 9; ++i) {
			if (*old_message_ptr == 0) { break; }
			old_message_array[i] = old_message_ptr;
			old_message_ptr += old_message_array[i].length() + 1;
		}

		uint32 new_message_size = 0;
		std::string new_message_array[9];

		for (int i = 0; i < 9; ++i) {
			if (old_message_array[i].length() == 0) { break; }
			ServerToTitaniumSayLink(new_message_array[i], old_message_array[i]);
			new_message_size += new_message_array[i].length() + 1;
		}

		in->size = sizeof(FormattedMessage_Struct) + new_message_size + 1;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->unknown0);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->string_id);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->type);

		for (int i = 0; i < 9; ++i) {
			if (new_message_array[i].length() == 0) { break; }
			VARSTRUCT_ENCODE_STRING(OutBuffer, new_message_array[i].c_str());
		}

		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GroundSpawn)
	{
		// We are not encoding the spawn_id field here, but it doesn't appear to matter.
		//
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Object_Struct *emu = (Object_Struct *)__emu_buffer;

		in->size = strlen(emu->object_name) + sizeof(structs::Object_Struct) - 1;
		in->pBuffer = new unsigned char[in->size];

		structs::Object_Struct *eq = (structs::Object_Struct *) in->pBuffer;

		eq->drop_id = emu->drop_id;
		eq->heading = emu->heading;
		eq->linked_list_addr[0] = 0;
		eq->linked_list_addr[1] = 0;
		strcpy(eq->object_name, emu->object_name);
		eq->object_type = emu->object_type;
		eq->spawn_id = 0;
		eq->unknown008[0] = 0;
		eq->unknown008[1] = 0;
		eq->unknown020 = 0;
		eq->unknown024 = 0;
		eq->unknown076 = 0;
		eq->unknown084 = 0xffffffff;
		eq->z = emu->z;
		eq->x = emu->x;
		eq->y = emu->y;
		eq->zone_id = emu->zone_id;
		eq->zone_instance = emu->zone_instance;


		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GuildMemberList)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Internal_GuildMembers_Struct *emu = (Internal_GuildMembers_Struct *)in->pBuffer;

		//make a new EQ buffer.
		uint32 pnl = strlen(emu->player_name);
		uint32 length = sizeof(structs::GuildMembers_Struct) + pnl +
			emu->count*sizeof(structs::GuildMemberEntry_Struct)
			+ emu->name_length + emu->note_length;
		in->pBuffer = new uint8[length];
		in->size = length;
		//no memset since we fill every byte.

		uint8 *buffer;
		buffer = in->pBuffer;

		//easier way to setup GuildMembers_Struct
		//set prefix name
		strcpy((char *)buffer, emu->player_name);
		buffer += pnl;
		*buffer = '\0';
		buffer++;

		//add member count.
		*((uint32 *)buffer) = htonl(emu->count);
		buffer += sizeof(uint32);

		if (emu->count > 0) {
			Internal_GuildMemberEntry_Struct *emu_e = emu->member;
			const char *emu_name = (const char *)(__emu_buffer +
				sizeof(Internal_GuildMembers_Struct)+ //skip header
				emu->count * sizeof(Internal_GuildMemberEntry_Struct)	//skip static length member data
				);
			const char *emu_note = (emu_name +
				emu->name_length + //skip name contents
				emu->count	//skip string terminators
				);

			structs::GuildMemberEntry_Struct *e = (structs::GuildMemberEntry_Struct *) buffer;

			uint32 r;
			for (r = 0; r < emu->count; r++, emu_e++) {

				//the order we set things here must match the struct

				//nice helper macro
				/*#define SlideStructString(field, str) \
				strcpy(e->field, str.c_str()); \
				e = (GuildMemberEntry_Struct *) ( ((uint8 *)e) + str.length() )*/
#define SlideStructString(field, str) \
			{ \
				int sl = strlen(str); \
				memcpy(e->field, str, sl+1); \
				e = (structs::GuildMemberEntry_Struct *) ( ((uint8 *)e) + sl ); \
				str += sl + 1; \
			}
#define PutFieldN(field) e->field = htonl(emu_e->field)

				SlideStructString(name, emu_name);
				PutFieldN(level);
				PutFieldN(banker);
				PutFieldN(class_);
				PutFieldN(rank);
				PutFieldN(time_last_on);
				PutFieldN(tribute_enable);
				PutFieldN(total_tribute);
				PutFieldN(last_tribute);
				e->unknown_one = htonl(1);
				SlideStructString(public_note, emu_note);
				e->zoneinstance = 0;
				e->zone_id = htons(emu_e->zone_id);

#undef SlideStructString
#undef PutFieldN

				e++;
			}
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Illusion)
	{
		ENCODE_LENGTH_EXACT(Illusion_Struct);
		SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);

		OUT(spawnid);
		OUT_str(charname);

		if (emu->race > 473)
			eq->race = 1;
		else
			OUT(race);

		OUT(gender);
		OUT(texture);
		OUT(helmtexture);
		OUT(face);
		OUT(hairstyle);
		OUT(haircolor);
		OUT(beard);
		OUT(beardcolor);
		OUT(size);
		/*
		//Test code for identifying the structure
		uint8 ofs;
		uint8 val;
		ofs = emu->texture;
		val = emu->face;
		((uint8*)eq)[ofs % 168] = val;
		*/

		FINISH_ENCODE();
	}

	ENCODE(OP_InspectAnswer)
	{
		ENCODE_LENGTH_EXACT(InspectResponse_Struct);
		SETUP_DIRECT_ENCODE(InspectResponse_Struct, structs::InspectResponse_Struct);

		OUT(TargetID);
		OUT(playerid);

		int r;
		for (r = 0; r <= 20; r++) {
			strn0cpy(eq->itemnames[r], emu->itemnames[r], sizeof(eq->itemnames[r]));
		}

		// move arrow item down to last element in titanium array
		strn0cpy(eq->itemnames[21], emu->itemnames[22], sizeof(eq->itemnames[21]));

		int k;
		for (k = 0; k <= 20; k++) {
			OUT(itemicons[k]);
		}

		// move arrow icon down to last element in titanium array
		eq->itemicons[21] = emu->itemicons[22];
		strn0cpy(eq->text, emu->text, sizeof(eq->text));

		FINISH_ENCODE();
	}

	ENCODE(OP_InspectRequest)
	{
		ENCODE_LENGTH_EXACT(Inspect_Struct);
		SETUP_DIRECT_ENCODE(Inspect_Struct, structs::Inspect_Struct);

		OUT(TargetID);
		OUT(PlayerID);

		FINISH_ENCODE();
	}

	ENCODE(OP_ItemLinkResponse) { ENCODE_FORWARD(OP_ItemPacket); }

	ENCODE(OP_ItemPacket)
	{
		//consume the packet
		EQApplicationPacket* in = *p;
		*p = nullptr;

		//store away the emu struct
		uchar* __emu_buffer = in->pBuffer;
		
		EQEmu::InternalSerializedItem_Struct* int_struct = (EQEmu::InternalSerializedItem_Struct*)(&__emu_buffer[4]);

		EQEmu::OutBuffer ob;
		EQEmu::OutBuffer::pos_type last_pos = ob.tellp();

		ob.write((const char*)__emu_buffer, 4);

		SerializeItem(ob, (const EQEmu::ItemInstance*)int_struct->inst, int_struct->slot_id, 0);
		if (ob.tellp() == last_pos) {
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Serialization failed on item slot %d.", int_struct->slot_id);
			delete in;
			return;
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();
		
		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_LeadershipExpUpdate)
	{
		SETUP_DIRECT_ENCODE(LeadershipExpUpdate_Struct, structs::LeadershipExpUpdate_Struct);

		OUT(group_leadership_exp);
		OUT(group_leadership_points);
		OUT(raid_leadership_exp);
		OUT(raid_leadership_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_LFGuild)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		uint32 Command = in->ReadUInt32();

		if (Command != 0)
		{
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(structs::LFGuild_PlayerToggle_Struct));

		memcpy(outapp->pBuffer, in->pBuffer, sizeof(structs::LFGuild_PlayerToggle_Struct));

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToTitaniumCorpseSlot(emu->slot_id);
		OUT(auto_loot);

		FINISH_ENCODE();
	}

	ENCODE(OP_MemorizeSpell)
	{
		ENCODE_LENGTH_EXACT(MemorizeSpell_Struct);
		SETUP_DIRECT_ENCODE(MemorizeSpell_Struct, structs::MemorizeSpell_Struct);

		// Since HT/LoH are translated up, we need to translate down only for memSpellSpellbar case
		if (emu->scribing == 3)
			eq->slot = static_cast<uint32>(ServerToTitaniumCastingSlot(static_cast<EQEmu::CastingSlot>(emu->slot)));
		else
			OUT(slot);
		OUT(spell_id);
		OUT(scribing);

		FINISH_ENCODE();
	}

	ENCODE(OP_MoveItem)
	{
		ENCODE_LENGTH_EXACT(MoveItem_Struct);
		SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);

		eq->from_slot = ServerToTitaniumSlot(emu->from_slot);
		eq->to_slot = ServerToTitaniumSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_NewSpawn) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_OnLevelMessage)
	{
		ENCODE_LENGTH_EXACT(OnLevelMessage_Struct);
		SETUP_DIRECT_ENCODE(OnLevelMessage_Struct, structs::OnLevelMessage_Struct);

		OUT_str(Title);
		OUT_str(Text);
		OUT(Buttons);
		OUT(Duration);
		OUT(PopupID);

		eq->unknown4236 = 0x00000000;
		eq->unknown4240 = 0xffffffff;

		FINISH_ENCODE();
	}

	ENCODE(OP_PetBuffWindow)
	{
		ENCODE_LENGTH_EXACT(PetBuff_Struct);
		SETUP_DIRECT_ENCODE(PetBuff_Struct, PetBuff_Struct);

		OUT(petid);
		OUT(buffcount);

		int EQBuffSlot = 0; // do we really want to shuffle them around like this?

		for (uint32 EmuBuffSlot = 0; EmuBuffSlot < PET_BUFF_COUNT; ++EmuBuffSlot)
		{
			if (emu->spellid[EmuBuffSlot])
			{
				eq->spellid[EQBuffSlot] = emu->spellid[EmuBuffSlot];
				eq->ticsremaining[EQBuffSlot++] = emu->ticsremaining[EmuBuffSlot];
			}
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_PlayerProfile)
	{
		SETUP_DIRECT_ENCODE(PlayerProfile_Struct, structs::PlayerProfile_Struct);

		uint32 r;

		eq->available_slots = 0xffffffff;
		memset(eq->unknown4184, 0xff, sizeof(eq->unknown4184));
		memset(eq->unknown04396, 0xff, sizeof(eq->unknown04396));

		//	OUT(checksum);
		OUT(gender);
		OUT(race);
		OUT(class_);
		//	OUT(unknown00016);
		OUT(level);
		eq->level1 = emu->level;
		//	OUT(unknown00022[2]);
		for (r = 0; r < 5; r++) {
			OUT(binds[r].zoneId);
			OUT(binds[r].x);
			OUT(binds[r].y);
			OUT(binds[r].z);
			OUT(binds[r].heading);
		}
		OUT(deity);
		OUT(intoxication);
		OUT_array(spellSlotRefresh, structs::MAX_PP_MEMSPELL);
		OUT(abilitySlotRefresh);
		OUT(haircolor);
		OUT(beardcolor);
		OUT(eyecolor1);
		OUT(eyecolor2);
		OUT(hairstyle);
		OUT(beard);
		//	OUT(unknown00178[10]);
		for (r = EQEmu::textures::textureBegin; r < EQEmu::textures::materialCount; r++) {
			OUT(item_material.Slot[r].Material);
			OUT(item_tint.Slot[r].Color);
		}
		//	OUT(unknown00224[48]);
		for (r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
			OUT(aa_array[r].AA);
			OUT(aa_array[r].value);
		}
		//	OUT(unknown02220[4]);
		OUT(points);
		OUT(mana);
		OUT(cur_hp);
		OUT(STR);
		OUT(STA);
		OUT(CHA);
		OUT(DEX);
		OUT(INT);
		OUT(AGI);
		OUT(WIS);
		OUT(face);
		//	OUT(unknown02264[47]);
		OUT_array(spell_book, structs::MAX_PP_SPELLBOOK);
		//	OUT(unknown4184[448]);
		OUT_array(mem_spells, structs::MAX_PP_MEMSPELL);
		//	OUT(unknown04396[32]);
		OUT(platinum);
		OUT(gold);
		OUT(silver);
		OUT(copper);
		OUT(platinum_cursor);
		OUT(gold_cursor);
		OUT(silver_cursor);
		OUT(copper_cursor);

		OUT_array(skills, structs::MAX_PP_SKILL);	// 1:1 direct copy (100 dword)
		OUT_array(InnateSkills, structs::MAX_PP_INNATE_SKILL);	// 1:1 direct copy (25 dword)

		//	OUT(unknown04760[236]);
		OUT(toxicity);
		OUT(thirst_level);
		OUT(hunger_level);
		for (r = 0; r < structs::BUFF_COUNT; r++) {
			OUT(buffs[r].effect_type);
			OUT(buffs[r].level);
			OUT(buffs[r].bard_modifier);
			OUT(buffs[r].unknown003);
			OUT(buffs[r].spellid);
			OUT(buffs[r].duration);
			OUT(buffs[r].counters);
			OUT(buffs[r].player_id);
		}
		for (r = 0; r < structs::MAX_PP_DISCIPLINES; r++) {
			OUT(disciplines.values[r]);
		}
		//	OUT(unknown05008[360]);
		//	OUT_array(recastTimers, structs::MAX_RECAST_TYPES);
		OUT(endurance);
		OUT(aapoints_spent);
		OUT(aapoints);

		//	OUT(unknown06160[4]);

		// Copy bandoliers where server and client indexes converge
		for (r = 0; r < EQEmu::profile::BANDOLIERS_SIZE && r < profile::BANDOLIERS_SIZE; ++r) {
			OUT_str(bandoliers[r].Name);
			for (uint32 k = 0; k < profile::BANDOLIER_ITEM_COUNT; ++k) { // Will need adjusting if 'server != client' is ever true
				OUT(bandoliers[r].Items[k].ID);
				OUT(bandoliers[r].Items[k].Icon);
				OUT_str(bandoliers[r].Items[k].Name);
			}
		}
		// Nullify bandoliers where server and client indexes diverge, with a client bias
		for (r = EQEmu::profile::BANDOLIERS_SIZE; r < profile::BANDOLIERS_SIZE; ++r) {
			eq->bandoliers[r].Name[0] = '\0';
			for (uint32 k = 0; k < profile::BANDOLIER_ITEM_COUNT; ++k) { // Will need adjusting if 'server != client' is ever true
				eq->bandoliers[r].Items[k].ID = 0;
				eq->bandoliers[r].Items[k].Icon = 0;
				eq->bandoliers[r].Items[k].Name[0] = '\0';
			}
		}

		//	OUT(unknown07444[5120]);

		// Copy potion belt where server and client indexes converge
		for (r = 0; r < EQEmu::profile::POTION_BELT_SIZE && r < profile::POTION_BELT_SIZE; ++r) {
			OUT(potionbelt.Items[r].ID);
			OUT(potionbelt.Items[r].Icon);
			OUT_str(potionbelt.Items[r].Name);
		}
		// Nullify potion belt where server and client indexes diverge, with a client bias
		for (r = EQEmu::profile::POTION_BELT_SIZE; r < profile::POTION_BELT_SIZE; ++r) {
			eq->potionbelt.Items[r].ID = 0;
			eq->potionbelt.Items[r].Icon = 0;
			eq->potionbelt.Items[r].Name[0] = '\0';
		}

		//	OUT(unknown12852[8]);
		//	OUT(unknown12864[76]);

		OUT_str(name);
		OUT_str(last_name);
		OUT(guild_id);
		OUT(birthday);
		OUT(lastlogin);
		OUT(timePlayedMin);
		OUT(pvp);
		OUT(anon);
		OUT(gm);
		OUT(guildrank);
		OUT(guildbanker);
		//	OUT(unknown13054[8]);
		OUT(exp);
		//	OUT(unknown13072[12]);
		OUT(timeentitledonaccount);
		OUT_array(languages, structs::MAX_PP_LANGUAGE);
		//	OUT(unknown13109[7]);
		OUT(x);
		OUT(y);
		OUT(z);
		OUT(heading);
		//	OUT(unknown13132[4]);
		OUT(platinum_bank);
		OUT(gold_bank);
		OUT(silver_bank);
		OUT(copper_bank);
		OUT(platinum_shared);
		//	OUT(unknown13156[84]);
		OUT(expansions);
		//	OUT(unknown13244[12]);
		OUT(autosplit);
		//	OUT(unknown13260[16]);
		OUT(zone_id);
		OUT(zoneInstance);
		for (r = 0; r < structs::MAX_GROUP_MEMBERS; r++) {
			OUT_str(groupMembers[r]);
		}
		strcpy(eq->groupLeader, emu->groupMembers[0]);
		//	OUT_str(groupLeader);
		//	OUT(unknown13728[660]);
		OUT(entityid);
		OUT(leadAAActive);
		//	OUT(unknown14392[4]);
		OUT(ldon_points_guk);
		OUT(ldon_points_mir);
		OUT(ldon_points_mmc);
		OUT(ldon_points_ruj);
		OUT(ldon_points_tak);
		OUT(ldon_points_available);
		//	OUT(unknown14420[132]);
		OUT(tribute_time_remaining);
		OUT(career_tribute_points);
		//	OUT(unknown7208);
		OUT(tribute_points);
		//	OUT(unknown7216);
		OUT(tribute_active);
		for (r = 0; r < structs::MAX_PLAYER_TRIBUTES; r++) {
			OUT(tributes[r].tribute);
			OUT(tributes[r].tier);
		}
		//	OUT(unknown14616[8]);
		OUT(group_leadership_exp);
		OUT(raid_leadership_exp);
		OUT(group_leadership_points);
		OUT(raid_leadership_points);
		OUT_array(leader_abilities.ranks, structs::MAX_LEADERSHIP_AA_ARRAY);
		//	OUT(unknown14772[128]);
		OUT(air_remaining);
		OUT(PVPKills);
		OUT(PVPDeaths);
		OUT(PVPCurrentPoints);
		OUT(PVPCareerPoints);
		OUT(PVPBestKillStreak);
		OUT(PVPWorstDeathStreak);
		OUT(PVPCurrentKillStreak);
		//	OUT(unknown14932[4580]);
		OUT(expAA);
		//	OUT(unknown19516[40]);
		OUT(currentRadCrystals);
		OUT(careerRadCrystals);
		OUT(currentEbonCrystals);
		OUT(careerEbonCrystals);
		OUT(groupAutoconsent);
		OUT(raidAutoconsent);
		OUT(guildAutoconsent);
		//	OUT(unknown19575[5]);
		eq->level3 = emu->level;
		eq->showhelm = emu->showhelm;
		//	OUT(unknown19584[4]);
		//	OUT(unknown19588);


		const uint8 bytes[] = {
			0x78, 0x03, 0x00, 0x00, 0x1A, 0x04, 0x00, 0x00, 0x1A, 0x04, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
			0x19, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
			0x0F, 0x00, 0x00, 0x00, 0x1F, 0x85, 0xEB, 0x3E, 0x33, 0x33, 0x33, 0x3F, 0x09, 0x00, 0x00, 0x00,
			0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x14
		};
		memcpy(eq->unknown12864, bytes, sizeof(bytes));

		//set the checksum...
		CRC32::SetEQChecksum(__packet->pBuffer, sizeof(structs::PlayerProfile_Struct) - 4);

		FINISH_ENCODE();
	}

	ENCODE(OP_ReadBook)
	{
		// no apparent slot translation needed
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;

		BookText_Struct *emu_BookText_Struct = (BookText_Struct *)__emu_buffer;

		in->size = sizeof(structs::BookText_Struct) + strlen(emu_BookText_Struct->booktext);
		in->pBuffer = new unsigned char[in->size];

		structs::BookText_Struct *eq_BookText_Struct = (structs::BookText_Struct*)in->pBuffer;

		eq_BookText_Struct->window = emu_BookText_Struct->window;
		eq_BookText_Struct->type = emu_BookText_Struct->type;
		strcpy(eq_BookText_Struct->booktext, emu_BookText_Struct->booktext);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_RespondAA)
	{
		ENCODE_LENGTH_EXACT(AATable_Struct);
		SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

		unsigned int r;
		for (r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
			OUT(aa_list[r].AA);
			OUT(aa_list[r].value);
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_SendAATable)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;
		AARankInfo_Struct *emu = (AARankInfo_Struct*)inapp->pBuffer;

		auto outapp = new EQApplicationPacket(
		    OP_SendAATable, sizeof(structs::SendAA_Struct) + emu->total_effects * sizeof(structs::AA_Ability));
		structs::SendAA_Struct *eq = (structs::SendAA_Struct*)outapp->pBuffer;

		inapp->SetReadPosition(sizeof(AARankInfo_Struct));
		outapp->SetWritePosition(sizeof(structs::SendAA_Struct));

		eq->id = emu->id;
		eq->unknown004 = 1;
		eq->id = emu->id;
		eq->hotkey_sid = emu->upper_hotkey_sid;
		eq->hotkey_sid2 = emu->lower_hotkey_sid;
		eq->desc_sid = emu->desc_sid;
		eq->title_sid = emu->title_sid;
		eq->class_type = emu->level_req;
		eq->cost = emu->cost;
		eq->seq = emu->seq;
		eq->current_level = emu->current_level;
		eq->type = emu->type;
		eq->spellid = emu->spell;
		eq->spell_type = emu->spell_type;
		eq->spell_refresh = emu->spell_refresh;
		eq->classes = emu->classes;
		eq->max_level = emu->max_level;
		eq->last_id = emu->prev_id;
		eq->next_id = emu->next_id;
		eq->cost2 = emu->total_cost;
		eq->total_abilities = emu->total_effects;

		for(auto i = 0; i < eq->total_abilities; ++i) {
			eq->abilities[i].skill_id = inapp->ReadUInt32();
			eq->abilities[i].base1 = inapp->ReadUInt32();
			eq->abilities[i].base2 = inapp->ReadUInt32();
			eq->abilities[i].slot = inapp->ReadUInt32();
		}

		if(emu->total_prereqs > 0) {
			eq->prereq_skill = inapp->ReadUInt32();
			eq->prereq_minpoints = inapp->ReadUInt32();
		}

		dest->FastQueuePacket(&outapp);
		delete inapp;
	}

	ENCODE(OP_SendCharInfo)
	{
		ENCODE_LENGTH_ATLEAST(CharacterSelect_Struct);
		SETUP_DIRECT_ENCODE(CharacterSelect_Struct, structs::CharacterSelect_Struct);

		unsigned char *emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);
		CharacterSelectEntry_Struct *emu_cse = (CharacterSelectEntry_Struct *)nullptr;

		for (size_t index = 0; index < 10; ++index) {
			memset(eq->Name[index], 0, 64);
		}

		// Non character-indexed packet fields
		eq->Unknown830[0] = 0;
		eq->Unknown830[1] = 0;
		eq->Unknown0962[0] = 0;
		eq->Unknown0962[1] = 0;

		size_t char_index = 0;
		for (; char_index < emu->CharCount && char_index < 8; ++char_index) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;

			eq->Race[char_index] = emu_cse->Race;
			if (eq->Race[char_index] > 473)
				eq->Race[char_index] = 1;

			for (int index = 0; index < EQEmu::textures::materialCount; ++index) {
				eq->CS_Colors[char_index].Slot[index].Color = emu_cse->Equip[index].Color;
			}

			eq->BeardColor[char_index] = emu_cse->BeardColor;
			eq->HairStyle[char_index] = emu_cse->HairStyle;

			for (int index = 0; index < EQEmu::textures::materialCount; ++index) {
				eq->Equip[char_index].Slot[index].Material = emu_cse->Equip[index].Material;
			}

			eq->SecondaryIDFile[char_index] = emu_cse->SecondaryIDFile;
			eq->Unknown820[char_index] = (uint8)0xFF;
			eq->Deity[char_index] = emu_cse->Deity;
			eq->GoHome[char_index] = emu_cse->GoHome;
			eq->Tutorial[char_index] = emu_cse->Tutorial;
			eq->Beard[char_index] = emu_cse->Beard;
			eq->Unknown902[char_index] = (uint8)0xFF;
			eq->PrimaryIDFile[char_index] = emu_cse->PrimaryIDFile;
			eq->HairColor[char_index] = emu_cse->HairColor;
			eq->Zone[char_index] = emu_cse->Zone;
			eq->Class[char_index] = emu_cse->Class;
			eq->Face[char_index] = emu_cse->Face;

			memcpy(eq->Name[char_index], emu_cse->Name, 64);

			eq->Gender[char_index] = emu_cse->Gender;
			eq->EyeColor1[char_index] = emu_cse->EyeColor1;
			eq->EyeColor2[char_index] = emu_cse->EyeColor2;
			eq->Level[char_index] = emu_cse->Level;

			emu_ptr += sizeof(CharacterSelectEntry_Struct);
		}

		for (; char_index < 10; ++char_index) {
			eq->Race[char_index] = 0;

			for (int index = 0; index < EQEmu::textures::materialCount; ++index) {
				eq->CS_Colors[char_index].Slot[index].Color = 0;
			}

			eq->BeardColor[char_index] = 0;
			eq->HairStyle[char_index] = 0;

			for (int index = 0; index < EQEmu::textures::materialCount; ++index) {
				eq->Equip[char_index].Slot[index].Material = 0;
			}

			eq->SecondaryIDFile[char_index] = 0;
			eq->Unknown820[char_index] = (uint8)0xFF;
			eq->Deity[char_index] = 0;
			eq->GoHome[char_index] = 0;
			eq->Tutorial[char_index] = 0;
			eq->Beard[char_index] = 0;
			eq->Unknown902[char_index] = (uint8)0xFF;
			eq->PrimaryIDFile[char_index] = 0;
			eq->HairColor[char_index] = 0;
			eq->Zone[char_index] = 0;
			eq->Class[char_index] = 0;
			eq->Face[char_index] = 0;

			strncpy(eq->Name[char_index], "<none>", 6);

			eq->Gender[char_index] = 0;
			eq->EyeColor1[char_index] = 0;
			eq->EyeColor2[char_index] = 0;
			eq->Level[char_index] = 0;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopPlayerSell)
	{
		ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		OUT(npcid);
		eq->itemslot = ServerToTitaniumSlot(emu->itemslot);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_SpecialMesg)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		SpecialMesg_Struct *emu = (SpecialMesg_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		std::string old_message = &emu->message[strlen(emu->sayer)];
		std::string new_message;

		ServerToTitaniumSayLink(new_message, old_message);

		//in->size = 3 + 4 + 4 + strlen(emu->sayer) + 1 + 12 + new_message.length() + 1;
		in->size = strlen(emu->sayer) + new_message.length() + 25;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->header[0]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->header[1]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->header[2]);

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->msg_type);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->target_spawn_id);

		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->sayer);

		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[0]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[1]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[2]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[3]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[4]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[5]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[6]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[7]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[8]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[9]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[10]);
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, emu->unknown12[11]);

		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_TaskDescription)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;

		char *InBuffer = (char *)in->pBuffer;
		char *block_start = InBuffer;

		InBuffer += sizeof(TaskDescriptionHeader_Struct);
		uint32 title_size = strlen(InBuffer) + 1;
		InBuffer += title_size;
		InBuffer += sizeof(TaskDescriptionData1_Struct);
		uint32 description_size = strlen(InBuffer) + 1;
		InBuffer += description_size;
		InBuffer += sizeof(TaskDescriptionData2_Struct);

		std::string old_message = InBuffer; // start 'Reward' as string
		std::string new_message;
		ServerToTitaniumSayLink(new_message, old_message);

		in->size = sizeof(TaskDescriptionHeader_Struct) + sizeof(TaskDescriptionData1_Struct) +
			sizeof(TaskDescriptionData2_Struct) + sizeof(TaskDescriptionTrailer_Struct) +
			title_size + description_size + new_message.length() + 1;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		memcpy(OutBuffer, block_start, (InBuffer - block_start));
		OutBuffer += (InBuffer - block_start);

		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		InBuffer += strlen(InBuffer) + 1;

		memcpy(OutBuffer, InBuffer, sizeof(TaskDescriptionTrailer_Struct));

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Track)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;
		Track_Struct *emu = (Track_Struct *)__emu_buffer;

		int EntryCount = in->size / sizeof(Track_Struct);

		if (EntryCount == 0 || ((in->size % sizeof(Track_Struct))) != 0)
		{
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Track_Struct));
			delete in;
			return;
		}

		in->size = sizeof(structs::Track_Struct) * EntryCount;
		in->pBuffer = new unsigned char[in->size];
		structs::Track_Struct *eq = (structs::Track_Struct *) in->pBuffer;

		for (int i = 0; i < EntryCount; ++i, ++eq, ++emu)
		{
			OUT(entityid);
			//OUT(padding002);
			OUT(distance);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Trader)
	{
		if ((*p)->size != sizeof(TraderBuy_Struct)) {
			EQApplicationPacket *in = *p;
			*p = nullptr;
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		ENCODE_FORWARD(OP_TraderBuy);
	}

	ENCODE(OP_TraderBuy)
	{
		ENCODE_LENGTH_EXACT(TraderBuy_Struct);
		SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);

		OUT(Action);
		OUT(Price);
		OUT(TraderID);
		memcpy(eq->ItemName, emu->ItemName, sizeof(eq->ItemName));
		OUT(ItemID);
		OUT(Quantity);
		OUT(AlreadySold);

		FINISH_ENCODE();
	}

	ENCODE(OP_TributeItem)
	{
		ENCODE_LENGTH_EXACT(TributeItem_Struct);
		SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

		eq->slot = ServerToTitaniumSlot(emu->slot);
		OUT(quantity);
		OUT(tribute_master_id);
		OUT(tribute_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_VetRewardsAvaliable)
	{
		EQApplicationPacket *inapp = *p;
		unsigned char * __emu_buffer = inapp->pBuffer;

		uint32 count = ((*p)->Size() / sizeof(InternalVeteranReward));
		*p = nullptr;

		auto outapp_create =
		    new EQApplicationPacket(OP_VetRewardsAvaliable, (sizeof(structs::VeteranReward) * count));
		uchar *old_data = __emu_buffer;
		uchar *data = outapp_create->pBuffer;
		for (uint32 i = 0; i < count; ++i)
		{
			structs::VeteranReward *vr = (structs::VeteranReward*)data;
			InternalVeteranReward *ivr = (InternalVeteranReward*)old_data;

			vr->claim_id = ivr->claim_id;
			vr->item.item_id = ivr->items[0].item_id;
			strcpy(vr->item.item_name, ivr->items[0].item_name);

			old_data += sizeof(InternalVeteranReward);
			data += sizeof(structs::VeteranReward);
		}

		dest->FastQueuePacket(&outapp_create);
		delete inapp;
	}

	ENCODE(OP_WearChange)
	{
		ENCODE_LENGTH_EXACT(WearChange_Struct);
		SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);

		OUT(spawn_id);
		OUT(material);
		OUT(color.Color);
		OUT(wear_slot_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneEntry) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_ZoneSpawns)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Spawn_Struct *emu = (Spawn_Struct *)__emu_buffer;

		//determine and verify length
		int entrycount = in->size / sizeof(Spawn_Struct);
		if (entrycount == 0 || (in->size % sizeof(Spawn_Struct)) != 0) {
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
			delete in;
			return;
		}

		//make the EQ struct.
		in->size = sizeof(structs::Spawn_Struct)*entrycount;
		in->pBuffer = new unsigned char[in->size];
		structs::Spawn_Struct *eq = (structs::Spawn_Struct *) in->pBuffer;

		//zero out the packet. We could avoid this memset by setting all fields (including unknowns)
		//in the loop.
		memset(in->pBuffer, 0, in->size);

		//do the transform...
		int r;
		int k;
		for (r = 0; r < entrycount; r++, eq++, emu++) {
			//		eq->unknown0000 = emu->unknown0000;
			eq->gm = emu->gm;
			//		eq->unknown0003 = emu->unknown0003;
			eq->aaitle = emu->aaitle;
			//		eq->unknown0004 = emu->unknown0004;
			eq->anon = emu->anon;
			eq->face = emu->face;
			strcpy(eq->name, emu->name);
			eq->deity = emu->deity;
			//		eq->unknown0073 = emu->unknown0073;
			eq->size = emu->size;
			//		eq->unknown0079 = emu->unknown0079;
			eq->NPC = emu->NPC;
			eq->invis = emu->invis;
			eq->haircolor = emu->haircolor;
			eq->curHp = emu->curHp;
			eq->max_hp = emu->max_hp;
			eq->findable = emu->findable;
			//		eq->unknown0089[5] = emu->unknown0089[5];
			eq->deltaHeading = emu->deltaHeading;
			eq->x = emu->x;
			//		eq->padding0054 = emu->padding0054;
			eq->y = emu->y;
			eq->animation = emu->animation;
			//		eq->padding0058 = emu->padding0058;
			eq->z = emu->z;
			eq->deltaY = emu->deltaY;
			eq->deltaX = emu->deltaX;
			eq->heading = emu->heading;
			//		eq->padding0066 = emu->padding0066;
			eq->deltaZ = emu->deltaZ;
			//		eq->padding0070 = emu->padding0070;
			eq->eyecolor1 = emu->eyecolor1;
			//		eq->unknown0115[24] = emu->unknown0115[24];
			eq->showhelm = emu->showhelm;
			//		eq->unknown0140[4] = emu->unknown0140[4];
			eq->is_npc = emu->is_npc;
			eq->hairstyle = emu->hairstyle;

			//if(emu->gender == 1){
			//	eq->hairstyle = eq->hairstyle == 0xFF ? 0 : eq->hairstyle;
			//}

			eq->beardcolor = emu->beardcolor;
			//		eq->unknown0147[4] = emu->unknown0147[4];
			eq->level = emu->level;
			eq->PlayerState = emu->PlayerState;
			eq->beard = emu->beard;
			strcpy(eq->suffix, emu->suffix);
			eq->petOwnerId = emu->petOwnerId;
			eq->guildrank = emu->guildrank;
			//		eq->unknown0194[3] = emu->unknown0194[3];
			for (k = EQEmu::textures::textureBegin; k < EQEmu::textures::materialCount; k++) {
				eq->equipment.Slot[k].Material = emu->equipment.Slot[k].Material;
				eq->equipment_tint.Slot[k].Color = emu->equipment_tint.Slot[k].Color;
			}
			for (k = 0; k < 8; k++) {
				eq->set_to_0xFF[k] = 0xFF;
			}

			eq->runspeed = emu->runspeed;
			eq->afk = emu->afk;
			eq->guildID = emu->guildID;
			strcpy(eq->title, emu->title);
			//		eq->unknown0274 = emu->unknown0274;
			eq->helm = emu->helm;
			if (emu->race > 473)
				eq->race = 1;
			else
				eq->race = emu->race;
			//		eq->unknown0288 = emu->unknown0288;
			strcpy(eq->lastName, emu->lastName);
			eq->walkspeed = emu->walkspeed;
			//		eq->unknown0328 = emu->unknown0328;
			eq->is_pet = emu->is_pet;
			eq->light = emu->light;
			eq->class_ = emu->class_;
			eq->eyecolor2 = emu->eyecolor2;
			//		eq->unknown0333 = emu->unknown0333;
			eq->flymode = emu->flymode;
			eq->gender = emu->gender;
			eq->bodytype = emu->bodytype;
			//		eq->unknown0336[3] = emu->unknown0336[3];
			eq->equip_chest2 = emu->equip_chest2;
			eq->spawnId = emu->spawnId;
			//		eq->unknown0344[4] = emu->unknown0344[4];
			eq->lfg = emu->lfg;

			/*
			if (emu->face == 99)	      {eq->face = 0;}
			if (emu->eyecolor1 == 99)  {eq->eyecolor1 = 0;}
			if (emu->eyecolor2 == 99)  {eq->eyecolor2 = 0;}
			if (emu->hairstyle == 99)  {eq->hairstyle = 0;}
			if (emu->haircolor == 99)  {eq->haircolor = 0;}
			if (emu->beard == 99)      {eq->beard = 0;}
			if (emu->beardcolor == 99) {eq->beardcolor = 0;}
			*/

		}

		//kill off the emu structure and send the eq packet.
		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

// DECODE methods
	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = TitaniumToServerSlot(eq->slot);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApplyPoison)
	{
		DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
		SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		emu->inventorySlot = TitaniumToServerSlot(eq->inventorySlot);
		IN(success);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AugmentItem)
	{
		DECODE_LENGTH_EXACT(structs::AugmentItem_Struct);
		SETUP_DIRECT_DECODE(AugmentItem_Struct, structs::AugmentItem_Struct);

		emu->container_slot = TitaniumToServerSlot(eq->container_slot);
		emu->augment_slot = eq->augment_slot;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Buff)
	{
		DECODE_LENGTH_EXACT(structs::SpellBuffPacket_Struct);
		SETUP_DIRECT_DECODE(SpellBuffPacket_Struct, structs::SpellBuffPacket_Struct);

		IN(entityid);
		IN(buff.effect_type);
		IN(buff.level);
		IN(buff.bard_modifier);
		IN(buff.spellid);
		IN(buff.duration);
		IN(buff.counters);
		IN(buff.player_id);
		emu->slotid = TitaniumToServerBuffSlot(eq->slotid);
		IN(bufffade);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Bug)
	{
		DECODE_LENGTH_EXACT(structs::BugReport_Struct);
		SETUP_DIRECT_DECODE(BugReport_Struct, structs::BugReport_Struct);

		emu->category_id = EQEmu::bug::CategoryNameToCategoryID(eq->category_name);
		memcpy(emu->category_name, eq, sizeof(structs::BugReport_Struct));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_CastSpell)
	{
		DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
		SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

		emu->slot = static_cast<uint32>(TitaniumToServerCastingSlot(static_cast<CastingSlot>(eq->slot), eq->inventoryslot));
		IN(spell_id);
		emu->inventoryslot = TitaniumToServerSlot(eq->inventoryslot);
		IN(target_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		std::string old_message = (char *)&__eq_buffer[sizeof(ChannelMessage_Struct)];
		std::string new_message;
		TitaniumToServerSayLink(new_message, old_message);

		__packet->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;
		__packet->pBuffer = new unsigned char[__packet->size];

		ChannelMessage_Struct *emu = (ChannelMessage_Struct *)__packet->pBuffer;

		memcpy(emu, __eq_buffer, sizeof(ChannelMessage_Struct));
		strcpy(emu->message, new_message.c_str());

		delete[] __eq_buffer;
	}

	DECODE(OP_CharacterCreate)
	{
		DECODE_LENGTH_EXACT(structs::CharCreate_Struct);
		SETUP_DIRECT_DECODE(CharCreate_Struct, structs::CharCreate_Struct);

		IN(class_);
		IN(beardcolor);
		IN(beard);
		IN(haircolor);
		IN(gender);
		IN(race);
		IN(start_zone);
		IN(hairstyle);
		IN(deity);
		IN(STR);
		IN(STA);
		IN(AGI);
		IN(DEX);
		IN(WIS);
		IN(INT);
		IN(CHA);
		IN(face);
		IN(eyecolor1);
		IN(eyecolor2);
		IN(tutorial);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Consume)
	{
		DECODE_LENGTH_EXACT(structs::Consume_Struct);
		SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);

		emu->slot = TitaniumToServerSlot(eq->slot);
		IN(auto_consumed);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DeleteItem)
	{
		DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
		SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		emu->from_slot = TitaniumToServerSlot(eq->from_slot);
		emu->to_slot =TitaniumToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Emote)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		std::string old_message = (char *)&__eq_buffer[4]; // unknown01 offset
		std::string new_message;
		TitaniumToServerSayLink(new_message, old_message);

		__packet->size = sizeof(Emote_Struct);
		__packet->pBuffer = new unsigned char[__packet->size];

		char *InBuffer = (char *)__packet->pBuffer;

		memcpy(InBuffer, __eq_buffer, 4);
		InBuffer += 4;
		strcpy(InBuffer, new_message.substr(0, 1023).c_str());
		InBuffer[1023] = '\0';

		delete[] __eq_buffer;
	}

	DECODE(OP_FaceChange)
	{
		DECODE_LENGTH_EXACT(structs::FaceChange_Struct);
		SETUP_DIRECT_DECODE(FaceChange_Struct, structs::FaceChange_Struct);
		IN(haircolor);
		IN(beardcolor);
		IN(eyecolor1);
		IN(eyecolor2);
		IN(hairstyle);
		IN(beard);
		IN(face);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_InspectAnswer)
	{
		DECODE_LENGTH_EXACT(structs::InspectResponse_Struct);
		SETUP_DIRECT_DECODE(InspectResponse_Struct, structs::InspectResponse_Struct);

		IN(TargetID);
		IN(playerid);

		int r;
		for (r = 0; r <= 20; r++) {
			strn0cpy(emu->itemnames[r], eq->itemnames[r], sizeof(emu->itemnames[r]));
		}

		// move arrow item up to last element in server array
		strn0cpy(emu->itemnames[21], "", sizeof(emu->itemnames[21]));
		strn0cpy(emu->itemnames[22], eq->itemnames[21], sizeof(emu->itemnames[22]));

		int k;
		for (k = 0; k <= 20; k++) {
			IN(itemicons[k]);
		}

		// move arrow icon up to last element in server array
		emu->itemicons[21] = 0xFFFFFFFF;
		emu->itemicons[22] = eq->itemicons[21];

		strn0cpy(emu->text, eq->text, sizeof(emu->text));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_InspectRequest)
	{
		DECODE_LENGTH_EXACT(structs::Inspect_Struct);
		SETUP_DIRECT_DECODE(Inspect_Struct, structs::Inspect_Struct);

		IN(TargetID);
		IN(PlayerID);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemLinkClick)
	{
		DECODE_LENGTH_EXACT(structs::ItemViewRequest_Struct);
		SETUP_DIRECT_DECODE(ItemViewRequest_Struct, structs::ItemViewRequest_Struct);
		MEMSET_IN(ItemViewRequest_Struct);

		IN(item_id);
		int r;
		for (r = 0; r < 5; r++) {
			IN(augments[r]);
		}
		IN(link_hash);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LFGuild)
	{
		uint32 Command = __packet->ReadUInt32();

		if (Command != 0)
			return;

		SETUP_DIRECT_DECODE(LFGuild_PlayerToggle_Struct, structs::LFGuild_PlayerToggle_Struct);
		memcpy(emu, eq, sizeof(structs::LFGuild_PlayerToggle_Struct));
		memset(emu->Unknown612, 0, sizeof(emu->Unknown612));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LoadSpellSet)
	{
		DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
		SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

		for (int i = 0; i < structs::MAX_PP_MEMSPELL; ++i)
			IN(spell[i]);
		for (int i = structs::MAX_PP_MEMSPELL; i < MAX_PP_MEMSPELL; ++i)
			emu->spell[i] = 0xFFFFFFFF;

		IN(unknown);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LootItem)
	{
		DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
		SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);

		IN(lootee);
		IN(looter);
		emu->slot_id = TitaniumToServerCorpseSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::General, Logs::Netcode, "[Titanium] Moved item from %u to %u", eq->from_slot, eq->to_slot);

		emu->from_slot = TitaniumToServerSlot(eq->from_slot);
		emu->to_slot = TitaniumToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PetCommands)
	{
		DECODE_LENGTH_EXACT(structs::PetCommand_Struct);
		SETUP_DIRECT_DECODE(PetCommand_Struct, structs::PetCommand_Struct);

		switch (eq->command)
		{
		case 1: // back off
			emu->command = 28;
			break;
		case 2: // get lost
			emu->command = 29;
			break;
		case 3: // as you were ???
			emu->command = 4; // fuck it follow
			break;
		case 4: // report HP
			emu->command = 0;
			break;
		case 5: // guard here
			emu->command = 5;
			break;
		case 6: // guard me
			emu->command = 4; // fuck it follow
			break;
		case 7: // attack
			emu->command = 2;
			break;
		case 8: // follow
			emu->command = 4;
			break;
		case 9: // sit down
			emu->command = 7;
			break;
		case 10: // stand up
			emu->command = 8;
			break;
		case 11: // taunt toggle
			emu->command = 12;
			break;
		case 12: // hold toggle
			emu->command = 15;
			break;
		case 13: // taunt on
			emu->command = 13;
			break;
		case 14: // no taunt
			emu->command = 14;
			break;
		// 15 is target, doesn't send packet
		case 16: // leader
			emu->command = 1;
			break;
		case 17: // feign
			emu->command = 27;
			break;
		case 18: // no cast toggle
			emu->command = 21;
			break;
		case 19: // focus toggle
			emu->command = 24;
			break;
		default:
			emu->command = eq->command;
		}
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ReadBook)
	{
		// no apparent slot translation needed
		DECODE_LENGTH_ATLEAST(structs::BookRequest_Struct);
		SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

		IN(window);
		IN(type);
		strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_SetServerFilter)
	{
		DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
		SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);

		int r;
		for (r = 0; r < 29; r++) {
			IN(filters[r]);
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerSell)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Struct);
		SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		IN(npcid);
		emu->itemslot = TitaniumToServerSlot(eq->itemslot);
		IN(quantity);
		IN(price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TraderBuy)
	{
		DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
		SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		MEMSET_IN(TraderBuy_Struct);

		IN(Action);
		IN(Price);
		IN(TraderID);
		memcpy(emu->ItemName, eq->ItemName, sizeof(emu->ItemName));
		IN(ItemID);
		IN(Quantity);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TradeSkillCombine)
	{
		DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
		SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

		emu->container_slot = TitaniumToServerSlot(eq->container_slot);
		IN(guildtribute_slot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = TitaniumToServerSlot(eq->slot);
		IN(quantity);
		IN(tribute_master_id);
		IN(tribute_points);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_WearChange)
	{
		DECODE_LENGTH_EXACT(structs::WearChange_Struct);
		SETUP_DIRECT_DECODE(WearChange_Struct, structs::WearChange_Struct);

		IN(spawn_id);
		IN(material);
		IN(color.Color);
		IN(wear_slot_id);
		emu->unknown06 = 0;
		emu->elite_material = 0;
		emu->hero_forge_model = 0;
		emu->unknown18 = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_WhoAllRequest)
	{
		DECODE_LENGTH_EXACT(structs::Who_All_Struct);
		SETUP_DIRECT_DECODE(Who_All_Struct, structs::Who_All_Struct);

		memcpy(emu->whom, eq->whom, sizeof(emu->whom));
		IN(wrace);
		IN(wclass);
		IN(lvllow);
		IN(lvlhigh);
		IN(gmlookup);
		emu->type = 3;

		FINISH_DIRECT_DECODE();
	}

// file scope helper methods
	void SerializeItem(EQEmu::OutBuffer& ob, const EQEmu::ItemInstance *inst, int16 slot_id_in, uint8 depth)
	{
		const char* protection = "\\\\\\\\\\";
		const EQEmu::ItemData* item = inst->GetUnscaledItem();

		ob << StringFormat("%.*s%s", (depth ? (depth - 1) : 0), protection, (depth ? "\"" : "")); // For leading quotes (and protection) if a subitem;
		
		// Instance data
		ob << itoa((inst->IsStackable() ? inst->GetCharges() : 0)); // stack count
		ob << '|' << itoa(0); // unknown
		ob << '|' << itoa((!inst->GetMerchantSlot() ? slot_id_in : inst->GetMerchantSlot())); // inst slot/merchant slot
		ob << '|' << itoa(inst->GetPrice()); // merchant price
		ob << '|' << itoa((!inst->GetMerchantSlot() ? 1 : inst->GetMerchantCount())); // inst count/merchant count
		ob << '|' << itoa((inst->IsScaling() ? (inst->GetExp() / 100) : 0)); // inst experience
		ob << '|' << itoa((!inst->GetMerchantSlot() ? inst->GetSerialNumber() : inst->GetMerchantSlot())); // merchant serial number
		ob << '|' << itoa(inst->GetRecastTimestamp()); // recast timestamp
		ob << '|' << itoa(((inst->IsStackable() ? ((inst->GetItem()->ItemType == EQEmu::item::ItemTypePotion) ? 1 : 0) : inst->GetCharges()))); // charge count
		ob << '|' << itoa((inst->IsAttuned() ? 1 : 0)); // inst attuned
		ob << '|' << itoa(0); // unknown
		ob << '|';

		ob << StringFormat("%.*s\"", depth, protection); // Quotes (and protection, if needed) around static data

		// Item data
		ob << itoa(item->ItemClass);
		ob << '|' << item->Name;
		ob << '|' << item->Lore;
		ob << '|' << item->IDFile;
		ob << '|' << itoa(item->ID);
		ob << '|' << itoa(((item->Weight > 255) ? 255 : item->Weight));

		ob << '|' << itoa(item->NoRent);
		ob << '|' << itoa(item->NoDrop);
		ob << '|' << itoa(item->Size);
		ob << '|' << itoa(item->Slots);
		ob << '|' << itoa(item->Price);
		ob << '|' << itoa(item->Icon);
		ob << '|' << "0";
		ob << '|' << "0";
		ob << '|' << itoa(item->BenefitFlag);
		ob << '|' << itoa(item->Tradeskills);

		ob << '|' << itoa(item->CR);
		ob << '|' << itoa(item->DR);
		ob << '|' << itoa(item->PR);
		ob << '|' << itoa(item->MR);
		ob << '|' << itoa(item->FR);

		ob << '|' << itoa(item->AStr);
		ob << '|' << itoa(item->ASta);
		ob << '|' << itoa(item->AAgi);
		ob << '|' << itoa(item->ADex);
		ob << '|' << itoa(item->ACha);
		ob << '|' << itoa(item->AInt);
		ob << '|' << itoa(item->AWis);

		ob << '|' << itoa(item->HP);
		ob << '|' << itoa(item->Mana);
		ob << '|' << itoa(item->AC);
		ob << '|' << itoa(item->Deity);

		ob << '|' << itoa(item->SkillModValue);
		ob << '|' << itoa(item->SkillModMax);
		ob << '|' << itoa(item->SkillModType);

		ob << '|' << itoa(item->BaneDmgRace);
		ob << '|' << itoa(item->BaneDmgAmt);
		ob << '|' << itoa(item->BaneDmgBody);

		ob << '|' << itoa(item->Magic);
		ob << '|' << itoa(item->CastTime_);
		ob << '|' << itoa(item->ReqLevel);
		ob << '|' << itoa(item->BardType);
		ob << '|' << itoa(item->BardValue);
		ob << '|' << itoa(item->Light);
		ob << '|' << itoa(item->Delay);

		ob << '|' << itoa(item->RecLevel);
		ob << '|' << itoa(item->RecSkill);

		ob << '|' << itoa(item->ElemDmgType);
		ob << '|' << itoa(item->ElemDmgAmt);

		ob << '|' << itoa(item->Range);
		ob << '|' << itoa(item->Damage);

		ob << '|' << itoa(item->Color);
		ob << '|' << itoa(item->Classes);
		ob << '|' << itoa(item->Races);
		ob << '|' << "0";

		ob << '|' << itoa(item->MaxCharges);
		ob << '|' << itoa(item->ItemType);
		ob << '|' << itoa(item->Material);
		ob << '|' << StringFormat("%f", item->SellRate);

		ob << '|' << "0";
		ob << '|' << itoa(item->CastTime_);
		ob << '|' << "0";

		ob << '|' << itoa(item->ProcRate);
		ob << '|' << itoa(item->CombatEffects);
		ob << '|' << itoa(item->Shielding);
		ob << '|' << itoa(item->StunResist);
		ob << '|' << itoa(item->StrikeThrough);
		ob << '|' << itoa(item->ExtraDmgSkill);
		ob << '|' << itoa(item->ExtraDmgAmt);
		ob << '|' << itoa(item->SpellShield);
		ob << '|' << itoa(item->Avoidance);
		ob << '|' << itoa(item->Accuracy);

		ob << '|' << itoa(item->CharmFileID);

		ob << '|' << itoa(item->FactionMod1);
		ob << '|' << itoa(item->FactionMod2);
		ob << '|' << itoa(item->FactionMod3);
		ob << '|' << itoa(item->FactionMod4);

		ob << '|' << itoa(item->FactionAmt1);
		ob << '|' << itoa(item->FactionAmt2);
		ob << '|' << itoa(item->FactionAmt3);
		ob << '|' << itoa(item->FactionAmt4);

		ob << '|' << item->CharmFile;

		ob << '|' << itoa(item->AugType);

		ob << '|' << itoa(item->AugSlotType[0]);
		ob << '|' << itoa(item->AugSlotVisible[0]);
		ob << '|' << itoa(item->AugSlotType[1]);
		ob << '|' << itoa(item->AugSlotVisible[1]);
		ob << '|' << itoa(item->AugSlotType[2]);
		ob << '|' << itoa(item->AugSlotVisible[2]);
		ob << '|' << itoa(item->AugSlotType[3]);
		ob << '|' << itoa(item->AugSlotVisible[3]);
		ob << '|' << itoa(item->AugSlotType[4]);
		ob << '|' << itoa(item->AugSlotVisible[4]);

		ob << '|' << itoa(item->LDoNTheme);
		ob << '|' << itoa(item->LDoNPrice);
		ob << '|' << itoa(item->LDoNSold);

		ob << '|' << itoa(item->BagType);
		ob << '|' << itoa(item->BagSlots);
		ob << '|' << itoa(item->BagSize);
		ob << '|' << itoa(item->BagWR);

		ob << '|' << itoa(item->Book);
		ob << '|' << itoa(item->BookType);

		ob << '|' << item->Filename;

		ob << '|' << itoa(item->BaneDmgRaceAmt);
		ob << '|' << itoa(item->AugRestrict);
		ob << '|' << itoa(item->LoreGroup);
		ob << '|' << itoa(item->PendingLoreFlag);
		ob << '|' << itoa(item->ArtifactFlag);
		ob << '|' << itoa(item->SummonedFlag);

		ob << '|' << itoa(item->Favor);
		ob << '|' << itoa(item->FVNoDrop);
		ob << '|' << itoa(item->Endur);
		ob << '|' << itoa(item->DotShielding);
		ob << '|' << itoa(item->Attack);
		ob << '|' << itoa(item->Regen);
		ob << '|' << itoa(item->ManaRegen);
		ob << '|' << itoa(item->EnduranceRegen);
		ob << '|' << itoa(item->Haste);
		ob << '|' << itoa(item->DamageShield);
		ob << '|' << itoa(item->RecastDelay);
		ob << '|' << itoa(item->RecastType);
		ob << '|' << itoa(item->GuildFavor);

		ob << '|' << itoa(item->AugDistiller);

		ob << '|' << "0"; // unknown
		ob << '|' << "0"; // unknown
		ob << '|' << itoa(item->Attuneable);
		ob << '|' << itoa(item->NoPet);
		ob << '|' << "0"; // unknown
		ob << '|' << itoa(item->PointType);

		ob << '|' << itoa(item->PotionBelt);
		ob << '|' << itoa(item->PotionBeltSlots);
		ob << '|' << itoa(item->StackSize);
		ob << '|' << itoa(item->NoTransfer);
		ob << '|' << itoa(item->Stackable);

		ob << '|' << itoa(item->Click.Effect);
		ob << '|' << itoa(item->Click.Type);
		ob << '|' << itoa(item->Click.Level2);
		ob << '|' << itoa(item->Click.Level);
		ob << '|' << "0"; // Click name

		ob << '|' << itoa(item->Proc.Effect);
		ob << '|' << itoa(item->Proc.Type);
		ob << '|' << itoa(item->Proc.Level2);
		ob << '|' << itoa(item->Proc.Level);
		ob << '|' << "0"; // Proc name

		ob << '|' << itoa(item->Worn.Effect);
		ob << '|' << itoa(item->Worn.Type);
		ob << '|' << itoa(item->Worn.Level2);
		ob << '|' << itoa(item->Worn.Level);
		ob << '|' << "0"; // Worn name

		ob << '|' << itoa(item->Focus.Effect);
		ob << '|' << itoa(item->Focus.Type);
		ob << '|' << itoa(item->Focus.Level2);
		ob << '|' << itoa(item->Focus.Level);
		ob << '|' << "0"; // Focus name

		ob << '|' << itoa(item->Scroll.Effect);
		ob << '|' << itoa(item->Scroll.Type);
		ob << '|' << itoa(item->Scroll.Level2);
		ob << '|' << itoa(item->Scroll.Level);
		ob << '|' << "0"; // Scroll name

		ob << StringFormat("%.*s\"", depth, protection); // Quotes (and protection, if needed) around static data

		// Sub data
		for (int index = EQEmu::invbag::SLOT_BEGIN; index <= invbag::SLOT_END; ++index) {
			ob << '|';

			EQEmu::ItemInstance* sub = inst->GetItem(index);
			if (!sub)
				continue;
			
			SerializeItem(ob, sub, 0, (depth + 1));
		}

		ob << StringFormat("%.*s%s", (depth ? (depth - 1) : 0), protection, (depth ? "\"" : "")); // For trailing quotes (and protection) if a subitem;

		if (!depth)
			ob.write("\0", 1);
	}

	static inline int16 ServerToTitaniumSlot(uint32 serverSlot)
	{
		//int16 TitaniumSlot;
		if (serverSlot == INVALID_INDEX)
			return INVALID_INDEX;

		return serverSlot; // deprecated
	}

	static inline int16 ServerToTitaniumCorpseSlot(uint32 serverCorpseSlot)
	{
		//int16 TitaniumCorpse;
		return serverCorpseSlot;
	}

	static inline uint32 TitaniumToServerSlot(int16 titaniumSlot)
	{
		//uint32 ServerSlot;
		if (titaniumSlot == INVALID_INDEX)
			return INVALID_INDEX;

		return titaniumSlot; // deprecated
	}

	static inline uint32 TitaniumToServerCorpseSlot(int16 titaniumCorpseSlot)
	{
		//uint32 ServerCorpse;
		return titaniumCorpseSlot;
	}

	static inline void ServerToTitaniumSayLink(std::string& titaniumSayLink, const std::string& serverSayLink)
	{
		if ((constants::SAY_LINK_BODY_SIZE == EQEmu::constants::SAY_LINK_BODY_SIZE) || (serverSayLink.find('\x12') == std::string::npos)) {
			titaniumSayLink = serverSayLink;
			return;
		}

		auto segments = SplitString(serverSayLink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= EQEmu::constants::SAY_LINK_BODY_SIZE) {
					titaniumSayLink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// 6.2:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X       XXXXXXXX (45)
				// Diff:                                       ^^^^^         ^  ^^^^^

				titaniumSayLink.push_back('\x12');
				titaniumSayLink.append(segments[segment_iter].substr(0, 31));
				titaniumSayLink.append(segments[segment_iter].substr(36, 5));

				if (segments[segment_iter][41] == '0')
					titaniumSayLink.push_back(segments[segment_iter][42]);
				else
					titaniumSayLink.push_back('F');

				titaniumSayLink.append(segments[segment_iter].substr(48));
				titaniumSayLink.push_back('\x12');
			}
			else {
				titaniumSayLink.append(segments[segment_iter]);
			}
		}
	}

	static inline void TitaniumToServerSayLink(std::string& serverSayLink, const std::string& titaniumSayLink)
	{
		if ((EQEmu::constants::SAY_LINK_BODY_SIZE == constants::SAY_LINK_BODY_SIZE) || (titaniumSayLink.find('\x12') == std::string::npos)) {
			serverSayLink = titaniumSayLink;
			return;
		}

		auto segments = SplitString(titaniumSayLink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= constants::SAY_LINK_BODY_SIZE) {
					serverSayLink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26          31 32    36       37       (Source)
				// 6.2:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X        XXXXXXXX (45)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
				// Diff:                                       ^^^^^         ^   ^^^^^

				serverSayLink.push_back('\x12');
				serverSayLink.append(segments[segment_iter].substr(0, 31));
				serverSayLink.append("00000");
				serverSayLink.append(segments[segment_iter].substr(31, 5));
				serverSayLink.push_back('0');
				serverSayLink.push_back(segments[segment_iter][36]);
				serverSayLink.append("00000");
				serverSayLink.append(segments[segment_iter].substr(37));
				serverSayLink.push_back('\x12');
			}
			else {
				serverSayLink.append(segments[segment_iter]);
			}
		}
	}

	static inline CastingSlot ServerToTitaniumCastingSlot(EQEmu::CastingSlot slot)
	{
		switch (slot) {
		case EQEmu::CastingSlot::Gem1:
			return CastingSlot::Gem1;
		case EQEmu::CastingSlot::Gem2:
			return CastingSlot::Gem2;
		case EQEmu::CastingSlot::Gem3:
			return CastingSlot::Gem3;
		case EQEmu::CastingSlot::Gem4:
			return CastingSlot::Gem4;
		case EQEmu::CastingSlot::Gem5:
			return CastingSlot::Gem5;
		case EQEmu::CastingSlot::Gem6:
			return CastingSlot::Gem6;
		case EQEmu::CastingSlot::Gem7:
			return CastingSlot::Gem7;
		case EQEmu::CastingSlot::Gem8:
			return CastingSlot::Gem8;
		case EQEmu::CastingSlot::Gem9:
			return CastingSlot::Gem9;
		case EQEmu::CastingSlot::Item:
			return CastingSlot::Item;
		case EQEmu::CastingSlot::PotionBelt:
			return CastingSlot::PotionBelt;
		case EQEmu::CastingSlot::Discipline:
			return CastingSlot::Discipline;
		case EQEmu::CastingSlot::AltAbility:
			return CastingSlot::AltAbility;
		default: // we shouldn't have any issues with other slots ... just return something
			return CastingSlot::Discipline;
		}
	}

	static inline EQEmu::CastingSlot TitaniumToServerCastingSlot(CastingSlot slot, uint32 itemlocation)
	{
		switch (slot) {
		case CastingSlot::Gem1:
			return EQEmu::CastingSlot::Gem1;
		case CastingSlot::Gem2:
			return EQEmu::CastingSlot::Gem2;
		case CastingSlot::Gem3:
			return EQEmu::CastingSlot::Gem3;
		case CastingSlot::Gem4:
			return EQEmu::CastingSlot::Gem4;
		case CastingSlot::Gem5:
			return EQEmu::CastingSlot::Gem5;
		case CastingSlot::Gem6:
			return EQEmu::CastingSlot::Gem6;
		case CastingSlot::Gem7:
			return EQEmu::CastingSlot::Gem7;
		case CastingSlot::Gem8:
			return EQEmu::CastingSlot::Gem8;
		case CastingSlot::Gem9:
			return EQEmu::CastingSlot::Gem9;
		case CastingSlot::Ability:
			return EQEmu::CastingSlot::Ability;
		// Tit uses 10 for item and discipline casting, but items have a valid location
		case CastingSlot::Item:
			if (itemlocation == INVALID_INDEX)
				return EQEmu::CastingSlot::Discipline;
			else
				return EQEmu::CastingSlot::Item;
		case CastingSlot::PotionBelt:
			return EQEmu::CastingSlot::PotionBelt;
		case CastingSlot::AltAbility:
			return EQEmu::CastingSlot::AltAbility;
		default: // we shouldn't have any issues with other slots ... just return something
			return EQEmu::CastingSlot::Discipline;
		}
	}

	static inline int ServerToTitaniumBuffSlot(int index)
	{
		// we're a disc
		if (index >= EQEmu::constants::LongBuffs + EQEmu::constants::ShortBuffs)
			return index - EQEmu::constants::LongBuffs - EQEmu::constants::ShortBuffs +
			       constants::LongBuffs + constants::ShortBuffs;
		// we're a song
		if (index >= EQEmu::constants::LongBuffs)
			return index - EQEmu::constants::LongBuffs + constants::LongBuffs;
		// we're a normal buff
		return index; // as long as we guard against bad slots server side, we should be fine
	}

	static inline int TitaniumToServerBuffSlot(int index)
	{
		// we're a disc
		if (index >= constants::LongBuffs + constants::ShortBuffs)
			return index - constants::LongBuffs - constants::ShortBuffs + EQEmu::constants::LongBuffs +
			       EQEmu::constants::ShortBuffs;
		// we're a song
		if (index >= constants::LongBuffs)
			return index - constants::LongBuffs + EQEmu::constants::LongBuffs;
		// we're a normal buff
		return index; // as long as we guard against bad slots server side, we should be fine
	}
} /*Titanium*/
