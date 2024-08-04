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
#include "tds.h"
#include "../opcodemgr.h"

#include "../eq_stream_ident.h"
#include "../crc32.h"

#include "../eq_packet_structs.h"
#include "../misc_functions.h"
#include "../strings.h"
#include "../inventory_profile.h"
#include "tds_structs.h"
#include "../rulesys.h"
#include "../path_manager.h"
#include "tds_constants.h"
#include "../classes.h"
#include "../races.h"
#include "../raid.h"

#include <iostream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cinttypes>

namespace TDS
{
	static const char *name = "TDS";
	static OpcodeManager *opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance* inst, int16 slot_id, uint8 depth, ItemPacketType packet_type);

	// server to client inventory location converters
	static inline structs::ItemSlotStruct ServerToTDSSlot(uint32 serverSlot, ItemPacketType PacketType = ItemPacketInvalid);
	static inline structs::MainInvItemSlotStruct ServerToTDSMainInvSlot(uint32 serverSlot);
	static inline uint32 ServerToTDSCorpseSlot(uint32 serverCorpseSlot);

	// client to server inventory location converters
	static inline uint32 TDSToServerSlot(structs::ItemSlotStruct tdsSlot, ItemPacketType PacketType = ItemPacketInvalid);
	static inline uint32 TDSToServerMainInvSlot(structs::MainInvItemSlotStruct tdsSlot);
	static inline uint32 TDSToServerCorpseSlot(uint32 tdsCorpseSlot);

	// server to client text link converter
	static inline void ServerToTDSTextLink(std::string& tdsTextLink, const std::string& serverTextLink);

	// client to server text link converter
	static inline void TDSToServerTextLink(std::string& serverTextLink, const std::string& tdsTextLink);

	void Register(EQStreamIdentifier &into)
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
#include "tds_ops.h"
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
		return EQ::versions::ClientVersion::TDS;
	}

#include "ss_define.h"

// ENCODE methods


	// RoF2 Specific Encodes Begin
	ENCODE(OP_SendMembershipDetails)
	{
		ENCODE_LENGTH_EXACT(Membership_Details_Struct);
		SETUP_DIRECT_ENCODE(Membership_Details_Struct, structs::Membership_Details_Struct);

		eq->membership_setting_count = 72;
		for (uint32 i = 0; i < emu->membership_setting_count; ++i) // 66
		{
			OUT(settings[i].setting_index);
			OUT(settings[i].setting_id);
			OUT(settings[i].setting_value);
		}
		// Last 6 new settings fields are all 0s on Live as of 12/29/14

		eq->race_entry_count = emu->race_entry_count;
		for (uint32 i = 0; i < emu->race_entry_count; ++i) // 15
		{
			OUT(membership_races[i].purchase_id);
			OUT(membership_races[i].bitwise_entry);
		}

		eq->class_entry_count = emu->class_entry_count;
		for (uint32 i = 0; i < emu->class_entry_count; ++i) // 15
		{
			OUT(membership_classes[i].purchase_id);
			OUT(membership_classes[i].bitwise_entry);
		}

		eq->exit_url_length = emu->exit_url_length;
		eq->exit_url_length2 = emu->exit_url_length2;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMembership)
	{
		ENCODE_LENGTH_EXACT(Membership_Struct);
		SETUP_DIRECT_ENCODE(Membership_Struct, structs::Membership_Struct);

		eq->membership = emu->membership;
		eq->races = emu->races;
		eq->classes = emu->classes;
		eq->entrysize = 25; //emu->entrysize;

		for (uint32 i = 0; i < emu->entrysize; ++i) // 21
		{
			OUT(entries[i]);
		}
		// Last 4 new entries are 0s on Live Silver as of 12/29/14
		// Setting them each to 1 for now.
		// This removes the "Buy Now" button from aug type 21 slots on items.
		for (uint32 i = 21; i < 25; ++i) // 4
		{
			eq->entries[i] = 1;
		}


		FINISH_ENCODE();
	}

	// RoF2 Specific Encodes End


	ENCODE(OP_Action)
	{
		ENCODE_LENGTH_EXACT(Action_Struct);
		SETUP_DIRECT_ENCODE(Action_Struct, structs::ActionAlt_Struct);

		OUT(target);
		OUT(source);
		OUT(level);
		eq->unknown06 = 0;
		eq->instrument_mod = 1.0f + (emu->instrument_mod - 10) / 10.0f;
		eq->bard_focus_id = INVALID_INDEX; // emu->bard_focus_id;
		eq->knockback_angle = INVALID_INDEX; // emu->sequence;
		eq->unknown22 = 0;
		OUT(type);
		eq->damage = 0;
		eq->unknown31 = 0;
		OUT(spell);
		eq->level2 = eq->level;
		eq->effect_flag = emu->effect_flag;
		eq->unknown39 = 14;
		eq->unknown43 = 0;
		eq->unknown44 = 17;
		eq->unknown45 = 0;
		eq->unknown46 = -1;
		eq->unknown50 = 0;
		eq->unknown54 = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_AdventureMerchantSell)
	{
		ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
		SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		eq->unknown000 = 1;
		OUT(npcid);
		eq->slot = ServerToTDSMainInvSlot(emu->slot);
		OUT(charges);
		OUT(sell_price);

		FINISH_ENCODE();
	}

	ENCODE(OP_AltCurrency)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *emu_buffer = in->pBuffer;
		uint32 opcode = *((uint32*)emu_buffer);

		if (opcode == AlternateCurrencyMode::Populate) {
			AltCurrencyPopulate_Struct *populate = (AltCurrencyPopulate_Struct*)emu_buffer;

			auto outapp = new EQApplicationPacket(
			    OP_AltCurrency, sizeof(structs::AltCurrencyPopulate_Struct) +
						sizeof(structs::AltCurrencyPopulateEntry_Struct) * populate->count);
			structs::AltCurrencyPopulate_Struct *out_populate = (structs::AltCurrencyPopulate_Struct*)outapp->pBuffer;

			out_populate->opcode = populate->opcode;
			out_populate->count = populate->count;
			for (uint32 i = 0; i < populate->count; ++i) {
				out_populate->entries[i].currency_number = populate->entries[i].currency_number;
				out_populate->entries[i].unknown00 = populate->entries[i].unknown00;
				out_populate->entries[i].currency_number2 = populate->entries[i].currency_number2;
				out_populate->entries[i].item_id = populate->entries[i].item_id;
				out_populate->entries[i].item_icon = populate->entries[i].item_icon;
				out_populate->entries[i].stack_size = populate->entries[i].stack_size;
				out_populate->entries[i].display = ((populate->entries[i].stack_size > 0) ? 1 : 0);
			}

			dest->FastQueuePacket(&outapp, ack_req);
		}
		else {
			auto outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyUpdate_Struct));
			memcpy(outapp->pBuffer, emu_buffer, sizeof(AltCurrencyUpdate_Struct));
			dest->FastQueuePacket(&outapp, ack_req);
		}

		//dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_AltCurrencySell)
	{
		ENCODE_LENGTH_EXACT(AltCurrencySellItem_Struct);
		SETUP_DIRECT_ENCODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		OUT(merchant_entity_id);
		eq->slot_id = ServerToTDSMainInvSlot(emu->slot_id);
		OUT(charges);
		OUT(cost);

		FINISH_ENCODE();
	}

	ENCODE(OP_Animation)
	{
		ENCODE_LENGTH_EXACT(Animation_Struct);
		SETUP_DIRECT_ENCODE(Animation_Struct, structs::Animation_Struct);

		OUT(spawnid);
		OUT(action);
		OUT(speed);

		FINISH_ENCODE();
	}

	ENCODE(OP_ApplyPoison)
	{
		ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
		SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		eq->inventorySlot = ServerToTDSMainInvSlot(emu->inventorySlot);
		OUT(success);

		FINISH_ENCODE();
	}

	ENCODE(OP_AugmentInfo)
	{
		ENCODE_LENGTH_EXACT(AugmentInfo_Struct);
		SETUP_DIRECT_ENCODE(AugmentInfo_Struct, structs::AugmentInfo_Struct);

		OUT(itemid);
		OUT(window);
		strn0cpy(eq->augment_info, emu->augment_info, 64);

		FINISH_ENCODE();
	}

	ENCODE(OP_Barter)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		char *Buffer = (char *)in->pBuffer;

		uint32 SubAction = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

		if (SubAction != Barter_BuyerAppearance)
		{
			dest->FastQueuePacket(&in, ack_req);

			return;
		}

		unsigned char *__emu_buffer = in->pBuffer;

		in->size = 80;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		char Name[64];

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, SubAction);
		uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, EntityID);
		uint8 Toggle = VARSTRUCT_DECODE_TYPE(uint8, Buffer);
		VARSTRUCT_DECODE_STRING(Name, Buffer);
		VARSTRUCT_ENCODE_STRING(OutBuffer, Name);
		OutBuffer = (char *)in->pBuffer + 72;
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, Toggle);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_BazaarSearch)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		char *Buffer = (char *)in->pBuffer;

		uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);

		if (SubAction != BazaarSearchResults)
		{
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		unsigned char *__emu_buffer = in->pBuffer;

		BazaarSearchResults_Struct *emu = (BazaarSearchResults_Struct *)__emu_buffer;

		int EntryCount = in->size / sizeof(BazaarSearchResults_Struct);

		if (EntryCount == 0 || (in->size % sizeof(BazaarSearchResults_Struct)) != 0)
		{
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(BazaarSearchResults_Struct));
			delete in;
			return;
		}

		in->size = EntryCount * sizeof(structs::BazaarSearchResults_Struct);
		in->pBuffer = new unsigned char[in->size];

		memset(in->pBuffer, 0, in->size);

		structs::BazaarSearchResults_Struct *eq = (structs::BazaarSearchResults_Struct *)in->pBuffer;

		for (int i = 0; i < EntryCount; ++i, ++emu, ++eq)
		{
			OUT(Beginning.Action);
			OUT(SellerID);
			memcpy(eq->SellerName, emu->SellerName, sizeof(eq->SellerName));
			OUT(NumItems);
			OUT(ItemID);
			OUT(SerialNumber);
			memcpy(eq->ItemName, emu->ItemName, sizeof(eq->ItemName));
			OUT(Cost);
			OUT(ItemStat);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_BeginCast)
	{
		SETUP_DIRECT_ENCODE(BeginCast_Struct, structs::BeginCast_Struct);

		OUT(spell_id);
		OUT(caster_id);
		OUT(cast_time);

		FINISH_ENCODE();
	}

	ENCODE(OP_BlockedBuffs)
	{
		ENCODE_LENGTH_EXACT(BlockedBuffs_Struct);
		SETUP_DIRECT_ENCODE(BlockedBuffs_Struct, structs::BlockedBuffs_Struct);

		for (uint32 i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			eq->SpellID[i] = emu->SpellID[i];

		// -1 for the extra 10 added in RoF2. We should really be encoding for the older clients, not RoF2, but
		// we can sort that out later.

		for (uint32 i = BLOCKED_BUFF_COUNT; i < structs::BLOCKED_BUFF_COUNT; ++i)
			eq->SpellID[i] = -1;

		OUT(Count);
		OUT(Pet);
		OUT(Initialise);
		OUT(Flags);

		FINISH_ENCODE();
	}

	ENCODE(OP_Buff)
	{
		ENCODE_LENGTH_EXACT(SpellBuffPacket_Struct);
		SETUP_DIRECT_ENCODE(SpellBuffPacket_Struct, structs::SpellBuffFade_Struct_Live);

		OUT(entityid);
		eq->unknown004 = 2;
		//eq->level = 80;
		//eq->effect = 0;
		//OUT(level); // SpellBuff_Struct
		//OUT(effect); // SpellBuff_Struct
		eq->unknown007 = 0;
		eq->unknown008 = 1.0f;
		//OUT(spellid); // SpellBuff_Struct
		//OUT(duration); // SpellBuffStruct
		eq->playerId = 0x7cde;
		OUT(slotid);
		//OUT(num_hits); // SpellBuffStruct
		if (emu->bufffade == 1)
			eq->bufffade = 1;
		else
			eq->bufffade = 2;

		// Bit of a hack. OP_Buff appears to add/remove the buff while OP_BuffCreate adds/removes the actual buff icon
		EQApplicationPacket *outapp = nullptr;
		if (eq->bufffade == 1)
		{
			outapp = new EQApplicationPacket(OP_BuffCreate, 29);
			outapp->WriteUInt32(emu->entityid);
			outapp->WriteUInt32(0x0271);	// Unk
			outapp->WriteUInt8(0);		// Type of OP_BuffCreate packet ?
			outapp->WriteUInt16(1);		// 1 buff in this packet
			outapp->WriteUInt32(emu->slotid);
			outapp->WriteUInt32(0xffffffff);		// SpellID (0xffff to remove)
			outapp->WriteUInt32(0);			// Duration
			outapp->WriteUInt32(0);			// ?
			outapp->WriteUInt8(0);		// Caster name
			outapp->WriteUInt8(0);		// Type
		}
		FINISH_ENCODE();

		if (outapp)
			dest->FastQueuePacket(&outapp);	// Send the OP_BuffCreate to remove the buff
	}

	ENCODE(OP_BuffCreate)
	{
		SETUP_VAR_ENCODE(BuffIcon_Struct);

		uint32 sz = 12 + (17 * emu->count);
		__packet->size = sz;
		__packet->pBuffer = new unsigned char[sz];
		memset(__packet->pBuffer, 0, sz);

		__packet->WriteUInt32(emu->entity_id);
		__packet->WriteUInt32(0);		// PlayerID ?
		__packet->WriteUInt8(emu->all_buffs);			// 1 indicates all buffs on the player (0 to add or remove a single buff)
		__packet->WriteUInt16(emu->count);

		for (uint16 i = 0; i < emu->count; ++i)
		{
			uint16 buffslot = emu->entries[i].buff_slot;
			// Not sure if this is needs amending for RoF2 yet.
			if (emu->entries[i].buff_slot >= 25)
			{
				buffslot += 17;
			}
			// TODO: We should really just deal with these "server side"
			// so we can have clients not limited to other clients.
			// This fixes discs, songs were changed to 20
			if (buffslot == 54)
				buffslot = 62;

			__packet->WriteUInt32(buffslot);
			__packet->WriteUInt32(emu->entries[i].spell_id);
			__packet->WriteUInt32(emu->entries[i].tics_remaining);
			__packet->WriteUInt32(emu->entries[i].num_hits); // Unknown
			__packet->WriteString("");
		}
		__packet->WriteUInt8(emu->type); // Unknown

		FINISH_ENCODE();
	}

	ENCODE(OP_CancelTrade)
	{
		ENCODE_LENGTH_EXACT(CancelTrade_Struct);
		SETUP_DIRECT_ENCODE(CancelTrade_Struct, structs::CancelTrade_Struct);

		OUT(fromid);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_CastSpell)
	{
		ENCODE_LENGTH_EXACT(CastSpell_Struct);
		SETUP_DIRECT_ENCODE(CastSpell_Struct, structs::CastSpell_Struct);

		if (emu->slot == 10)
			eq->slot = 13;
		else
			OUT(slot);

		OUT(spell_id);
		eq->inventoryslot = ServerToTDSSlot(emu->inventoryslot);
		//OUT(inventoryslot);
		OUT(target_id);

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
		ServerToTDSTextLink(new_message, old_message);

		//in->size = strlen(emu->sender) + 1 + strlen(emu->targetname) + 1 + strlen(emu->message) + 1 + 36;
		in->size = strlen(emu->sender) + strlen(emu->targetname) + new_message.length() + 39;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->sender);
		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->targetname);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->language);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->chan_num);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->skill_in_language);
		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_CharInventory)
	{
		//consume the packet
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (!in->size) {
			in->size = 4;
			in->pBuffer = new uchar[in->size];
			memset(in->pBuffer, 0, in->size);

			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		//store away the emu struct
		uchar* __emu_buffer = in->pBuffer;

		int ItemCount = in->size / sizeof(EQ::InternalSerializedItem_Struct);

		if (!ItemCount || (in->size % sizeof(EQ::InternalSerializedItem_Struct)) != 0) {

			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(EQ::InternalSerializedItem_Struct));

			delete in;

			return;
		}

		EQ::InternalSerializedItem_Struct *eq = (EQ::InternalSerializedItem_Struct *)in->pBuffer;

		EQ::OutBuffer ob;
		EQ::OutBuffer::pos_type last_pos = ob.tellp();

		ob.write((const char*)&ItemCount, sizeof(uint32));

		for (int r = 0; r < ItemCount; ++r, ++eq) {

			uint32 Length = 0;

			SerializeItem(ob, (const EQ::ItemInstance*)eq->inst, eq->slot_id, 0, ItemPacketCharInventory);

			if (ob.tellp() != last_pos) {

				uchar *OldBuffer = in->pBuffer;
				in->pBuffer = new uchar[in->size + Length];
				memcpy(in->pBuffer, OldBuffer, in->size);

				safe_delete_array(OldBuffer);

				memcpy(in->pBuffer + in->size, &ob, Length);
				in->size += Length;

				delete[] &ob;
			}
			else {
				LogNetcode("TDS::ENCODE(OP_CharInventory) Serialization failed on item slot [{}] during OP_CharInventory.  Item skipped", eq->slot_id);
			}
		}

		delete[] __emu_buffer;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Sending inventory to client");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ClickObjectAction)
	{
		ENCODE_LENGTH_EXACT(ClickObjectAction_Struct);
		SETUP_DIRECT_ENCODE(ClickObjectAction_Struct, structs::ClickObjectAction_Struct);

		OUT(drop_id);
		eq->unknown04 = -1;
		eq->unknown08 = -1;
		OUT(type);
		OUT(icon);
		eq->unknown16 = 0;
		OUT_str(object_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_ClientUpdate)
	{
		ENCODE_LENGTH_EXACT(PlayerPositionUpdateServer_Struct);
		SETUP_DIRECT_ENCODE(PlayerPositionUpdateServer_Struct, structs::PlayerPositionUpdateServer_Struct);

		OUT(spawn_id);
		OUT(x_pos);
		OUT(delta_x);
		OUT(delta_y);
		OUT(z_pos);
		OUT(delta_heading);
		OUT(y_pos);
		OUT(delta_z);
		OUT(animation);
		OUT(heading);

		FINISH_ENCODE();
	}

	ENCODE(OP_Consider)
	{
		ENCODE_LENGTH_EXACT(Consider_Struct);
		SETUP_DIRECT_ENCODE(Consider_Struct, structs::Consider_Struct);

		OUT(playerid);
		OUT(targetid);
		OUT(faction);
		OUT(level);
		OUT(pvpcon);

		FINISH_ENCODE();
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
		OUT(force)
		//OUT(meleepush_xy);
		//OUT(meleepush_z)

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteCharge)
	{
		Log(Logs::Moderate, Logs::Netcode, "TDS::ENCODE(OP_DeleteCharge)");

		ENCODE_FORWARD(OP_MoveItem);
	}

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToTDSSlot(emu->from_slot);
		eq->to_slot = ServerToTDSSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteSpawn)
	{
		ENCODE_LENGTH_EXACT(DeleteSpawn_Struct);
		SETUP_DIRECT_ENCODE(DeleteSpawn_Struct, structs::DeleteSpawn_Struct);

		OUT(spawn_id);
		eq->unknown04 = 1;	// Observed

		FINISH_ENCODE();
	}

	ENCODE(OP_DisciplineUpdate)
	{
		ENCODE_LENGTH_EXACT(Disciplines_Struct);
		SETUP_DIRECT_ENCODE(Disciplines_Struct, structs::Disciplines_Struct);

		memcpy(&eq->values, &emu->values, sizeof(Disciplines_Struct));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzChooseZone)
	{
		SETUP_VAR_ENCODE(DynamicZoneChooseZone_Struct);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->count);

		for (uint32 i = 0; i < emu->count; ++i)
		{
			buf.WriteUInt16(emu->choices[i].dz_zone_id);
			buf.WriteUInt16(emu->choices[i].dz_instance_id);
			buf.WriteUInt32(emu->choices[i].unknown_id1);
			buf.WriteUInt32(emu->choices[i].dz_type);
			buf.WriteUInt32(emu->choices[i].unknown_id2);
			buf.WriteString(emu->choices[i].description);
			buf.WriteString(emu->choices[i].leader_name);
		}

		__packet->size = buf.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buf.buffer(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzCompass)
	{
		SETUP_VAR_ENCODE(DynamicZoneCompass_Struct);
		ALLOC_VAR_ENCODE(structs::DynamicZoneCompass_Struct,
			sizeof(structs::DynamicZoneCompass_Struct) +
			sizeof(structs::DynamicZoneCompassEntry_Struct) * emu->count
		);

		OUT(client_id);
		OUT(count);

		for (uint32 i = 0; i < emu->count; ++i)
		{
			OUT(entries[i].dz_zone_id);
			OUT(entries[i].dz_instance_id);
			OUT(entries[i].dz_type);
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

		OUT(client_id);
		OUT(assigned);
		OUT(max_players);
		strn0cpy(eq->expedition_name, emu->expedition_name, sizeof(eq->expedition_name));
		strn0cpy(eq->leader_name, emu->leader_name, sizeof(eq->leader_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionInvite)
	{
		ENCODE_LENGTH_EXACT(ExpeditionInvite_Struct);
		SETUP_DIRECT_ENCODE(ExpeditionInvite_Struct, structs::ExpeditionInvite_Struct);

		OUT(client_id);
		strn0cpy(eq->inviter_name, emu->inviter_name, sizeof(eq->inviter_name));
		strn0cpy(eq->expedition_name, emu->expedition_name, sizeof(eq->expedition_name));
		OUT(swapping);
		strn0cpy(eq->swap_name, emu->swap_name, sizeof(eq->swap_name));
		OUT(dz_zone_id);
		OUT(dz_instance_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionLockoutTimers)
	{
		SETUP_VAR_ENCODE(ExpeditionLockoutTimers_Struct);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->count);
		for (uint32 i = 0; i < emu->count; ++i)
		{
			buf.WriteString(emu->timers[i].expedition_name);
			buf.WriteUInt32(emu->timers[i].seconds_remaining);
			buf.WriteInt32(emu->timers[i].event_type);
			buf.WriteString(emu->timers[i].event_name);
		}

		__packet->size = buf.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buf.buffer(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzSetLeaderName)
	{
		ENCODE_LENGTH_EXACT(ExpeditionSetLeaderName_Struct);
		SETUP_DIRECT_ENCODE(ExpeditionSetLeaderName_Struct, structs::ExpeditionSetLeaderName_Struct);

		OUT(client_id);
		strn0cpy(eq->leader_name, emu->leader_name, sizeof(eq->leader_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberList)
	{
		SETUP_VAR_ENCODE(ExpeditionMemberList_Struct);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->member_count);
		for (uint32 i = 0; i < emu->member_count; ++i)
		{
			buf.WriteString(emu->members[i].name);
			buf.WriteUInt8(emu->members[i].expedition_status);
		}

		__packet->size = buf.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buf.buffer(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberListName)
	{
		ENCODE_LENGTH_EXACT(ExpeditionMemberListName_Struct);
		SETUP_DIRECT_ENCODE(ExpeditionMemberListName_Struct, structs::ExpeditionMemberListName_Struct);

		OUT(client_id);
		OUT(add_name);
		strn0cpy(eq->name, emu->name, sizeof(eq->name));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberListStatus)
	{
		auto emu = reinterpret_cast<ExpeditionMemberList_Struct*>((*p)->pBuffer);
		if (emu->member_count == 1)
		{
			ENCODE_FORWARD(OP_DzMemberList);
		}
	}

	ENCODE(OP_Emote)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		Emote_Struct *emu = (Emote_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		std::string old_message = emu->message;
		std::string new_message;
		ServerToTDSTextLink(new_message, old_message);

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

	ENCODE(OP_ExpansionInfo)
	{
		ENCODE_LENGTH_EXACT(ExpansionInfo_Struct);
		SETUP_DIRECT_ENCODE(ExpansionInfo_Struct, structs::ExpansionInfo_Struct);

		OUT(Expansions);

		FINISH_ENCODE();
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
			ServerToTDSTextLink(new_message_array[i], old_message_array[i]);
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

	ENCODE(OP_GMLastName)
	{
		ENCODE_LENGTH_EXACT(GMLastName_Struct);
		SETUP_DIRECT_ENCODE(GMLastName_Struct, structs::GMLastName_Struct);

		OUT_str(name);
		OUT_str(gmname);
		OUT_str(lastname);
		for (int i = 0; i<4; i++)
		{
			eq->unknown[i] = emu->unknown[i];
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_GMTrainSkillConfirm)
	{
		ENCODE_LENGTH_EXACT(GMTrainSkillConfirm_Struct);
		SETUP_DIRECT_ENCODE(GMTrainSkillConfirm_Struct, structs::GMTrainSkillConfirm_Struct);

		OUT(SkillID);
		OUT(Cost);
		OUT(NewSkill);
		OUT_str(TrainerName);

		FINISH_ENCODE();
	}

	ENCODE(OP_GroundSpawn)
	{
		// We are not encoding the spawn_id field here, but it doesn't appear to matter.
		//
		EQApplicationPacket *in = *p;
		*p = nullptr;

		Object_Struct *emu = (Object_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		in->size = strlen(emu->object_name) + sizeof(Object_Struct)-1;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);
		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->object_name);
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_id);
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_instance);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);	// Some unique id
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Same for all objects in the zone
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->heading);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, 0);	// Normally 0, but seen (float)255.0 as well
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->solidtype);	// Unknown
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->size != 0 && (float)emu->size < 5000.f ? (float)((float)emu->size / 100.0f) : 1.f );	// This appears to be the size field. Hackish logic because some PEQ DB items were corrupt.
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->y);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->x);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->z);
		VARSTRUCT_ENCODE_TYPE(int32, OutBuffer, emu->object_type);	// Unknown, observed 0x00000014

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GroupCancelInvite)
	{
		ENCODE_LENGTH_EXACT(GroupCancel_Struct);
		SETUP_DIRECT_ENCODE(GroupCancel_Struct, structs::GroupCancel_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));
		OUT(toggle);

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupFollow)
	{
		ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
		SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupFollow2)
	{
		ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
		SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupInvite)
	{
		ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
		SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupInvite_Struct);

		memcpy(eq->invitee_name, emu->name1, sizeof(eq->invitee_name));
		memcpy(eq->inviter_name, emu->name2, sizeof(eq->inviter_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupUpdate)
	{
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] OP_GroupUpdate");
		EQApplicationPacket *in = *p;
		GroupJoin_Struct *gjs = (GroupJoin_Struct*)in->pBuffer;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received outgoing OP_GroupUpdate with action code %i", gjs->action);
		if ((gjs->action == groupActLeave) || (gjs->action == groupActDisband))
		{
			if ((gjs->action == groupActDisband) || !strcmp(gjs->yourname, gjs->membername))
			{
				//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Group Leave, yourname = %s, membername = %s", gjs->yourname, gjs->membername);

				auto outapp =
				    new EQApplicationPacket(OP_GroupDisbandYou, sizeof(structs::GroupGeneric_Struct));

				structs::GroupGeneric_Struct *ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
				memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
				memcpy(ggs->name2, gjs->membername, sizeof(ggs->name1));
				dest->FastQueuePacket(&outapp);

				// Make an empty GLAA packet to clear out their useable GLAAs
				//
				outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

				dest->FastQueuePacket(&outapp);
				delete in;
				return;
			}
			//if(gjs->action == groupActLeave)
			//	Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Group Leave, yourname = %s, membername = %s", gjs->yourname, gjs->membername);

			auto outapp =
			    new EQApplicationPacket(OP_GroupDisbandOther, sizeof(structs::GroupGeneric_Struct));

			structs::GroupGeneric_Struct *ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
			memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
			memcpy(ggs->name2, gjs->membername, sizeof(ggs->name2));
			//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp);

			delete in;
			return;
		}

		if (in->size == sizeof(GroupUpdate2_Struct))
		{
			// Group Update2
			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Struct is GroupUpdate2");

			unsigned char *__emu_buffer = in->pBuffer;
			GroupUpdate2_Struct *gu2 = (GroupUpdate2_Struct*)__emu_buffer;

			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Yourname is %s", gu2->yourname);

			int MemberCount = 1;
			int PacketLength = 8 + strlen(gu2->leadersname) + 1 + 22 + strlen(gu2->yourname) + 1;

			for (int i = 0; i < 5; ++i)
			{
				//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Membername[%i] is %s", i,  gu2->membername[i]);
				if (gu2->membername[i][0] != '\0')
				{
					PacketLength += (22 + strlen(gu2->membername[i]) + 1);
					++MemberCount;
				}
			}

			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Leadername is %s", gu2->leadersname);

			auto outapp = new EQApplicationPacket(OP_GroupUpdateB, PacketLength);

			char *Buffer = (char *)outapp->pBuffer;

			// Header
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Think this should be SpawnID, but it doesn't seem to matter
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberCount);
			VARSTRUCT_ENCODE_STRING(Buffer, gu2->leadersname);

			// Leader
			//

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_STRING(Buffer, gu2->yourname);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			//VARSTRUCT_ENCODE_STRING(Buffer, "");
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x46);	// Observed 0x41 and 0x46 here
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);

			int MemberNumber = 1;

			for (int i = 0; i < 5; ++i)
			{
				if (gu2->membername[i][0] == '\0')
					continue;

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberNumber++);
				VARSTRUCT_ENCODE_STRING(Buffer, gu2->membername[i]);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
				//VARSTRUCT_ENCODE_STRING(Buffer, "");
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x41);	// Observed 0x41 and 0x46 here
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Low byte is Main Assist Flag
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);
			}

			//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp);

			outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

			GroupLeadershipAAUpdate_Struct *GLAAus = (GroupLeadershipAAUpdate_Struct*)outapp->pBuffer;

			GLAAus->NPCMarkerID = gu2->NPCMarkerID;
			memcpy(&GLAAus->LeaderAAs, &gu2->leader_aas, sizeof(GLAAus->LeaderAAs));

			dest->FastQueuePacket(&outapp);
			delete in;

			return;

		}
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Generic GroupUpdate, yourname = %s, membername = %s", gjs->yourname, gjs->membername);
		ENCODE_LENGTH_EXACT(GroupJoin_Struct);
		SETUP_DIRECT_ENCODE(GroupJoin_Struct, structs::GroupJoin_Struct);

		memcpy(eq->membername, emu->membername, sizeof(eq->membername));

		auto outapp =
		    new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));
		GroupLeadershipAAUpdate_Struct *GLAAus = (GroupLeadershipAAUpdate_Struct*)outapp->pBuffer;

		GLAAus->NPCMarkerID = emu->NPCMarkerID;

		memcpy(&GLAAus->LeaderAAs, &emu->leader_aas, sizeof(GLAAus->LeaderAAs));
		//Log.Hex(Logs::Netcode, __packet->pBuffer, __packet->size);

		FINISH_ENCODE();

		dest->FastQueuePacket(&outapp);
	}
/* TODO: Check this
	ENCODE(OP_GuildBank)
	{
		auto in = *p;
		*p = nullptr;
		auto outapp = new EQApplicationPacket(OP_GuildBank, in->size + 4); // all of them are 4 bytes bigger

		// The first action in the enum was removed, everything 1 less
		// Normally we cast them to their structs, but there are so many here! will only do when it's easier
		switch (in->ReadUInt32()) {
		case 10: // GuildBankAcknowledge
			outapp->WriteUInt32(9);
			outapp->WriteUInt32(in->ReadUInt32());
			outapp->WriteUInt32(0);
			break;
		case 5: // GuildBankDeposit (ack)
			outapp->WriteUInt32(4);
			outapp->WriteUInt32(in->ReadUInt32());
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(in->ReadUInt32());
			break;
		case 1: { // GuildBankItemUpdate
			auto emu = (GuildBankItemUpdate_Struct *)in->pBuffer;
			auto eq = (structs::GuildBankItemUpdate_Struct *)outapp->pBuffer;
			eq->Action = 0;
			OUT(Unknown004);
			eq->Unknown08 = 0;
			OUT(SlotID);
			OUT(Area);
			OUT(Unknown012);
			OUT(ItemID);
			OUT(Icon);
			OUT(Quantity);
			OUT(Permissions);
			OUT(AllowMerge);
			OUT(Useable);
			OUT_str(ItemName);
			OUT_str(Donator);
			OUT_str(WhoFor);
			OUT(Unknown226);
			break;
		}
		default:
			break;
		}
		delete in;
		dest->FastQueuePacket(&outapp);
	}
*/
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

		// Guild ID
		buffer += sizeof(uint32);

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

				/* Translate older ranks to new values */
				switch (emu_e->rank) {
				case 0: { e->rank = htonl(5); break; }  // GUILD_MEMBER	0
				case 1: { e->rank = htonl(3); break; }  // GUILD_OFFICER 1
				case 2: { e->rank = htonl(1); break; }  // GUILD_LEADER	2
				default: { e->rank = htonl(emu_e->rank); break; } // GUILD_NONE
				}

				PutFieldN(time_last_on);
				PutFieldN(tribute_enable);
				e->unknown01 = 0;
				PutFieldN(total_tribute);
				PutFieldN(last_tribute);
				e->unknown_one = htonl(1);
				SlideStructString(public_note, emu_note);
				e->zoneinstance = 0;
				e->zone_id = htons(emu_e->zone_id);
				e->unknown_one2 = htonl(1);
				e->unknown04 = 0;

#undef SlideStructString
#undef PutFieldN

				e++;
			}
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GuildMemberUpdate)
	{
		SETUP_DIRECT_ENCODE(GuildMemberUpdate_Struct, structs::GuildMemberUpdate_Struct);

		OUT(GuildID);
		memcpy(eq->MemberName, emu->MemberName, sizeof(eq->MemberName));
		OUT(ZoneID);
		OUT(InstanceID);
		OUT(LastSeen);
		eq->Unknown76 = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildsList)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		uint32 NumberOfGuilds = in->size / 64;
		uint32 PacketSize = 68;	// 64 x 0x00 + a uint32 that I am guessing is the highest guild ID in use.

		unsigned char *__emu_buffer = in->pBuffer;

		char *InBuffer = (char *)__emu_buffer;

		uint32 HighestGuildID = 0;

		for (unsigned int i = 0; i < NumberOfGuilds; ++i)
		{
			if (InBuffer[0])
			{
				PacketSize += (5 + strlen(InBuffer));
				HighestGuildID = i - 1;
			}
			InBuffer += 64;
		}

		PacketSize++;	// Appears to be an extra 0x00 at the very end.

		in->size = PacketSize;
		in->pBuffer = new unsigned char[in->size];

		InBuffer = (char *)__emu_buffer;

		char *OutBuffer = (char *)in->pBuffer;

		// Init the first 64 bytes to zero, as per live.
		//
		memset(OutBuffer, 0, 64);

		OutBuffer += 64;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, HighestGuildID);

		for (unsigned int i = 0; i < NumberOfGuilds; ++i)
		{
			if (InBuffer[0])
			{
				VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, i - 1);
				VARSTRUCT_ENCODE_STRING(OutBuffer, InBuffer);
			}
			InBuffer += 64;
		}

		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0x00);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_HPUpdate)
	{
		SETUP_DIRECT_ENCODE(SpawnHPUpdate_Struct, structs::SpawnHPUpdate_Struct);

		OUT(spawn_id);
		OUT(cur_hp);
		OUT(max_hp);

		FINISH_ENCODE();
	}

	ENCODE(OP_Illusion)
	{
		ENCODE_LENGTH_EXACT(Illusion_Struct);
		SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);

		OUT(spawnid);
		OUT_str(charname);
		OUT(race);
		OUT(unknown006[0]);
		OUT(unknown006[1]);
		OUT(gender);
		OUT(texture);
		OUT(helmtexture);
		OUT(face);
		OUT(hairstyle);
		OUT(haircolor);
		OUT(beard);
		OUT(beardcolor);
		OUT(size);
		OUT(drakkin_heritage);
		OUT(drakkin_tattoo);
		OUT(drakkin_details);
		eq->unknown316 = -1;	// Observed

		FINISH_ENCODE();
	}

	/*ENCODE(OP_InspectAnswer)
	{
	ENCODE_LENGTH_EXACT(InspectResponse_Struct);
	SETUP_DIRECT_ENCODE(InspectResponse_Struct, structs::InspectResponse_Struct);

	OUT(TargetID);
	OUT(playerid);

	int r;
	for (r = 0; r < 21; r++) {
	strn0cpy(eq->itemnames[r], emu->itemnames[r], sizeof(eq->itemnames[r]));
	}
	// Swap last 2 slots for Arrow and Power Source
	strn0cpy(eq->itemnames[21], emu->itemnames[22], sizeof(eq->itemnames[21]));
	strn0cpy(eq->unknown_zero, emu->itemnames[21], sizeof(eq->unknown_zero));

	int k;
	for (k = 0; k < 21; k++) {
	OUT(itemicons[k]);
	}
	// Swap last 2 slots for Arrow and Power Source
	eq->itemicons[21] = emu->itemicons[22];
	eq->unknown_zero2 = emu->itemicons[21];
	strn0cpy(eq->text, emu->text, sizeof(eq->text));

	FINISH_ENCODE();
	}*/

	ENCODE(OP_InspectBuffs)
	{
		ENCODE_LENGTH_EXACT(InspectBuffs_Struct);
		SETUP_DIRECT_ENCODE(InspectBuffs_Struct, structs::InspectBuffs_Struct);

		// we go over the internal 25 instead of the packet's since no entry is 0, which it will be already
		for (int i = 0; i < BUFF_COUNT; i++) {
			OUT(spell_id[i]);
			OUT(tics_remaining[i]);
		}

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

	ENCODE(OP_InterruptCast)
	{
		ENCODE_LENGTH_EXACT(InterruptCast_Struct);
		SETUP_DIRECT_ENCODE(InterruptCast_Struct, structs::InterruptCast_Struct);

		OUT(spawnid);
		OUT(messageid);

		FINISH_ENCODE();
	}

	ENCODE(OP_ItemLinkResponse) { ENCODE_FORWARD(OP_ItemPacket); }

	ENCODE(OP_ItemPacket)
	{
		//consume the packet
		EQApplicationPacket* in = *p;
		*p = nullptr;

		uchar* __emu_buffer = in->pBuffer;

		ItemPacket_Struct* old_item_pkt = (ItemPacket_Struct *)__emu_buffer;
		EQ::InternalSerializedItem_Struct *int_struct = (EQ::InternalSerializedItem_Struct *)(old_item_pkt->SerializedItem);

		EQ::OutBuffer ob;
		EQ::OutBuffer::pos_type last_pos = ob.tellp();

		ob.write((const char*)__emu_buffer, 4);

		uint32 length;
		SerializeItem(ob, (const EQ::ItemInstance*)int_struct->inst, int_struct->slot_id, 0, old_item_pkt->PacketType);

		if (ob.tellp() == last_pos) {
			LogNetcode("TDS::ENCODE(OP_ItemPacket) Serialization failed on item slot [{}]", int_struct->slot_id);
			delete in;
			return;
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();

		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ItemVerifyReply)
	{
		ENCODE_LENGTH_EXACT(ItemVerifyReply_Struct);
		SETUP_DIRECT_ENCODE(ItemVerifyReply_Struct, structs::ItemVerifyReply_Struct);

		eq->slot = ServerToTDSSlot(emu->slot);
		OUT(spell);
		OUT(target);

		FINISH_ENCODE();
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

	ENCODE(OP_LogServer)
	{
		ENCODE_LENGTH_EXACT(LogServer_Struct);
		SETUP_DIRECT_ENCODE(LogServer_Struct, structs::LogServer_Struct);

		strncpy(eq->worldshortname, emu->worldshortname, sizeof(eq->worldshortname));

		//OUT(enablevoicemacros);	// These two are lost, but must be one of the 1s in unknown[249]
		//OUT(enablemail);
		OUT(enable_pvp);
		OUT(enable_FV);

		eq->unknown016 = 1;
		eq->unknown020[0] = 1;

		eq->unknown249[0] = 1;
		eq->unknown249[1] = 1;
		eq->unknown249[8] = 1;
		eq->unknown249[9] = 1;
		eq->unknown249[12] = 1;
		eq->unknown249[14] = 1;
		eq->unknown249[15] = 1;
		eq->unknown249[16] = 1;

		eq->unknown276[0] = 1.0f;
		eq->unknown276[1] = 1.0f;
		eq->unknown276[6] = 1.0f;

		FINISH_ENCODE();
	}

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Moderate, Logs::Netcode, "TDS::ENCODE(OP_LootItem)");

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToTDSCorpseSlot(emu->slot_id);
		OUT(auto_loot);

		FINISH_ENCODE();
	}

	ENCODE(OP_ManaChange)
	{
		ENCODE_LENGTH_EXACT(ManaChange_Struct);
		SETUP_DIRECT_ENCODE(ManaChange_Struct, structs::ManaChange_Struct);

		OUT(new_mana);
		OUT(stamina);
		OUT(spell_id);
		eq->unknown16 = -1; // Self Interrupt/Success = -1, Fizzle = 1, Other Interrupt = 2?

		FINISH_ENCODE();
	}

	ENCODE(OP_MercenaryDataResponse)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		MercenaryMerchantList_Struct *emu = (MercenaryMerchantList_Struct *)__emu_buffer;

		char *Buffer = (char *)in->pBuffer;

		int PacketSize = sizeof(structs::MercenaryMerchantList_Struct) - 4 + emu->MercTypeCount * 4;
		PacketSize += (sizeof(structs::MercenaryListEntry_Struct) - sizeof(structs::MercenaryStance_Struct)) * emu->MercCount;

		uint32 r;
		uint32 k;
		for (r = 0; r < emu->MercCount; r++)
		{
			PacketSize += sizeof(structs::MercenaryStance_Struct) * emu->Mercs[r].StanceCount;
		}

		auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse, PacketSize);
		Buffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercTypeCount);

		for (r = 0; r < emu->MercTypeCount; r++)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercGrades[r]);
		}

		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);

		for (r = 0; r < emu->MercCount; r++)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercID);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercType);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercSubType);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].PurchaseCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].UpkeepCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Status);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyUpkeep);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyType);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->Mercs[r].MercUnk01);
			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->Mercs[r].TimeLeft);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MerchantSlot);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercUnk02);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].StanceCount);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercUnk03);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->Mercs[r].MercUnk04);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// MercName
			for (k = 0; k < emu->Mercs[r].StanceCount; k++)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].StanceIndex);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].Stance);
			}
		}

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_MercenaryDataUpdate)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		MercenaryDataUpdate_Struct *emu = (MercenaryDataUpdate_Struct *)__emu_buffer;

		char *Buffer = (char *)in->pBuffer;

		EQApplicationPacket *outapp;

		uint32 PacketSize = 0;

		// There are 2 different sized versions of this packet depending if a merc is hired or not
		if (emu->MercStatus >= 0)
		{
			PacketSize += sizeof(structs::MercenaryDataUpdate_Struct) + (sizeof(structs::MercenaryData_Struct) - sizeof(structs::MercenaryStance_Struct)) * emu->MercCount;

			uint32 r;
			uint32 k;
			for (r = 0; r < emu->MercCount; r++)
			{
				PacketSize += sizeof(structs::MercenaryStance_Struct) * emu->MercData[r].StanceCount;
				PacketSize += strlen(emu->MercData[r].MercName);	// Null Terminator size already accounted for in the struct
			}

			outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, PacketSize);
			Buffer = (char *)outapp->pBuffer;

			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercStatus);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);

			for (r = 0; r < emu->MercCount; r++)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercID);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercType);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercSubType);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].PurchaseCost);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].UpkeepCost);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Status);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyCost);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyUpkeep);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyType);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->MercData[r].MercUnk01);
				VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercData[r].TimeLeft);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MerchantSlot);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercUnk02);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].StanceCount);
				VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercData[r].MercUnk03);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->MercData[r].MercUnk04);
				//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// MercName
				VARSTRUCT_ENCODE_STRING(Buffer, emu->MercData[r].MercName);
				for (k = 0; k < emu->MercData[r].StanceCount; k++)
				{
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].StanceIndex);
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].Stance);
				}
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);	// MercUnk05
			}
		}
		else
		{
			PacketSize += sizeof(structs::NoMercenaryHired_Struct);

			outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, PacketSize);
			Buffer = (char *)outapp->pBuffer;

			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercStatus);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);
		}

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_MoveItem)
	{
		ENCODE_LENGTH_EXACT(MoveItem_Struct);
		SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Moderate, Logs::Netcode, "TDS::ENCODE(OP_MoveItem)");

		eq->from_slot = ServerToTDSSlot(emu->from_slot);
		eq->to_slot = ServerToTDSSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_NewSpawn) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_NewZone)
	{
		SETUP_DIRECT_ENCODE(NewZone_Struct, structs::NewZone_Struct);

		OUT_str(char_name);
		OUT_str(zone_short_name);
		OUT_str(zone_long_name);
		OUT(ztype);
		int r;
		for (r = 0; r < 4; r++) {
			OUT(fog_red[r]);
			OUT(fog_green[r]);
			OUT(fog_blue[r]);
			OUT(fog_minclip[r]);
			OUT(fog_maxclip[r]);
		}
		OUT(gravity);
		OUT(time_type);
		for (r = 0; r < 4; r++) {
			OUT(rain_chance[r]);
		}
		for (r = 0; r < 4; r++) {
			OUT(rain_duration[r]);
		}
		for (r = 0; r < 4; r++) {
			OUT(snow_chance[r]);
		}
		for (r = 0; r < 4; r++) {
			OUT(snow_duration[r]);
		}
		for (r = 0; r < 32; r++) {
			eq->unknown537[r] = 0xFF;	//observed
		}
		OUT(sky);
		OUT(zone_exp_multiplier);
		OUT(safe_y);
		OUT(safe_x);
		OUT(safe_z);
		OUT(max_z);
		OUT(underworld);
		OUT(minclip);
		OUT(maxclip);
		OUT_str(zone_short_name2);
		OUT(zone_id);
		OUT(zone_instance);
		OUT(SuspendBuffs);

		eq->FogDensity = emu->fog_density;

		/*fill in some unknowns with observed values, hopefully it will help */
		eq->unknown569 = 0;
		eq->unknown571 = 0;
		eq->unknown572 = 4;
		eq->unknown576 = 2;
		eq->unknown580 = 0;

		eq->unknown800 = -1;
		eq->unknown844 = 600;
		eq->unknown848 = 2008; // Guild Lobby observed value
		eq->unknown880 = 50;
		eq->unknown884 = 10;
		eq->unknown888 = 1;
		eq->unknown889 = 0;
		eq->unknown890 = 1;
		eq->unknown891 = 0;
		eq->unknown892 = 0;
		eq->unknown893 = 0;
		eq->fall_damage = 0;	// 0 = Fall Damage on, 1 = Fall Damage off
		eq->unknown895 = 0;
		eq->unknown896 = 180;
		eq->unknown900 = 180;
		eq->unknown904 = 180;
		eq->unknown908 = 2;
		eq->unknown912 = 2;
		eq->unknown932 = -1;	// Set from PoK Example
		eq->unknown936 = -1;	// Set from PoK Example
		eq->unknown944 = 1.0;	// Set from PoK Example
		eq->unknown948 = 0;		// New on Live as of Dec 15 2014
		eq->unknown952 = 100;	// New on Live as of Dec 15 2014

		FINISH_ENCODE();
	}

	ENCODE(OP_OnLevelMessage)
	{
		ENCODE_LENGTH_EXACT(OnLevelMessage_Struct);
		SETUP_DIRECT_ENCODE(OnLevelMessage_Struct, structs::OnLevelMessage_Struct);

		// This packet is variable sized now, but forcing it to the old packet size for now.
		eq->Title_Count = 128;
		memcpy(eq->Title, emu->Title, sizeof(eq->Title));
		eq->Text_Count = 4096;
		memcpy(eq->Text, emu->Text, sizeof(eq->Text));
		OUT(Buttons);
		OUT(Duration);
		OUT(PopupID);
		OUT(NegativeID);
		// These two field names are used if Buttons == 1. We should add an interface to them via Perl.
		eq->ButtonName0_Count = 25;
		OUT_str(ButtonName0);
		eq->ButtonName1_Count = 25;
		OUT_str(ButtonName1);

		FINISH_ENCODE();
	}

	/*
	ENCODE(OP_OpenNewTasksWindow)
	{
	AvailableTaskHeader_Struct*	__emu_AvailableTaskHeader;
	AvailableTaskData1_Struct* 	__emu_AvailableTaskData1;
	AvailableTaskData2_Struct* 	__emu_AvailableTaskData2;
	AvailableTaskTrailer_Struct* 	__emu_AvailableTaskTrailer;

	structs::AvailableTaskHeader_Struct*	__eq_AvailableTaskHeader;
	structs::AvailableTaskData1_Struct* 	__eq_AvailableTaskData1;
	structs::AvailableTaskData2_Struct* 	__eq_AvailableTaskData2;
	structs::AvailableTaskTrailer_Struct* 	__eq_AvailableTaskTrailer;

	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *__emu_buffer = in->pBuffer;

	__emu_AvailableTaskHeader = (AvailableTaskHeader_Struct*)__emu_buffer;

	// For each task, SoF has an extra uint32 and what appears to be space for a null terminated string.
	//
	in->size = in->size + (__emu_AvailableTaskHeader->TaskCount * 5);

	in->pBuffer = new unsigned char[in->size];

	unsigned char *__eq_buffer = in->pBuffer;

	__eq_AvailableTaskHeader = (structs::AvailableTaskHeader_Struct*)__eq_buffer;

	char *__eq_ptr, *__emu_Ptr;

	// Copy Header
	//
	//

	__eq_AvailableTaskHeader->TaskCount = __emu_AvailableTaskHeader->TaskCount;
	__eq_AvailableTaskHeader->unknown1 = __emu_AvailableTaskHeader->unknown1;
	__eq_AvailableTaskHeader->TaskGiver = __emu_AvailableTaskHeader->TaskGiver;

	__emu_Ptr = (char *) __emu_AvailableTaskHeader + sizeof(AvailableTaskHeader_Struct);
	__eq_ptr = (char *) __eq_AvailableTaskHeader + sizeof(structs::AvailableTaskHeader_Struct);

	for(uint32 i=0; i<__emu_AvailableTaskHeader->TaskCount; i++) {

	__emu_AvailableTaskData1 = (AvailableTaskData1_Struct*)__emu_Ptr;
	__eq_AvailableTaskData1 = (structs::AvailableTaskData1_Struct*)__eq_ptr;

	__eq_AvailableTaskData1->TaskID = __emu_AvailableTaskData1->TaskID;
	// This next unknown seems to affect the colour of the task title. 0x3f80000 is what I have seen
	// in RoF2 packets. Changing it to 0x3f000000 makes the title red.
	__eq_AvailableTaskData1->unknown1 = 0x3f800000;
	__eq_AvailableTaskData1->TimeLimit = __emu_AvailableTaskData1->TimeLimit;
	__eq_AvailableTaskData1->unknown2 = __emu_AvailableTaskData1->unknown2;

	__emu_Ptr += sizeof(AvailableTaskData1_Struct);
	__eq_ptr += sizeof(structs::AvailableTaskData1_Struct);

	strcpy(__eq_ptr, __emu_Ptr); // Title

	__emu_Ptr += strlen(__emu_Ptr) + 1;
	__eq_ptr += strlen(__eq_ptr) + 1;

	strcpy(__eq_ptr, __emu_Ptr); // Description

	__emu_Ptr += strlen(__emu_Ptr) + 1;
	__eq_ptr += strlen(__eq_ptr) + 1;

	__eq_ptr[0] = 0;
	__eq_ptr += strlen(__eq_ptr) + 1;

	__emu_AvailableTaskData2 = (AvailableTaskData2_Struct*)__emu_Ptr;
	__eq_AvailableTaskData2 = (structs::AvailableTaskData2_Struct*)__eq_ptr;

	__eq_AvailableTaskData2->unknown1 = __emu_AvailableTaskData2->unknown1;
	__eq_AvailableTaskData2->unknown2 = __emu_AvailableTaskData2->unknown2;
	__eq_AvailableTaskData2->unknown3 = __emu_AvailableTaskData2->unknown3;
	__eq_AvailableTaskData2->unknown4 = __emu_AvailableTaskData2->unknown4;

	__emu_Ptr += sizeof(AvailableTaskData2_Struct);
	__eq_ptr += sizeof(structs::AvailableTaskData2_Struct);

	strcpy(__eq_ptr, __emu_Ptr); // Unknown string

	__emu_Ptr += strlen(__emu_Ptr) + 1;
	__eq_ptr += strlen(__eq_ptr) + 1;

	strcpy(__eq_ptr, __emu_Ptr); // Unknown string

	__emu_Ptr += strlen(__emu_Ptr) + 1;
	__eq_ptr += strlen(__eq_ptr) + 1;

	__emu_AvailableTaskTrailer = (AvailableTaskTrailer_Struct*)__emu_Ptr;
	__eq_AvailableTaskTrailer = (structs::AvailableTaskTrailer_Struct*)__eq_ptr;

	__eq_AvailableTaskTrailer->ItemCount = __emu_AvailableTaskTrailer->ItemCount;
	__eq_AvailableTaskTrailer->unknown1 = __emu_AvailableTaskTrailer->unknown1;
	__eq_AvailableTaskTrailer->unknown2 = __emu_AvailableTaskTrailer->unknown2;
	__eq_AvailableTaskTrailer->StartZone = __emu_AvailableTaskTrailer->StartZone;

	__emu_Ptr += sizeof(AvailableTaskTrailer_Struct);
	__eq_ptr += sizeof(structs::AvailableTaskTrailer_Struct);

	strcpy(__eq_ptr, __emu_Ptr); // Unknown string

	__emu_Ptr += strlen(__emu_Ptr) + 1;
	__eq_ptr += strlen(__eq_ptr) + 1;
	}

	delete[] __emu_buffer;
	dest->FastQueuePacket(&in, ack_req);
	}
	*/

	ENCODE(OP_PetBuffWindow)
	{
		// The format of the RoF2 packet is identical to the OP_BuffCreate packet.

		SETUP_VAR_ENCODE(PetBuff_Struct);

		uint32 sz = 12 + (17 * emu->buffcount);
		__packet->size = sz;
		__packet->pBuffer = new unsigned char[sz];
		memset(__packet->pBuffer, 0, sz);

		__packet->WriteUInt32(emu->petid);
		__packet->WriteUInt32(0);		// PlayerID ?
		__packet->WriteUInt8(1);		// 1 indicates all buffs on the pet (0 to add or remove a single buff)
		__packet->WriteUInt16(emu->buffcount);

		for (uint16 i = 0; i < BUFF_COUNT; ++i)
		{
			if (emu->spellid[i])
			{
				__packet->WriteUInt32(i);
				__packet->WriteUInt32(emu->spellid[i]);
				__packet->WriteUInt32(emu->ticsremaining[i]);
				__packet->WriteUInt32(0); // Unknown
				__packet->WriteString("");
			}
		}
		__packet->WriteUInt8(0); // Unknown

		FINISH_ENCODE();
	}

	ENCODE(OP_PlayerProfile)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;
		PlayerProfile_Struct* emu = (PlayerProfile_Struct *)__emu_buffer;

		uint32 PacketSize = 40000;	// Calculate this later
		uint32 FieldEnum = 0;

		auto outapp = new EQApplicationPacket(OP_PlayerProfile, PacketSize);

		// *section 1
		outapp->WriteUInt32(0);		// Checksum, we will update this later
		outapp->WriteUInt32(0);		// Checksum size, we will update this later

		// *section 2
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown

		outapp->WriteUInt8(emu->gender);	// Gender
		outapp->WriteUInt32(emu->race);		// Race
		outapp->WriteUInt32(emu->class_);	// Class
		outapp->WriteUInt8(emu->level);		// Level
		outapp->WriteUInt8(emu->level);		// HighestLevel

		// *section 3
		FieldEnum = 5;
		outapp->WriteUInt32(FieldEnum);			// Bind count
		for (int r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(emu->binds[r].zoneId);
			outapp->WriteFloat(emu->binds[r].x);
			outapp->WriteFloat(emu->binds[r].y);
			outapp->WriteFloat(emu->binds[r].z);
			outapp->WriteFloat(emu->binds[r].heading);
		}

		// *section 4
		outapp->WriteUInt32(emu->deity);
		outapp->WriteUInt32(emu->intoxication);

		// *section 5
		FieldEnum = 10;
		outapp->WriteUInt32(FieldEnum);		// Unknown count
		for (int r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);		// Unknown
		}

		// *section 6 (don't believe these assignments are correct)
		FieldEnum = 22;
		outapp->WriteUInt32(FieldEnum);		// Equipment count
		for (int r = EQ::textures::textureBegin; r < EQ::textures::materialCount; r++) {
			outapp->WriteUInt32(emu->item_material.Slot[r].Material);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}
		// Write zeroes for the next 13 equipment slots
		for (int r = 9; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		// *section 7
		FieldEnum = EQ::textures::materialCount;
		outapp->WriteUInt32(FieldEnum);		// Equipment2 count
		for (int r = EQ::textures::textureBegin; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		// *section 8
		FieldEnum = EQ::textures::materialCount;
		outapp->WriteUInt32(FieldEnum);		// Tint Count
		for (int r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(emu->item_tint.Slot[r].Color);
		}
		// Write zeroes for extra two tint values (< 7)
		//outapp->WriteUInt32(0);
		//outapp->WriteUInt32(0);

		// *section 9
		FieldEnum = EQ::textures::materialCount;
		outapp->WriteUInt32(FieldEnum);		// Tint2 Count
		for (int r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(emu->item_tint.Slot[r].Color);
		}
		// Write zeroes for extra two tint values (< 7)
		//outapp->WriteUInt32(0);
		//outapp->WriteUInt32(0);

		// section 10
		outapp->WriteUInt8(emu->haircolor);
		outapp->WriteUInt8(emu->beardcolor);
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt8(emu->eyecolor1);
		outapp->WriteUInt8(emu->eyecolor2);
		outapp->WriteUInt8(emu->hairstyle);
		outapp->WriteUInt8(emu->beard);
		outapp->WriteUInt8(emu->face);
		outapp->WriteUInt8(0);			// oldface
		outapp->WriteUInt32(emu->drakkin_heritage);
		outapp->WriteUInt32(emu->drakkin_tattoo);
		outapp->WriteUInt32(emu->drakkin_details);
		outapp->WriteUInt8(0xff);		// Unknown 0xff
		outapp->WriteUInt8(1);			// Unknown 1
		outapp->WriteUInt8(0xff);		// Unknown 0xff
		outapp->WriteUInt8(1);			// Unknown 1
		outapp->WriteFloat(5.0f);		// Height
		outapp->WriteFloat(3.0f);		// Unknown 3.0
		outapp->WriteFloat(2.5f);		// Unknown 2.5
		outapp->WriteFloat(5.5f);		// Unknown 5.5
		outapp->WriteUInt32(0);			// Primary ?
		outapp->WriteUInt32(0);			// Secondary ?
		outapp->WriteUInt32(emu->points);	// Unspent skill points
		outapp->WriteUInt32(emu->mana);
		outapp->WriteUInt32(emu->cur_hp);
		outapp->WriteUInt32(emu->STR);
		outapp->WriteUInt32(emu->STA);
		outapp->WriteUInt32(emu->CHA);
		outapp->WriteUInt32(emu->DEX);
		outapp->WriteUInt32(emu->INT);
		outapp->WriteUInt32(emu->AGI);
		outapp->WriteUInt32(emu->WIS);
		outapp->WriteUInt32(0);			// Unknown (hSTR?)
		outapp->WriteUInt32(0);			// Unknown (hSTA?)
		outapp->WriteUInt32(0);			// Unknown (hCHA?)
		outapp->WriteUInt32(0);			// Unknown (hDEX?)
		outapp->WriteUInt32(0);			// Unknown (hINT?)
		outapp->WriteUInt32(0);			// Unknown (hAGI?)
		outapp->WriteUInt32(0);			// Unknown (hWIS?)

		// *section 11
		FieldEnum = structs::MAX_PP_AA_ARRAY; // 300
		outapp->WriteUInt32(FieldEnum);		// AA Count
		for (uint32 r = 0; r < MAX_PP_AA_ARRAY; r++) {
			outapp->WriteUInt32(emu->aa_array[r].AA);
			outapp->WriteUInt32(emu->aa_array[r].value);
			outapp->WriteUInt32(emu->aa_array[r].charges);
		}
		for (uint32 r = MAX_PP_AA_ARRAY; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		// *section 12
		FieldEnum = structs::MAX_PP_SKILL; // 100
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < MAX_PP_SKILL; r++) {
			outapp->WriteUInt32(emu->skills[r]);
		}
		for (uint32 r = MAX_PP_SKILL; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
		}

		// *section 13
		FieldEnum = 25;
		outapp->WriteUInt32(FieldEnum);			// Unknown count
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);			// Unknown
		}

		// *section 14
		FieldEnum = structs::MAX_PP_DISCIPLINES; // 300
		outapp->WriteUInt32(structs::MAX_PP_DISCIPLINES);	// Discipline count
		for (uint32 r = 0; r < MAX_PP_DISCIPLINES; r++) {
			outapp->WriteUInt32(emu->disciplines.values[r]);
		}
		for (uint32 r = MAX_PP_DISCIPLINES; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
		}

		// *section 15
		FieldEnum = 20;
		outapp->WriteUInt32(FieldEnum);			// Timestamp count
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
		}

		// *section 16
		FieldEnum = structs::MAX_RECAST_TYPES; // 20
		outapp->WriteUInt32(MAX_RECAST_TYPES);			// Timestamp count
		for (uint32 r = 0; r < MAX_RECAST_TYPES; r++) {
			outapp->WriteUInt32(emu->recastTimers[r]);
		}
		for (uint32 r = MAX_RECAST_TYPES; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
		}

		// *section 17
		FieldEnum = 100;
		outapp->WriteUInt32(FieldEnum);			// Timestamp2 count
		for (uint32 r = 0; r < 100; r++) {
			outapp->WriteUInt32(0);
		}

		// *section 18
		FieldEnum = spells::SPELLBOOK_SIZE; // 800
		outapp->WriteUInt32(FieldEnum);		// Spellbook slots
		for (uint32 r = 0; r < spells::SPELLBOOK_SIZE; r++) { // TODO: Compare EQ::spells::SPELLBOOK_SIZE and spells::SPELL_ID_MAX
			outapp->WriteUInt32(emu->spell_book[r]);
		}
		for (uint32 r = spells::SPELLBOOK_SIZE; r < FieldEnum; r++) {
			outapp->WriteUInt32(0xFFFFFFFFU);
		}

		// *section 19
		FieldEnum = spells::SPELL_GEM_COUNT; // 16
		outapp->WriteUInt32(structs::MAX_PP_MEMSPELL);		// Memorised spell slots
		for (uint32 r = 0; r < spells::SPELL_GEM_COUNT; r++) { // TODO: See if client can handle more than 12 (EQ::spells::SPELL_GEM_COUNT)
			outapp->WriteUInt32(emu->mem_spells[r]);
		}
		for (uint32 r = spells::SPELL_GEM_COUNT; r < FieldEnum; r++) {
			outapp->WriteUInt32(0xFFFFFFFFU);
		}

		// *section 20
		FieldEnum = 13;
		outapp->WriteUInt32(FieldEnum);			// Unknown count
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);			// Unknown
		}

		// *section 21
		outapp->WriteUInt8(0);			// Unknown

		// *section 22
		FieldEnum = structs::BUFF_COUNT; // 42
		outapp->WriteUInt32(structs::BUFF_COUNT);
		for (uint32 r = 0; r < BUFF_COUNT; r++) {
			float instrument_mod = 0.0f;
			uint8 slotid = emu->buffs[r].effect_type;
			uint32 player_id = emu->buffs[r].player_id;
			if (emu->buffs[r].spellid != 0xFFFF && emu->buffs[r].spellid != 0) {
				instrument_mod = 1.0f + (emu->buffs[r].bard_modifier - 10) / 10.0f;
				slotid = 2;
				player_id = 0x000717fd;
			}
			else {
				slotid = 0;
			}
			outapp->WriteUInt8(0);		// Had this as slot, but always appears to be 0 on live.
			outapp->WriteFloat(instrument_mod);
			outapp->WriteUInt32(player_id);
			outapp->WriteUInt8(0);
			outapp->WriteUInt32(emu->buffs[r].counters);
			//outapp->WriteUInt8(emu->buffs[r].bard_modifier);
			outapp->WriteUInt32(emu->buffs[r].duration);
			outapp->WriteUInt8(emu->buffs[r].level);
			outapp->WriteUInt32(emu->buffs[r].spellid);
			outapp->WriteUInt32(slotid);			// Only ever seen 2
			outapp->WriteUInt32(0);
			outapp->WriteUInt8(0);
			outapp->WriteUInt32(emu->buffs[r].counters);	// Appears twice ?
			for (uint32 j = 0; j < 44; ++j) {
				outapp->WriteUInt8(0);	// Unknown
			}
		}
		for (uint32 r = BUFF_COUNT; r < FieldEnum; r++) {
			for (uint32 j = 0; j < 20; ++j) { // 80 bytes of zeroes
				outapp->WriteUInt32(0);
			}
		}

		// *section 23
		outapp->WriteUInt32(emu->platinum);
		outapp->WriteUInt32(emu->gold);
		outapp->WriteUInt32(emu->silver);
		outapp->WriteUInt32(emu->copper);
		outapp->WriteUInt32(emu->platinum_cursor);
		outapp->WriteUInt32(emu->gold_cursor);
		outapp->WriteUInt32(emu->silver_cursor);
		outapp->WriteUInt32(emu->copper_cursor);
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// This is the cooldown timer for the monk 'Mend' skill. Client will add 6 minutes to this value the first time the
		// player logs in. After that it will honour whatever value we send here.
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(emu->thirst_level);
		outapp->WriteUInt32(emu->hunger_level);

		// section 24
		outapp->WriteUInt32(emu->aapoints_spent); // did not show up..may be due to lack of aa's
		FieldEnum = 6;
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
		}
		outapp->WriteUInt32(emu->aapoints);		// AA Points unspent

		// section 25
		outapp->WriteUInt8(0);				// Hide
		outapp->WriteUInt8(0);				// Sneak

		// *section 26
		FieldEnum = profile::BANDOLIERS_SIZE; // 20
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < EQ::profile::BANDOLIERS_SIZE && r < FieldEnum; ++r) {
			outapp->WriteString(emu->bandoliers[r].Name);
			for (uint32 j = 0; j < profile::BANDOLIER_ITEM_COUNT; ++j) { // Will need adjusting if 'server != client' is ever true
				outapp->WriteString(emu->bandoliers[r].Items[j].Name);
				outapp->WriteUInt32(emu->bandoliers[r].Items[j].ID);
				if (emu->bandoliers[r].Items[j].Icon) {
					outapp->WriteSInt32(emu->bandoliers[r].Items[j].Icon);
				}
				else {
					outapp->WriteSInt32(-1); // If no icon, it must send -1 or Treasure Chest Icon (836) is displayed
				}
			}
		}
		for (uint32 r = EQ::profile::BANDOLIERS_SIZE; r < FieldEnum; ++r) {
			outapp->WriteString("");
			for (uint32 j = 0; j < profile::BANDOLIER_ITEM_COUNT; ++j) { // Will need adjusting if 'server != client' is ever true
				outapp->WriteString("");
				outapp->WriteUInt32(0);
				outapp->WriteSInt32(-1);
			}
		}

		// *section 27
		FieldEnum = profile::POTION_BELT_SIZE; // 5
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < EQ::profile::POTION_BELT_SIZE && r < FieldEnum; ++r) {
			outapp->WriteString(emu->potionbelt.Items[r].Name);
			outapp->WriteUInt32(emu->potionbelt.Items[r].ID);
			if (emu->potionbelt.Items[r].Icon) {
				outapp->WriteSInt32(emu->potionbelt.Items[r].Icon);
			}
			else {
				outapp->WriteSInt32(-1); // If no icon, it must send -1 or Treasure Chest Icon (836) is displayed
			}
		}
		for (uint32 r = EQ::profile::POTION_BELT_SIZE; r < FieldEnum; ++r) {
			outapp->WriteString("");
			outapp->WriteUInt32(0);
			outapp->WriteSInt32(-1);
		}

		// *section 28
		outapp->WriteSInt32(-1);	// Unknown;
		outapp->WriteSInt32(123);	// HP Total ?
		outapp->WriteSInt32(234);	// Endurance Total ?
		outapp->WriteSInt32(345);	// Mana Total ?
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(22);	// Unknown - Expansion count ? (was 20)
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(emu->endurance);

		// *section 29
		outapp->WriteUInt32(0);		// Unknown - Observed 0x7cde - This is also seen in guild packets sent to this character.
		outapp->WriteUInt32(0);		// Unknown - Observed 0x64

		// section 30
		FieldEnum = 64;
		outapp->WriteUInt32(FieldEnum);	// Name Length
		uint32 CurrentPosition = outapp->GetWritePosition();
		outapp->WriteString(emu->name);
		outapp->SetWritePosition(CurrentPosition + FieldEnum);

		// *section 31
		FieldEnum = 32;
		outapp->WriteUInt32(FieldEnum);	// Last Name Length
		CurrentPosition = outapp->GetWritePosition();
		outapp->WriteString(emu->last_name);
		outapp->SetWritePosition(CurrentPosition + FieldEnum);

		// *section 32
		outapp->WriteUInt32(emu->birthday);
		outapp->WriteUInt32(emu->birthday);		// Account start date ?
		outapp->WriteUInt32(emu->lastlogin);
		outapp->WriteUInt32(emu->timePlayedMin);
		outapp->WriteUInt32(emu->timeentitledonaccount);
		outapp->WriteUInt32(emu->expansions);

		// *section 33
		FieldEnum = structs::MAX_PP_LANGUAGE; // 32
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < MAX_PP_LANGUAGE; r++) {
			outapp->WriteUInt8(emu->languages[r]);
		}
		for (uint32 r = MAX_PP_LANGUAGE; r < FieldEnum; r++) {
			outapp->WriteUInt8(0);
		}

		// *section 34
		outapp->WriteUInt16(emu->zone_id);
		outapp->WriteUInt16(emu->zoneInstance);
		outapp->WriteFloat(emu->y);
		outapp->WriteFloat(emu->x);
		outapp->WriteFloat(emu->z);
		outapp->WriteFloat(emu->heading);
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(emu->pvp);
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(emu->gm);
		outapp->WriteUInt32(emu->guild_id);
		outapp->WriteUInt8(emu->guildrank);	// guildrank
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);			// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt64(emu->exp);		// int32 in client
		outapp->WriteUInt8(5);			// Unknown - Seen 5 on Live - Eye Height?
		outapp->WriteUInt32(emu->platinum_bank);
		outapp->WriteUInt32(emu->gold_bank);
		outapp->WriteUInt32(emu->silver_bank);
		outapp->WriteUInt32(emu->copper_bank);
		outapp->WriteUInt32(emu->platinum_shared);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		// *section 35
		FieldEnum = 20;
		outapp->WriteUInt32(FieldEnum);				// Unknown
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		// *section 36
		outapp->WriteSInt32(-1);				// Unknown
		outapp->WriteSInt32(-1);				// Unknown
		outapp->WriteUInt32(emu->career_tribute_points);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(emu->tribute_points);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown

		// *section 37
		FieldEnum = EQ::invtype::TRIBUTE_SIZE; // 5
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++) {
			outapp->WriteUInt32(emu->tributes[r].tribute);
			outapp->WriteUInt32(emu->tributes[r].tier);
		}

		// *section 38
		FieldEnum = 10;
		outapp->WriteUInt32(FieldEnum);		// Guild Tribute Count ?
		for (uint32 r = 0; r < 10; r++) {
			outapp->WriteUInt32(0xffffffff);
			outapp->WriteUInt32(0);
		}

		// *section 39
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		// *section 40
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		for (uint32 r = 0; r < 114; r++) {
			outapp->WriteUInt8(0);			// Unknown
		}
		outapp->WriteUInt16(16256);
		for (uint32 r = 116; r < 124; r++) {
			outapp->WriteUInt8(0);			// Unknown
		}

		// section 41 - removed 1 uint32 write from original sequence..did not match up - could have been FieldEnum for section 42
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(emu->currentRadCrystals);
		outapp->WriteUInt32(emu->careerRadCrystals);
		outapp->WriteUInt32(emu->currentEbonCrystals);
		outapp->WriteUInt32(emu->careerEbonCrystals);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Loyalty Tokens
		outapp->WriteUInt32(0);				// Loyalty Velocity?

		// *section 42
		FieldEnum = 64;
		outapp->WriteUInt32(FieldEnum);			// Unknown
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt8(0);				// Unknown
		}

		// *section 43
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		// *section 44
		for (uint32 r = 0; r < 31; r++) {
			outapp->WriteUInt8(0);				// Unknown
		}

		// *section 45
		outapp->WriteUInt32(0); // uint32 - GuildTributePoolPoints? (seems to be shared across guild?)

		// *section 46
		FieldEnum = 6;
		outapp->WriteUInt32(FieldEnum);		// Count ... of LDoN stats ?
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(emu->ldon_points_guk);
		outapp->WriteUInt32(emu->ldon_points_mir);
		outapp->WriteUInt32(emu->ldon_points_mmc);
		outapp->WriteUInt32(emu->ldon_points_ruj);
		outapp->WriteUInt32(emu->ldon_points_tak);
		outapp->WriteUInt32(emu->ldon_points_available);

		// *section 47
		outapp->WriteUInt32(emu->air_remaining);		// ?
		outapp->WriteUInt32(emu->PVPKills);
		outapp->WriteUInt32(emu->PVPDeaths);
		outapp->WriteUInt32(emu->PVPCurrentPoints);
		outapp->WriteUInt32(emu->PVPCareerPoints);
		outapp->WriteUInt32(emu->PVPBestKillStreak);
		outapp->WriteUInt32(emu->PVPWorstDeathStreak);
		outapp->WriteUInt32(emu->PVPCurrentKillStreak);

		// *section 48
		outapp->WriteString(emu->PVPLastKill.Name);
		outapp->WriteUInt32(emu->PVPLastKill.Level);
		outapp->WriteUInt32(emu->PVPLastKill.Race);
		outapp->WriteUInt32(emu->PVPLastKill.Class);
		outapp->WriteUInt32(emu->PVPLastKill.Zone);
		outapp->WriteUInt32(emu->PVPLastKill.Time);
		outapp->WriteUInt32(emu->PVPLastKill.Points);

		// *section 49
		outapp->WriteString(emu->PVPLastDeath.Name);
		outapp->WriteUInt32(emu->PVPLastDeath.Level);
		outapp->WriteUInt32(emu->PVPLastDeath.Race);
		outapp->WriteUInt32(emu->PVPLastDeath.Class);
		outapp->WriteUInt32(emu->PVPLastDeath.Zone);
		outapp->WriteUInt32(emu->PVPLastDeath.Time);
		outapp->WriteUInt32(emu->PVPLastDeath.Points);

		// *section 50
		outapp->WriteUInt32(emu->PVPNumberOfKillsInLast24Hours);

		// *section 51 - may have to test as 'FieldEnum = 0'
		FieldEnum = 50; // Last 50 Kills
		outapp->WriteUInt32(FieldEnum);
		for (uint32 r = 0; r < FieldEnum; ++r) {
			outapp->WriteString(emu->PVPRecentKills[r].Name);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Level);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Race);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Class);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Zone);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Time);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Points);
		}

		// *section 52
		outapp->WriteUInt32(emu->expAA);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Krono - itemid 88888 Hard coded in client?
		outapp->WriteUInt8(emu->groupAutoconsent);
		outapp->WriteUInt8(emu->raidAutoconsent);
		outapp->WriteUInt8(emu->guildAutoconsent);
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt32(emu->level);	// Level3 ?
		outapp->WriteUInt8(emu->showhelm);
		outapp->WriteUInt32(emu->RestTimer);

		// *section 53
		FieldEnum = 1024;
		outapp->WriteUInt32(FieldEnum);			// Unknown Count
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt8(0);				// Unknown
		}

		// *section 54
		FieldEnum = 0;
		outapp->WriteUInt32(FieldEnum);			// Unknown Count
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
		}

		// *section 55
		FieldEnum = 0; // can probably set to 0
		outapp->WriteUInt32(FieldEnum);			// Unknown Count
		for (uint32 r = 0; r < FieldEnum; r++) {
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
			outapp->WriteUInt32(0);				// Unknown
		}

		// *section 56
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(1);				// Unknown - seen 1

		// *section 57
		outapp->WriteUInt8(0);				// Padding

		LogNetcode("[STRUCTS] Player Profile Packet is [{}] bytes", outapp->GetWritePosition());

		auto NewBuffer = new unsigned char[outapp->GetWritePosition()];
		memcpy(NewBuffer, outapp->pBuffer, outapp->GetWritePosition());
		safe_delete_array(outapp->pBuffer);
		outapp->pBuffer = NewBuffer;
		outapp->size = outapp->GetWritePosition();
		outapp->SetWritePosition(4);
		outapp->WriteUInt32(outapp->size - 9);

		CRC32::SetEQChecksum(outapp->pBuffer, outapp->size - 1, 8);
		//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
		return;
	}

	ENCODE(OP_RaidJoin)
	{
		EQApplicationPacket *inapp = *p;
		unsigned char * __emu_buffer = inapp->pBuffer;
		RaidCreate_Struct *raid_create = (RaidCreate_Struct*)__emu_buffer;

		auto outapp_create = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidGeneral_Struct));
		structs::RaidGeneral_Struct *general = (structs::RaidGeneral_Struct*)outapp_create->pBuffer;

		general->action = 8;
		general->parameter = 1;
		strn0cpy(general->leader_name, raid_create->leader_name, 64);
		strn0cpy(general->player_name, raid_create->leader_name, 64);

		dest->FastQueuePacket(&outapp_create);
		safe_delete(inapp);
	}

	ENCODE(OP_RaidUpdate)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;
		unsigned char * __emu_buffer = inapp->pBuffer;
		RaidGeneral_Struct *raid_gen = (RaidGeneral_Struct*)__emu_buffer;

		if (raid_gen->action == 0) // raid add has longer length than other raid updates
		{
			RaidAddMember_Struct* in_add_member = (RaidAddMember_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidAddMember_Struct));
			structs::RaidAddMember_Struct *add_member = (structs::RaidAddMember_Struct*)outapp->pBuffer;

			add_member->raidGen.action = in_add_member->raidGen.action;
			add_member->raidGen.parameter = in_add_member->raidGen.parameter;
			strn0cpy(add_member->raidGen.leader_name, in_add_member->raidGen.leader_name, 64);
			strn0cpy(add_member->raidGen.player_name, in_add_member->raidGen.player_name, 64);
			add_member->_class = in_add_member->_class;
			add_member->level = in_add_member->level;
			add_member->isGroupLeader = in_add_member->isGroupLeader;
			add_member->flags[0] = in_add_member->flags[0];
			add_member->flags[1] = in_add_member->flags[1];
			add_member->flags[2] = in_add_member->flags[2];
			add_member->flags[3] = in_add_member->flags[3];
			add_member->flags[4] = in_add_member->flags[4];
			dest->FastQueuePacket(&outapp);
		}
		else if (raid_gen->action == 35)
		{
			RaidMOTD_Struct *inmotd = (RaidMOTD_Struct *)__emu_buffer;
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidMOTD_Struct) +
										 strlen(inmotd->motd) + 1);
			structs::RaidMOTD_Struct *outmotd = (structs::RaidMOTD_Struct *)outapp->pBuffer;

			outmotd->general.action = inmotd->general.action;
			strn0cpy(outmotd->general.player_name, inmotd->general.player_name, 64);
			strn0cpy(outmotd->motd, inmotd->motd, strlen(inmotd->motd) + 1);
			dest->FastQueuePacket(&outapp);
		}
		else if (raid_gen->action == 14 || raid_gen->action == 30)
		{
			RaidLeadershipUpdate_Struct *inlaa = (RaidLeadershipUpdate_Struct *)__emu_buffer;
			auto outapp =
			    new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidLeadershipUpdate_Struct));
			structs::RaidLeadershipUpdate_Struct *outlaa = (structs::RaidLeadershipUpdate_Struct *)outapp->pBuffer;

			outlaa->action = inlaa->action;
			strn0cpy(outlaa->player_name, inlaa->player_name, 64);
			strn0cpy(outlaa->leader_name, inlaa->leader_name, 64);
			memcpy(&outlaa->raid, &inlaa->raid, sizeof(RaidLeadershipAA_Struct));
			dest->FastQueuePacket(&outapp);
		}
		else
		{
			RaidGeneral_Struct* in_raid_general = (RaidGeneral_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidGeneral_Struct));
			structs::RaidGeneral_Struct *raid_general = (structs::RaidGeneral_Struct*)outapp->pBuffer;
			strn0cpy(raid_general->leader_name, in_raid_general->leader_name, 64);
			strn0cpy(raid_general->player_name, in_raid_general->player_name, 64);
			raid_general->action = in_raid_general->action;
			raid_general->parameter = in_raid_general->parameter;
			dest->FastQueuePacket(&outapp);
		}

		safe_delete(inapp);
	}

	ENCODE(OP_ReadBook)
	{
		ENCODE_LENGTH_ATLEAST(BookText_Struct);
		SETUP_DIRECT_ENCODE(BookText_Struct, structs::BookRequest_Struct);

		if (emu->window == 0xFF)
			eq->window = 0xFFFFFFFF;
		else
			eq->window = emu->window;
		OUT(type);
		OUT(invslot);
		strn0cpy(eq->txtfile, emu->booktext, sizeof(eq->txtfile));

		FINISH_ENCODE();
	}

	ENCODE(OP_RecipeAutoCombine)
	{
		ENCODE_LENGTH_EXACT(RecipeAutoCombine_Struct);
		SETUP_DIRECT_ENCODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

		OUT(object_type);
		OUT(some_id);
		eq->container_slot = ServerToTDSSlot(emu->unknown1);
		structs::ItemSlotStruct TDSSlot;
		TDSSlot.SlotType = 8;	// Observed
		TDSSlot.Unknown02 = 0;
		TDSSlot.MainSlot = 0xffff;
		TDSSlot.SubSlot = 0xffff;
		TDSSlot.AugSlot = 0xffff;
		TDSSlot.Unknown01 = 0;
		eq->unknown_slot = TDSSlot;
		OUT(recipe_id);
		OUT(reply_code);

		FINISH_ENCODE();
	}

	ENCODE(OP_RemoveBlockedBuffs) { ENCODE_FORWARD(OP_BlockedBuffs); }

	ENCODE(OP_RequestClientZoneChange)
	{
		ENCODE_LENGTH_EXACT(RequestClientZoneChange_Struct);
		SETUP_DIRECT_ENCODE(RequestClientZoneChange_Struct, structs::RequestClientZoneChange_Struct);

		OUT(zone_id);
		OUT(instance_id);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT(heading);
		eq->type = 0x0b;
		eq->unknown004 = 0xffffffff;
		eq->unknown172 = 0x0168b500;

		FINISH_ENCODE();
	}

	ENCODE(OP_RespondAA)
	{
		SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

		eq->aa_spent = emu->aa_spent;
		// These fields may need to be correctly populated at some point
		eq->aapoints_assigned = emu->aa_spent;
		eq->aa_spent_general = 0;
		eq->aa_spent_archetype = 0;
		eq->aa_spent_class = 0;
		eq->aa_spent_special = 0;

		for (uint32 i = 0; i < MAX_PP_AA_ARRAY; ++i)
		{
			eq->aa_list[i].AA = emu->aa_list[i].AA;
			eq->aa_list[i].value = emu->aa_list[i].value;
			eq->aa_list[i].charges = emu->aa_list[i].charges;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_RezzRequest)
	{
		SETUP_DIRECT_ENCODE(Resurrect_Struct, structs::Resurrect_Struct);

		OUT(zone_id);
		OUT(instance_id);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT_str(your_name);
		OUT_str(rezzer_name);
		OUT(spellid);
		OUT_str(corpse_name);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_SendAATable)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;
		AARankInfo_Struct *emu = (AARankInfo_Struct*)inapp->pBuffer;
		
		// the structs::SendAA_Struct includes enough space for 1 prereq which is the min even if it has no prereqs
		auto prereq_size = emu->total_prereqs > 1 ? (emu->total_prereqs - 1) * 8 : 0;
		auto outapp = new EQApplicationPacket(OP_SendAATable, sizeof(structs::SendAA_Struct) + emu->total_effects * sizeof(structs::AA_Ability) + prereq_size);
		inapp->SetReadPosition(sizeof(AARankInfo_Struct)+emu->total_effects * sizeof(AARankEffect_Struct));
		

		std::vector<int32> skill;
		std::vector<int32> points;
		for(auto i = 0; i < emu->total_prereqs; ++i) {
			skill.push_back(inapp->ReadUInt32());
			points.push_back(inapp->ReadUInt32());
		}

		outapp->WriteUInt32(emu->id);
		outapp->WriteUInt8(1);
		outapp->WriteSInt32(emu->upper_hotkey_sid);
		outapp->WriteSInt32(emu->lower_hotkey_sid);
		outapp->WriteSInt32(emu->title_sid);
		outapp->WriteSInt32(emu->desc_sid);
		outapp->WriteSInt32(emu->level_req);
		outapp->WriteSInt32(emu->cost);
		outapp->WriteUInt32(emu->seq);
		outapp->WriteUInt32(emu->current_level);

		if (emu->total_prereqs) {
			outapp->WriteUInt32(emu->total_prereqs);
			for (auto &e : skill)
				outapp->WriteSInt32(e);
			outapp->WriteUInt32(emu->total_prereqs);
			for (auto &e : points)
				outapp->WriteSInt32(e);
		} else {
			outapp->WriteUInt32(1);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(1);
			outapp->WriteUInt32(0);
 		}

		outapp->WriteSInt32(emu->type);
		outapp->WriteSInt32(emu->spell);
		outapp->WriteSInt32(1);
		outapp->WriteSInt32(emu->spell_type);
		outapp->WriteSInt32(emu->spell_refresh);
		outapp->WriteSInt32(emu->classes);
		outapp->WriteSInt32(emu->max_level);
		outapp->WriteSInt32(emu->prev_id);
		outapp->WriteSInt32(emu->next_id);
		outapp->WriteSInt32(emu->total_cost);
		outapp->WriteUInt8(0);
		outapp->WriteUInt8(emu->grant_only);
		outapp->WriteUInt8(0);
		outapp->WriteUInt32(emu->charges);
		outapp->WriteSInt32(emu->expansion);
		outapp->WriteSInt32(emu->category);
		outapp->WriteUInt8(0); // shroud
		outapp->WriteUInt8(0); // unknown109
		outapp->WriteUInt8(0); // loh
		outapp->WriteUInt8(0); // unknown111
		outapp->WriteUInt32(emu->total_effects);

		inapp->SetReadPosition(sizeof(AARankInfo_Struct));
		for(auto i = 0; i < emu->total_effects; ++i) {
			outapp->WriteUInt32(inapp->ReadUInt32()); // skill_id
			outapp->WriteUInt32(inapp->ReadUInt32()); // base1
			outapp->WriteUInt32(inapp->ReadUInt32()); // base2
			outapp->WriteUInt32(inapp->ReadUInt32()); // slot
 		}
			
		dest->FastQueuePacket(&outapp);
		delete inapp;
	}

	ENCODE(OP_SendCharInfo)
	{
		ENCODE_LENGTH_ATLEAST(CharacterSelect_Struct);
		SETUP_VAR_ENCODE(CharacterSelect_Struct);

		// Zero-character count shunt
		if (emu->CharCount == 0) {
			ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, sizeof(structs::CharacterSelect_Struct));
			eq->CharCount = emu->CharCount;

			FINISH_ENCODE();
			return;
		}

		unsigned char *emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);
		CharacterSelectEntry_Struct *emu_cse = (CharacterSelectEntry_Struct *)nullptr;

		size_t names_length = 0;
		size_t character_count = 0;
		for (; character_count < emu->CharCount && character_count < consts::CHARACTER_CREATION_LIMIT; ++character_count) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;
			names_length += strlen(emu_cse->Name);
			emu_ptr += sizeof(CharacterSelectEntry_Struct);
		}

		size_t total_length = sizeof(structs::CharacterSelect_Struct)
			+ character_count * sizeof(structs::CharacterSelectEntry_Struct)
			+ names_length;

		ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, total_length);
		structs::CharacterSelectEntry_Struct *eq_cse = (structs::CharacterSelectEntry_Struct *)nullptr;

		eq->CharCount = character_count;
		//eq->TotalChars = emu->TotalChars;

		//if (eq->TotalChars > consts::CHARACTER_CREATION_LIMIT)
		//	eq->TotalChars = consts::CHARACTER_CREATION_LIMIT;

		emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);

		unsigned char *eq_ptr = __packet->pBuffer;
		eq_ptr += sizeof(structs::CharacterSelect_Struct);

		for (int counter = 0; counter < character_count; ++counter) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;
			eq_cse = (structs::CharacterSelectEntry_Struct *)eq_ptr; // base address

			strcpy(eq_cse->Name, emu_cse->Name);
			eq_ptr += strlen(emu_cse->Name);
			eq_cse = (structs::CharacterSelectEntry_Struct *)eq_ptr; // offset address (base + name length offset)
			eq_cse->Name[0] = '\0'; // (offset)eq_cse->Name[0] = (base)eq_cse->Name[strlen(emu_cse->Name)]

			eq_cse->Class = emu_cse->Class;
			eq_cse->Race = emu_cse->Race;
			eq_cse->Level = emu_cse->Level;
			eq_cse->ShroudClass = emu_cse->ShroudClass;
			eq_cse->ShroudRace = emu_cse->ShroudRace;
			eq_cse->Zone = emu_cse->Zone;
			eq_cse->Instance = emu_cse->Instance;
			eq_cse->Gender = emu_cse->Gender;
			eq_cse->Face = emu_cse->Face;

			for (int equip_index = 0; equip_index < EQ::textures::materialCount; equip_index++) {
				eq_cse->Equip[equip_index].Material = emu_cse->Equip[equip_index].Material;
				eq_cse->Equip[equip_index].Unknown1 = emu_cse->Equip[equip_index].Unknown1;
				eq_cse->Equip[equip_index].EliteMaterial = emu_cse->Equip[equip_index].EliteModel;
				eq_cse->Equip[equip_index].HeroForgeModel = emu_cse->Equip[equip_index].HerosForgeModel;
				eq_cse->Equip[equip_index].Material2 = emu_cse->Equip[equip_index].Unknown2;
				eq_cse->Equip[equip_index].Color.Color = emu_cse->Equip[equip_index].Color;
			}

			eq_cse->Unknown15 = emu_cse->Unknown15;
			eq_cse->Unknown19 = emu_cse->Unknown19;
			eq_cse->DrakkinTattoo = emu_cse->DrakkinTattoo;
			eq_cse->DrakkinDetails = emu_cse->DrakkinDetails;
			eq_cse->Deity = emu_cse->Deity;
			eq_cse->PrimaryIDFile = emu_cse->PrimaryIDFile;
			eq_cse->SecondaryIDFile = emu_cse->SecondaryIDFile;
			eq_cse->HairColor = emu_cse->HairColor;
			eq_cse->BeardColor = emu_cse->BeardColor;
			eq_cse->EyeColor1 = emu_cse->EyeColor1;
			eq_cse->EyeColor2 = emu_cse->EyeColor2;
			eq_cse->HairStyle = emu_cse->HairStyle;
			eq_cse->Beard = emu_cse->Beard;
			eq_cse->GoHome = emu_cse->GoHome;
			eq_cse->Tutorial = emu_cse->Tutorial;
			eq_cse->DrakkinHeritage = emu_cse->DrakkinHeritage;
			eq_cse->Unknown1 = emu_cse->Unknown1;
			eq_cse->Enabled = emu_cse->Enabled;
			eq_cse->LastLogin = emu_cse->LastLogin;
			eq_cse->Unknown2 = emu_cse->Unknown2;
			eq_cse->Unknown281 = 0;
			eq_cse->Unknown282 = 0;

			emu_ptr += sizeof(CharacterSelectEntry_Struct);
			eq_ptr += sizeof(structs::CharacterSelectEntry_Struct);
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_SendZonepoints)
	{
		SETUP_VAR_ENCODE(ZonePoints);
		ALLOC_VAR_ENCODE(structs::ZonePoints, sizeof(structs::ZonePoints) + sizeof(structs::ZonePoint_Entry) * (emu->count + 1));

		eq->count = emu->count;
		for (uint32 i = 0; i < emu->count; ++i)
		{
			eq->zpe[i].iterator = emu->zpe[i].iterator;
			eq->zpe[i].x = emu->zpe[i].x;
			eq->zpe[i].y = emu->zpe[i].y;
			eq->zpe[i].z = emu->zpe[i].z;
			eq->zpe[i].heading = emu->zpe[i].heading;
			eq->zpe[i].zoneid = emu->zpe[i].zoneid;
			eq->zpe[i].zoneinstance = emu->zpe[i].zoneinstance;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_SetGuildRank)
	{
		ENCODE_LENGTH_EXACT(GuildSetRank_Struct);
		SETUP_DIRECT_ENCODE(GuildSetRank_Struct, structs::GuildSetRank_Struct);

		eq->GuildID = emu->Unknown00;

		/* Translate older ranks to new values */
		switch (emu->Rank) {
		case 0: { eq->Rank = 5; break; }  // GUILD_MEMBER	0
		case 1: { eq->Rank = 3; break; }  // GUILD_OFFICER	1
		case 2: { eq->Rank = 1; break; }  // GUILD_LEADER	2
		default: { eq->Rank = emu->Rank; break; }
		}

		memcpy(eq->MemberName, emu->MemberName, sizeof(eq->MemberName));
		OUT(Banker);
		eq->Unknown76 = 1;

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopPlayerBuy)
	{
		ENCODE_LENGTH_EXACT(Merchant_Sell_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Sell_Struct, structs::Merchant_Sell_Struct);

		OUT(npcid);
		OUT(playerid);
		OUT(itemslot);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopPlayerSell)
	{
		ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		OUT(npcid);
		eq->itemslot = ServerToTDSMainInvSlot(emu->itemslot);
		//OUT(itemslot);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopRequest)
	{
		ENCODE_LENGTH_EXACT(Merchant_Click_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Click_Struct, structs::Merchant_Click_Struct);

		OUT(npcid);
		OUT(playerid);
		OUT(command);
		OUT(rate);
		eq->unknown01 = 3;	// Not sure what these values do yet, but list won't display without them
		eq->unknown02 = 2592000;

		FINISH_ENCODE();
	}

	ENCODE(OP_SkillUpdate)
	{
		ENCODE_LENGTH_EXACT(SkillUpdate_Struct);
		SETUP_DIRECT_ENCODE(SkillUpdate_Struct, structs::SkillUpdate_Struct);

		OUT(skillId);
		OUT(value);
		eq->unknown08 = 1;		// Observed
		eq->unknown09 = 80;		// Observed
		eq->unknown10 = 136;	// Observed
		eq->unknown11 = 54;		// Observed

		FINISH_ENCODE();
	}

	ENCODE(OP_SomeItemPacketMaybe)
	{
		// This Opcode is not named very well. It is used for the animation of arrows leaving the player's bow
		// and flying to the target.
		//

		ENCODE_LENGTH_EXACT(Arrow_Struct);
		SETUP_DIRECT_ENCODE(Arrow_Struct, structs::Arrow_Struct);

		OUT(src_y);
		OUT(src_x);
		OUT(src_z);
		OUT(velocity);
		OUT(launch_angle);
		OUT(tilt);
		OUT(arc);
		OUT(source_id);
		OUT(target_id);
		OUT(item_id);

		eq->unknown070 = 175; // This needs to be set to something, else we get a 1HS animation instead of ranged.

		OUT(item_type);
		OUT(skill);

		strncpy(eq->model_name, emu->model_name, sizeof(eq->model_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_SpawnAppearance)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *emu_buffer = in->pBuffer;

		SpawnAppearance_Struct *sas = (SpawnAppearance_Struct *)emu_buffer;

		if (sas->type != AT_Size)
		{
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_ChangeSize, sizeof(ChangeSize_Struct));

		ChangeSize_Struct *css = (ChangeSize_Struct *)outapp->pBuffer;

		css->EntityID = sas->spawn_id;
		css->Size = (float)sas->parameter;
		css->Unknown08 = 0;
		css->Unknown12 = 1.0f;

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_SpawnDoor)
	{
		SETUP_VAR_ENCODE(Door_Struct);
		int door_count = __packet->size / sizeof(Door_Struct);
		int total_length = door_count * sizeof(structs::Door_Struct);
		ALLOC_VAR_ENCODE(structs::Door_Struct, total_length);

		int r;
		for (r = 0; r < door_count; r++) {
			strncpy(eq[r].name, emu[r].name, sizeof(eq[r].name));
			eq[r].xPos = emu[r].xPos;
			eq[r].yPos = emu[r].yPos;
			eq[r].zPos = emu[r].zPos;
			eq[r].heading = emu[r].heading;
			eq[r].incline = emu[r].incline;
			eq[r].size = emu[r].size;
			eq[r].doorId = emu[r].doorId;
			eq[r].opentype = emu[r].opentype;
			eq[r].state_at_spawn = emu[r].state_at_spawn;
			eq[r].invert_state = emu[r].invert_state;
			eq[r].door_param = emu[r].door_param;
			eq[r].unknown0080 = 0;
			eq[r].unknown0081 = 1; // Both must be 1 to allow clicking doors
			eq[r].unknown0082 = 0;
			eq[r].unknown0083 = 1; // Both must be 1 to allow clicking doors
			eq[r].unknown0084 = 0;
			eq[r].unknown0085 = 0;
			eq[r].unknown0086 = 0;
		}

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

		ServerToTDSTextLink(new_message, old_message);

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

	ENCODE(OP_Stun)
	{
		ENCODE_LENGTH_EXACT(Stun_Struct);
		SETUP_DIRECT_ENCODE(Stun_Struct, structs::Stun_Struct);

		OUT(duration);
		eq->unknown005 = 163;
		eq->unknown006 = 67;

		FINISH_ENCODE();
	}

	ENCODE(OP_TargetBuffs) { ENCODE_FORWARD(OP_BuffCreate); }

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

		TaskDescriptionData1_Struct *emu_tdd1 = (TaskDescriptionData1_Struct *)InBuffer;
		emu_tdd1->StartTime = (time(nullptr) - emu_tdd1->StartTime); // RoF2 has elapsed time here rather than start time

		InBuffer += sizeof(TaskDescriptionData1_Struct);
		uint32 description_size = strlen(InBuffer) + 1;
		InBuffer += description_size;
		InBuffer += sizeof(TaskDescriptionData2_Struct);

		std::string old_message = InBuffer; // start 'Reward' as string
		std::string new_message;
		ServerToTDSTextLink(new_message, old_message);

		in->size = sizeof(TaskDescriptionHeader_Struct) + sizeof(TaskDescriptionData1_Struct)+
			sizeof(TaskDescriptionData2_Struct) + sizeof(TaskDescriptionTrailer_Struct)+
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

#if 0 // original code
		EQApplicationPacket *in = *p;
		*p = nullptr;

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_TaskDescription, in->size + 1);
		// Set the Write pointer as we don't know what has been done with the packet before we get it.
		in->SetReadPosition(0);
		// Copy the header
		for (int i = 0; i < 5; ++i)
			outapp->WriteUInt32(in->ReadUInt32());

		// Copy Title
		while (uint8 c = in->ReadUInt8())
			outapp->WriteUInt8(c);
		outapp->WriteUInt8(0);

		outapp->WriteUInt32(in->ReadUInt32());	// Duration
		outapp->WriteUInt32(in->ReadUInt32());	// Unknown
		uint32 StartTime = in->ReadUInt32();
		outapp->WriteUInt32(time(nullptr) - StartTime);	// RoF2 has elapsed time here rather than starttime

		// Copy the rest of the packet verbatim
		uint32 BytesLeftToCopy = in->size - in->GetReadPosition();
		memcpy(outapp->pBuffer + outapp->GetWritePosition(), in->pBuffer + in->GetReadPosition(), BytesLeftToCopy);

		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
#endif
	}

	ENCODE(OP_TaskHistoryReply)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		// First we need to calculate the length of the new packet
		in->SetReadPosition(4);
		uint32 ActivityCount = in->ReadUInt32();

		uint32 Text1Length = 0;
		uint32 Text2Length = 0;
		uint32 Text3Length = 0;

		uint32 OutboundPacketSize = 8;

		for (uint32 i = 0; i < ActivityCount; ++i)
		{
			Text1Length = 0;
			Text2Length = 0;
			Text3Length = 0;

			in->ReadUInt32(); // Activity type

			// Skip past Text1
			while (in->ReadUInt8())
				++Text1Length;

			// Skip past Text2
			while (in->ReadUInt8())
				++Text2Length;

			in->ReadUInt32();
			in->ReadUInt32();
			in->ReadUInt32();
			uint32 ZoneID = in->ReadUInt32();
			in->ReadUInt32();

			// Skip past Text3
			while (in->ReadUInt8())
				++Text3Length;

			char ZoneNumber[10];

			sprintf(ZoneNumber, "%i", ZoneID);

			OutboundPacketSize += (24 + Text1Length + 1 + Text2Length + Text3Length + 1 + 7 + (strlen(ZoneNumber) * 2));
		}

		in->SetReadPosition(0);

		auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, OutboundPacketSize);

		outapp->WriteUInt32(in->ReadUInt32());	// Task index
		outapp->WriteUInt32(in->ReadUInt32());	// Activity count

		for (uint32 i = 0; i < ActivityCount; ++i)
		{
			Text1Length = 0;
			Text2Length = 0;
			Text3Length = 0;

			outapp->WriteUInt32(in->ReadUInt32()); // ActivityType

			// Copy Text1
			while (uint8 c = in->ReadUInt8())
				outapp->WriteUInt8(c);

			outapp->WriteUInt8(0);	// Text1 has a null terminator

			uint32 CurrentPosition = in->GetReadPosition();

			// Determine Length of Text2
			while (in->ReadUInt8())
				++Text2Length;

			outapp->WriteUInt32(Text2Length);

			in->SetReadPosition(CurrentPosition);

			// Copy Text2
			while (uint8 c = in->ReadUInt8())
				outapp->WriteUInt8(c);

			outapp->WriteUInt32(in->ReadUInt32()); // Goalcount
			in->ReadUInt32();
			in->ReadUInt32();
			uint32 ZoneID = in->ReadUInt32();
			in->ReadUInt32();

			char ZoneNumber[10];

			sprintf(ZoneNumber, "%i", ZoneID);

			outapp->WriteUInt32(2);
			outapp->WriteUInt8(0x2d); // "-"
			outapp->WriteUInt8(0x31); // "1"

			outapp->WriteUInt32(2);
			outapp->WriteUInt8(0x2d); // "-"
			outapp->WriteUInt8(0x31); // "1"
			outapp->WriteString(ZoneNumber);

			outapp->WriteUInt32(0);

			// Copy Tex3t
			while (uint8 c = in->ReadUInt8())
				outapp->WriteUInt8(c);

			outapp->WriteUInt8(0);	// Text3 has a null terminator

			outapp->WriteUInt8(0x31); // "1"
			outapp->WriteString(ZoneNumber);
		}

		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
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
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Track_Struct));
			delete in;
			return;
		}

		int PacketSize = 2;

		for (int i = 0; i < EntryCount; ++i, ++emu)
			PacketSize += (13 + strlen(emu->name));

		emu = (Track_Struct *)__emu_buffer;

		in->size = PacketSize;
		in->pBuffer = new unsigned char[in->size];

		char *Buffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint16, Buffer, EntryCount);

		for (int i = 0; i < EntryCount; ++i, ++emu)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->entityid);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->distance);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_npc);
			VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_pet);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_merc);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Trader)
	{
		if ((*p)->size == sizeof(ClickTrader_Struct))
		{
			ENCODE_LENGTH_EXACT(ClickTrader_Struct);
			SETUP_DIRECT_ENCODE(ClickTrader_Struct, structs::ClickTrader_Struct);

			eq->Code = emu->Code;
			// Live actually has 200 items now, but 80 is the most our internal struct supports
			for (uint32 i = 0; i < 200; i++)
			{
				eq->items[i].Unknown18 = 0;
				if (i < 80) {
					snprintf(eq->items[i].SerialNumber, sizeof(eq->items[i].SerialNumber), "%016d", emu->SerialNumber[i]);
					eq->ItemCost[i] = emu->ItemCost[i];
				}
				else {
					snprintf(eq->items[i].SerialNumber, sizeof(eq->items[i].SerialNumber), "%016d", 0);
					eq->ItemCost[i] = 0;
				}
			}

			FINISH_ENCODE();
		}
		else if ((*p)->size == sizeof(Trader_ShowItems_Struct))
		{
			ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
			SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);

			eq->Code = emu->Code;
			//strncpy(eq->SerialNumber, "0000000000000000", sizeof(eq->SerialNumber));
			//snprintf(eq->SerialNumber, sizeof(eq->SerialNumber), "%016d", 0);
			eq->TraderID = emu->TraderID;
			//eq->Stacksize = 0;
			//eq->Price = 0;

			FINISH_ENCODE();
		}
		else if ((*p)->size == sizeof(TraderStatus_Struct))
		{
			ENCODE_LENGTH_EXACT(TraderStatus_Struct);
			SETUP_DIRECT_ENCODE(TraderStatus_Struct, structs::TraderStatus_Struct);

			eq->Code = emu->Code;

			FINISH_ENCODE();
		}
		else if ((*p)->size == sizeof(TraderBuy_Struct))
		{
			ENCODE_FORWARD(OP_TraderBuy);
		}
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

	ENCODE(OP_TraderDelItem)
	{
		ENCODE_LENGTH_EXACT(TraderDelItem_Struct);
		SETUP_DIRECT_ENCODE(TraderDelItem_Struct, structs::TraderDelItem_Struct);

		OUT(TraderID);
		snprintf(eq->SerialNumber, sizeof(eq->SerialNumber), "%016d", emu->ItemID);
		LogTrading("ENCODE(OP_TraderDelItem): TraderID [{}], SerialNumber: [{}]", emu->TraderID, emu->ItemID);

		FINISH_ENCODE();
	}

	ENCODE(OP_TraderShop)
	{
		uint32 psize = (*p)->size;
		if (psize == sizeof(TraderClick_Struct))
		{
			ENCODE_LENGTH_EXACT(TraderClick_Struct);
			SETUP_DIRECT_ENCODE(TraderClick_Struct, structs::TraderClick_Struct);

			eq->Code = 28; // Seen on Live
			OUT(TraderID);
			OUT(Approval);

			FINISH_ENCODE();
		}
		else if (psize == sizeof(BazaarWelcome_Struct))
		{
			ENCODE_LENGTH_EXACT(BazaarWelcome_Struct);
			SETUP_DIRECT_ENCODE(BazaarWelcome_Struct, structs::BazaarWelcome_Struct);

			eq->Code = emu->Beginning.Action;
			eq->EntityID = emu->Unknown012;
			OUT(Traders);
			OUT(Items);
			eq->Traders2 = emu->Traders;
			eq->Items2 = emu->Items;

			LogTrading("ENCODE(OP_TraderShop): BazaarWelcome_Struct Code [{}], Traders [{}], Items [{}]",
				eq->Code, eq->Traders, eq->Items);

			FINISH_ENCODE();
		}
		else if (psize == sizeof(TraderBuy_Struct))
		{
			ENCODE_LENGTH_EXACT(TraderBuy_Struct);
			SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);

			OUT(Action);
			OUT(TraderID);

			//memcpy(eq->BuyerName, emu->BuyerName, sizeof(eq->BuyerName));
			//memcpy(eq->SellerName, emu->SellerName, sizeof(eq->SellerName));

			memcpy(eq->ItemName, emu->ItemName, sizeof(eq->ItemName));
			OUT(ItemID);
			OUT(AlreadySold);
			OUT(Price);
			OUT(Quantity);
			snprintf(eq->SerialNumber, sizeof(eq->SerialNumber), "%016d", emu->ItemID);

			LogTrading("ENCODE(OP_TraderShop): Buy Action [{}], Price [{}], Trader [{}], ItemID [{}], Quantity [{}], ItemName, [{}]",
				eq->Action, eq->Price, eq->TraderID, eq->ItemID, eq->Quantity, emu->ItemName);

			FINISH_ENCODE();
		}
		else
		{
			LogTrading("ENCODE(OP_TraderShop): Encode Size Unknown ([{}])", psize);
		}
	}

	ENCODE(OP_TributeInfo)
	{
		ENCODE_LENGTH_ATLEAST(TributeAbility_Struct);
		SETUP_VAR_ENCODE(TributeAbility_Struct);
		ALLOC_VAR_ENCODE(structs::TributeAbility_Struct, sizeof(structs::TributeAbility_Struct) + strlen(emu->name) + 1);

		OUT(tribute_id);
		OUT(tier_count);

		for (uint32 i = 0; i < MAX_TRIBUTE_TIERS; ++i)
		{
			eq->tiers[i].level = emu->tiers[i].level;
			eq->tiers[i].tribute_item_id = emu->tiers[i].tribute_item_id;
			eq->tiers[i].cost = emu->tiers[i].cost;
		}

		eq->unknown128 = 0;

		strcpy(eq->name, emu->name);

		FINISH_ENCODE();
	}

	ENCODE(OP_TributeItem)
	{
		ENCODE_LENGTH_EXACT(TributeItem_Struct);
		SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

		eq->slot = ServerToTDSSlot(emu->slot);
		OUT(quantity);
		OUT(tribute_master_id);
		OUT(tribute_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_VetClaimReply)
	{
		ENCODE_LENGTH_EXACT(VeteranClaim);
		SETUP_DIRECT_ENCODE(VeteranClaim, structs::VeteranClaim);

		memcpy(eq->name, emu->name, sizeof(emu->name));
		OUT(claim_id);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_VetRewardsAvaliable)
	{
		EQApplicationPacket *inapp = *p;
		auto __emu_buffer = inapp->pBuffer;

		uint32 count = ((*p)->Size() / sizeof(InternalVeteranReward));

		// calculate size of names, note the packet DOES NOT have null termed c-strings
		std::vector<uint32> name_lengths;
		for (int i = 0; i < count; ++i) {
			InternalVeteranReward *ivr = (InternalVeteranReward *)__emu_buffer;

			for (int i = 0; i < ivr->claim_count; i++) {
				uint32 length = strnlen(ivr->items[i].item_name, 63);
				if (length)
					name_lengths.push_back(length);
			}

			__emu_buffer += sizeof(InternalVeteranReward);
		}

		uint32 packet_size = std::accumulate(name_lengths.begin(), name_lengths.end(), 0) +
				     sizeof(structs::VeteranReward) + (sizeof(structs::VeteranRewardEntry) * count) +
				     // size of name_lengths is the same as item count
				     (sizeof(structs::VeteranRewardItem) * name_lengths.size());

		// build packet now!
		auto outapp = new EQApplicationPacket(OP_VetRewardsAvaliable, packet_size);
		__emu_buffer = inapp->pBuffer;

		outapp->WriteUInt32(count);
		auto name_itr = name_lengths.begin();
		for (int i = 0; i < count; i++) {
			InternalVeteranReward *ivr = (InternalVeteranReward *)__emu_buffer;

			outapp->WriteUInt32(ivr->claim_id);
			outapp->WriteUInt32(ivr->number_available);
			outapp->WriteUInt32(ivr->claim_count);
			outapp->WriteUInt8(1); // enabled

			for (int j = 0; j < ivr->claim_count; j++) {
				assert(name_itr != name_lengths.end()); // the way it's written, it should never happen, so just assert
				outapp->WriteUInt32(*name_itr);
				outapp->WriteData(ivr->items[j].item_name, *name_itr);
				outapp->WriteUInt32(ivr->items[j].item_id);
				outapp->WriteUInt32(ivr->items[j].charges);
				++name_itr;
			}

			__emu_buffer += sizeof(InternalVeteranReward);
		}

		dest->FastQueuePacket(&outapp);
		delete inapp;
	}

	ENCODE(OP_WearChange)
	{
		ENCODE_LENGTH_EXACT(WearChange_Struct);
		SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);

		OUT(spawn_id);
		OUT(material);
		OUT(unknown06);
		OUT(elite_material);
		OUT(hero_forge_model);
		OUT(unknown18);
		OUT(color.Color);
		OUT(wear_slot_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_WhoAllResponse)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		char *InBuffer = (char *)in->pBuffer;

		WhoAllReturnStruct *wars = (WhoAllReturnStruct*)InBuffer;

		int Count = wars->playercount;

		auto outapp = new EQApplicationPacket(OP_WhoAllResponse, in->size + (Count * 4));

		char *OutBuffer = (char *)outapp->pBuffer;

		// The struct fields were moved around a bit, so adjust values before copying
		wars->unknown44[0] = Count;
		wars->unknown52 = 0;

		memcpy(OutBuffer, InBuffer, sizeof(WhoAllReturnStruct));

		OutBuffer += sizeof(WhoAllReturnStruct);
		InBuffer += sizeof(WhoAllReturnStruct);

		for (int i = 0; i < Count; ++i)
		{
			uint32 x;

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);

			InBuffer += 4;
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0xffffffff);

			char Name[64];

			VARSTRUCT_DECODE_STRING(Name, InBuffer);	// Char Name
			VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);

			VARSTRUCT_DECODE_STRING(Name, InBuffer);	// Guild Name
			VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

			for (int j = 0; j < 7; ++j)
			{
				x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
				VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);
			}

			VARSTRUCT_DECODE_STRING(Name, InBuffer);		// Account
			VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);
		}

		//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
		dest->FastQueuePacket(&outapp);
		delete in;
	}

	ENCODE(OP_ZoneChange)
	{
		ENCODE_LENGTH_EXACT(ZoneChange_Struct);
		SETUP_DIRECT_ENCODE(ZoneChange_Struct, structs::ZoneChange_Struct);

		memcpy(eq->char_name, emu->char_name, sizeof(emu->char_name));
		OUT(zoneID);
		OUT(instanceID);
		OUT(y);
		OUT(x);
		OUT(z)
		OUT(zone_reason);
		OUT(success);

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneEntry) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_ZonePlayerToBind)
	{
		SETUP_VAR_ENCODE(ZonePlayerToBind_Struct);
		ALLOC_LEN_ENCODE(sizeof(structs::ZonePlayerToBind_Struct) + strlen(emu->zone_name));

		__packet->SetWritePosition(0);
		__packet->WriteUInt16(emu->bind_zone_id);
		__packet->WriteUInt16(emu->bind_instance_id);
		__packet->WriteFloat(emu->x);
		__packet->WriteFloat(emu->y);
		__packet->WriteFloat(emu->z);
		__packet->WriteFloat(emu->heading);
		__packet->WriteString(emu->zone_name);
		__packet->WriteUInt8(1); // save items
		__packet->WriteUInt32(0); // hp
		__packet->WriteUInt32(0); // mana
		__packet->WriteUInt32(0); // endurance

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneServerInfo)
	{
		SETUP_DIRECT_ENCODE(ZoneServerInfo_Struct, ZoneServerInfo_Struct);

		OUT_str(ip);
		OUT(port);

		FINISH_ENCODE();
	}

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
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
			delete in;
			return;
		}

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[STRUCTS] Spawn name is [%s]", emu->name);

		emu = (Spawn_Struct *)__emu_buffer;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[STRUCTS] Spawn packet size is %i, entries = %i", in->size, entrycount);

		char *Buffer = (char *)in->pBuffer, *BufferStart;

		int r;
		int k;
		for (r = 0; r < entrycount; r++, emu++) {

			int PacketSize = 217; // was 206

			PacketSize += strlen(emu->name);
			PacketSize += strlen(emu->lastName);

			emu->title[31] = 0;
			emu->suffix[31] = 0;

			if (strlen(emu->title))
				PacketSize += strlen(emu->title) + 1;

			if (strlen(emu->suffix))
				PacketSize += strlen(emu->suffix) + 1;

			bool ShowName = 1;
			if (emu->bodytype >= 66)
			{
				emu->race = 127;
				emu->bodytype = 11;
				emu->gender = 0;
				ShowName = 0;
			}

			float SpawnSize = emu->size;
			if (!((emu->NPC == 0) || (emu->race <= 12) || (emu->race == 128) || (emu->race == 130) || (emu->race == 330) || (emu->race == 522)))
			{
				PacketSize += 60;

				if (emu->size == 0)
				{
					emu->size = 6;
					SpawnSize = 6;
				}
			}
			else
				PacketSize += 216;

			if (emu->NPC)
				PacketSize += 12;

			if (SpawnSize == 0)
			{
				SpawnSize = 3;
			}

			auto outapp = new EQApplicationPacket(OP_ZoneEntry, PacketSize);
			Buffer = (char *)outapp->pBuffer;
			BufferStart = Buffer;
			VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->spawnId);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, SpawnSize - 0.7);	// Eye Height?
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC);

			structs::Spawn_Struct_Bitfields *Bitfields = (structs::Spawn_Struct_Bitfields*)Buffer;

			Bitfields->gender = emu->gender;
			Bitfields->ispet = emu->is_pet;
			Bitfields->afk = emu->afk;
			Bitfields->anon = emu->anon;
			Bitfields->gm = emu->gm;
			Bitfields->sneak = 0;
			Bitfields->lfg = emu->lfg;
			Bitfields->invis = emu->invis;
			Bitfields->linkdead = 0;
			Bitfields->showhelm = emu->showhelm;
			Bitfields->trader = 0;
			Bitfields->targetable = 1;
			Bitfields->targetable_with_hotkey = emu->targetable_with_hotkey ? 1 : 0;
			Bitfields->showname = ShowName;

			// Not currently found
			// Bitfields->statue = 0;
			// Bitfields->buyer = 0;

			Buffer += sizeof(structs::Spawn_Struct_Bitfields);

			uint8 OtherData = 0;

			if (strlen(emu->title))
				OtherData = OtherData | 16;

			if (strlen(emu->suffix))
				OtherData = OtherData | 32;

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, OtherData);

			VARSTRUCT_ENCODE_TYPE(float, Buffer, -1);	// unknown3
			VARSTRUCT_ENCODE_TYPE(float, Buffer, 0);	// unknown4

			// Setting this next field to zero will cause a crash. Looking at ShowEQ, if it is zero, the bodytype field is not
			// present. Will sort that out later.
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 1);	// This is a properties count field
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->bodytype);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->curHp);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->haircolor);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->beardcolor);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->eyecolor1);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->eyecolor2);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->hairstyle);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->beard);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_heritage);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_tattoo);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_details);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->equip_chest2);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown9
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown10
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->helm); // unknown11

			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->size);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->face);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->walkspeed);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->runspeed);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->race);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// ShowEQ calls this 'Holding'
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->deity);
			if (emu->NPC)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xFFFFFFFF);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x00000000);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildID);

				/* Translate older ranks to new values */
				switch (emu->guildrank) {
				case 0: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 5);  break; }  // GUILD_MEMBER	0
				case 1: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 3);  break; }  // GUILD_OFFICER	1
				case 2: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);  break; }  // GUILD_LEADER	2
				default: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildrank); break; }  //
				}
			}

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->class_);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// pvp
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->StandState);	// standstate
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->light);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->flymode);

			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 100); // LiveUnknown1 12/06/14 (possibly mana percent?)
			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 10); // LiveUnknown2 12/06/14
			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // LiveUnknown3 12/06/14

			VARSTRUCT_ENCODE_STRING(Buffer, emu->lastName);

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// aatitle ??
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC ? 0 : 1); // unknown - Must be 1 for guild name to be shown abover players head.
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->petOwnerId);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown13
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->PlayerState);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown15
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown16
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown17

			if (emu->NPC) {
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			}

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // unknown18
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // unknown19

			if ((emu->NPC == 0) || (emu->race <= 12) || (emu->race == 128) || (emu->race == 130) || (emu->race == 330) || (emu->race == 522))
			{			
				for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; ++k)
				{
					{
						VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment_tint.Slot[k].Color);
					}
				}

				structs::Texture_Struct *Equipment = (structs::Texture_Struct *)Buffer;

				for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
					Equipment[k].Material = emu->equipment.Slot[k].Material;
					Equipment[k].Unknown1 = emu->equipment.Slot[k].Unknown1;
					Equipment[k].EliteMaterial = emu->equipment.Slot[k].EliteModel;
					Equipment[k].HeroForgeModel = emu->equipment.Slot[k].HerosForgeModel;
					Equipment[k].Material2 = emu->equipment.Slot[k].Unknown2;
				}

				Buffer += (sizeof(structs::Texture_Struct) * EQ::textures::materialCount);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment.Primary.Material);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment.Secondary.Material);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			}

			structs::Spawn_Struct_Position *Position = (structs::Spawn_Struct_Position*)Buffer;

			Position->y = emu->y;
			Position->deltaZ = emu->deltaZ;
			Position->deltaX = emu->deltaX;
			Position->x = emu->x;
			Position->heading = emu->heading;
			Position->deltaHeading = emu->deltaHeading;
			Position->z = emu->z;
			Position->animation = emu->animation;
			Position->deltaY = emu->deltaY;

			Buffer += sizeof(structs::Spawn_Struct_Position);

			if (strlen(emu->title))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->title);
			}

			if (strlen(emu->suffix))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->suffix);
			}

			Buffer += 8;
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->IsMercenary);
			VARSTRUCT_ENCODE_STRING(Buffer, "0000000000000000");
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff);
			// 37 zero bytes follow
			Buffer += 37; // was 29
			if (Buffer != (BufferStart + PacketSize))
			{
				LogNetcode("[ERROR] SPAWN ENCODE LOGIC PROBLEM: Buffer pointer is now [{}] from end", Buffer - (BufferStart + PacketSize));
			}
			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Sending zone spawn for %s packet is %i bytes", emu->name, outapp->size);
			//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp, ack_req);
		}

		delete in;
	}

// DECODE methods

	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = TDSToServerMainInvSlot(eq->slot);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySell)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySellItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = TDSToServerMainInvSlot(eq->slot_id);
		IN(charges);
		IN(cost);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySellSelection)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySelectItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySelectItem_Struct, structs::AltCurrencySelectItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = TDSToServerMainInvSlot(eq->slot_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Animation)
	{
		DECODE_LENGTH_EXACT(structs::Animation_Struct);
		SETUP_DIRECT_DECODE(Animation_Struct, structs::Animation_Struct);

		IN(spawnid);
		IN(action);
		IN(speed);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApplyPoison)
	{
		DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
		SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		emu->inventorySlot = TDSToServerMainInvSlot(eq->inventorySlot);
		IN(success);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AugmentInfo)
	{
		DECODE_LENGTH_EXACT(structs::AugmentInfo_Struct);
		SETUP_DIRECT_DECODE(AugmentInfo_Struct, structs::AugmentInfo_Struct);

		IN(itemid);
		IN(window);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AugmentItem)
	{
		DECODE_LENGTH_EXACT(structs::AugmentItem_Struct);
		SETUP_DIRECT_DECODE(AugmentItem_Struct, structs::AugmentItem_Struct);

		emu->container_slot = TDSToServerSlot(eq->container_slot);
		emu->augment_slot = TDSToServerSlot(eq->augment_slot);
		emu->container_index = eq->container_index;
		emu->augment_index = eq->augment_index;
		emu->dest_inst_id = eq->dest_inst_id;
		emu->augment_action = eq->augment_action;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BazaarSearch)
	{
		char *Buffer = (char *)__packet->pBuffer;

		uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);

		if ((SubAction != BazaarInspectItem) || (__packet->size != sizeof(structs::NewBazaarInspect_Struct)))
			return;

		SETUP_DIRECT_DECODE(NewBazaarInspect_Struct, structs::NewBazaarInspect_Struct);

		IN(Beginning.Action);
		memcpy(emu->Name, eq->Name, sizeof(emu->Name));
		IN(SerialNumber);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BlockedBuffs)
	{
		DECODE_LENGTH_EXACT(structs::BlockedBuffs_Struct);
		SETUP_DIRECT_DECODE(BlockedBuffs_Struct, structs::BlockedBuffs_Struct);

		for (uint32 i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			emu->SpellID[i] = eq->SpellID[i];

		IN(Count);
		IN(Pet);
		IN(Initialise);
		IN(Flags);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Buff)
	{
		DECODE_LENGTH_EXACT(structs::SpellBuffFade_Struct_Live);
		SETUP_DIRECT_DECODE(structs::SpellBuffFade_Struct_Live, structs::SpellBuffFade_Struct_Live);

		IN(entityid);
		//IN(slot);
		IN(level);
		IN(effect);
		IN(spellid);
		IN(duration);
		IN(slotid);
		IN(bufffade);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BuffRemoveRequest)
	{
		// This is to cater for the fact that short buff box buffs start at 30 as opposed to 25 in prior clients.
		//
		DECODE_LENGTH_EXACT(structs::BuffRemoveRequest_Struct);
		SETUP_DIRECT_DECODE(BuffRemoveRequest_Struct, structs::BuffRemoveRequest_Struct);

		emu->SlotID = (eq->SlotID < 42) ? eq->SlotID : (eq->SlotID - 17);

		IN(EntityID);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_CastSpell)
	{
		DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
		SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

		if (eq->slot == 13)
			emu->slot = 10;
		else
			IN(slot);

		IN(spell_id);
		emu->inventoryslot = TDSToServerSlot(eq->inventoryslot);
		//IN(inventoryslot);
		IN(target_id);
		IN(y_pos);
		IN(x_pos);
		IN(z_pos);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		char *InBuffer = (char *)__eq_buffer;

		char Sender[64];
		char Target[64];

		VARSTRUCT_DECODE_STRING(Sender, InBuffer);
		VARSTRUCT_DECODE_STRING(Target, InBuffer);

		InBuffer += 4;

		uint32 Language = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
		uint32 Channel = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

		InBuffer += 5;

		uint32 Skill = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

		std::string old_message = InBuffer;
		std::string new_message;
		TDSToServerTextLink(new_message, old_message);

		if (new_message[0] == '.')
			new_message.replace(0, 1, "#");

		//__packet->size = sizeof(ChannelMessage_Struct)+strlen(InBuffer) + 1;
		__packet->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;

		__packet->pBuffer = new unsigned char[__packet->size];
		ChannelMessage_Struct *emu = (ChannelMessage_Struct *)__packet->pBuffer;

		strn0cpy(emu->targetname, Target, sizeof(emu->targetname));
		strn0cpy(emu->sender, Target, sizeof(emu->sender));
		emu->language = Language;
		emu->chan_num = Channel;
		emu->skill_in_language = Skill;
		strcpy(emu->message, new_message.c_str());

		delete[] __eq_buffer;
	}

	DECODE(OP_CharacterCreate)
	{
		DECODE_LENGTH_EXACT(structs::CharCreate_Struct);
		SETUP_DIRECT_DECODE(CharCreate_Struct, structs::CharCreate_Struct);

		IN(gender);
		IN(race);
		IN(class_);
		IN(deity);
		IN(start_zone);
		IN(haircolor);
		IN(beard);
		IN(beardcolor);
		IN(hairstyle);
		IN(face);
		IN(eyecolor1);
		IN(eyecolor2);
		IN(drakkin_heritage);
		IN(drakkin_tattoo);
		IN(drakkin_details);
		IN(STR);
		IN(STA);
		IN(AGI);
		IN(DEX);
		IN(WIS);
		IN(INT);
		IN(CHA);
		IN(tutorial);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ClientUpdate)
	{
		// for some odd reason, there is an extra byte on the end of this on occasion..
		DECODE_LENGTH_ATLEAST(structs::PlayerPositionUpdateClient_Struct);
		SETUP_DIRECT_DECODE(PlayerPositionUpdateClient_Struct, structs::PlayerPositionUpdateClient_Struct);

		IN(spawn_id);
		IN(sequence);
		IN(x_pos);
		IN(y_pos);
		IN(z_pos);
		IN(heading);
		IN(delta_x);
		IN(delta_y);
		IN(delta_z);
		IN(delta_heading);
		IN(animation);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Consider)
	{
		DECODE_LENGTH_EXACT(structs::Consider_Struct);
		SETUP_DIRECT_DECODE(Consider_Struct, structs::Consider_Struct);

		IN(playerid);
		IN(targetid);
		IN(faction);
		IN(level);
		//emu->cur_hp = 1;
		//emu->max_hp = 2;
		//emu->pvpcon = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ConsiderCorpse) { DECODE_FORWARD(OP_Consider); }

	DECODE(OP_Consume)
	{
		DECODE_LENGTH_EXACT(structs::Consume_Struct);
		SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);

		emu->slot = TDSToServerSlot(eq->slot);
		IN(auto_consumed);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Damage)
	{
		DECODE_LENGTH_EXACT(structs::CombatDamage_Struct);
		SETUP_DIRECT_DECODE(CombatDamage_Struct, structs::CombatDamage_Struct);

		IN(target);
		IN(source);
		IN(type);
		IN(spellid);
		IN(damage);
		//IN(meleepush_xy);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DeleteItem)
	{
		DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
		SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		emu->from_slot = TDSToServerSlot(eq->from_slot);
		emu->to_slot = TDSToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzAddPlayer)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommand_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommand_Struct, structs::ExpeditionCommand_Struct);

		strn0cpy(emu->name, eq->name, sizeof(emu->name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzChooseZoneReply)
	{
		DECODE_LENGTH_EXACT(structs::DynamicZoneChooseZoneReply_Struct);
		SETUP_DIRECT_DECODE(DynamicZoneChooseZoneReply_Struct, structs::DynamicZoneChooseZoneReply_Struct);

		IN(unknown000);
		IN(unknown004);
		IN(unknown008);
		IN(unknown_id1);
		IN(dz_zone_id);
		IN(dz_instance_id);
		IN(dz_type);
		IN(unknown_id2);
		IN(unknown028);
		IN(unknown032);
		IN(unknown036);
		IN(unknown040);
		IN(unknown044);
		IN(unknown048);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzExpeditionInviteResponse)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionInviteResponse_Struct);
		SETUP_DIRECT_DECODE(ExpeditionInviteResponse_Struct, structs::ExpeditionInviteResponse_Struct);

		IN(dz_zone_id);
		IN(dz_instance_id);
		IN(accepted);
		IN(swapping);
		strn0cpy(emu->swap_name, eq->swap_name, sizeof(emu->swap_name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzMakeLeader)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommand_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommand_Struct, structs::ExpeditionCommand_Struct);

		strn0cpy(emu->name, eq->name, sizeof(emu->name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzRemovePlayer)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommand_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommand_Struct, structs::ExpeditionCommand_Struct);

		strn0cpy(emu->name, eq->name, sizeof(emu->name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzSwapPlayer)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommandSwap_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommandSwap_Struct, structs::ExpeditionCommandSwap_Struct);

		strn0cpy(emu->add_player_name, eq->add_player_name, sizeof(emu->add_player_name));
		strn0cpy(emu->rem_player_name, eq->rem_player_name, sizeof(emu->rem_player_name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Emote)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		std::string old_message = (char *)&__eq_buffer[4]; // unknown01 offset
		std::string new_message;
		TDSToServerTextLink(new_message, old_message);

		__packet->size = sizeof(Emote_Struct);
		__packet->pBuffer = new unsigned char[__packet->size];

		char *InBuffer = (char *)__packet->pBuffer;

		memcpy(InBuffer, __eq_buffer, 4);
		InBuffer += 4;
		strcpy(InBuffer, new_message.substr(0, 1023).c_str());
		InBuffer[1023] = '\0';

		delete[] __eq_buffer;
	}

	DECODE(OP_EnvDamage)
	{
		DECODE_LENGTH_EXACT(structs::EnvDamage2_Struct);
		SETUP_DIRECT_DECODE(EnvDamage2_Struct, structs::EnvDamage2_Struct);

		IN(id);
		IN(damage);
		IN(dmgtype);
		emu->constant = 0xFFFF;

		FINISH_DIRECT_DECODE();
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
		IN(drakkin_heritage);
		IN(drakkin_tattoo);
		IN(drakkin_details);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_FindPersonRequest)
	{
		DECODE_LENGTH_EXACT(structs::FindPersonRequest_Struct);
		SETUP_DIRECT_DECODE(FindPersonRequest_Struct, structs::FindPersonRequest_Struct);

		IN(npc_id);
		IN(client_pos.x);
		IN(client_pos.y);
		IN(client_pos.z);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GMLastName)
	{
		DECODE_LENGTH_EXACT(structs::GMLastName_Struct);
		SETUP_DIRECT_DECODE(GMLastName_Struct, structs::GMLastName_Struct);

		memcpy(emu->name, eq->name, sizeof(emu->name));
		memcpy(emu->gmname, eq->gmname, sizeof(emu->gmname));
		memcpy(emu->lastname, eq->lastname, sizeof(emu->lastname));
		for (int i = 0; i<4; i++)
		{
			emu->unknown[i] = eq->unknown[i];
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupCancelInvite)
	{
		DECODE_LENGTH_EXACT(structs::GroupCancel_Struct);
		SETUP_DIRECT_DECODE(GroupCancel_Struct, structs::GroupCancel_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));
		IN(toggle);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupDisband)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_Disband");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupGeneric_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupGeneric_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupFollow");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow2)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupFollow2");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupInvite)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupInvite");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupInvite_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupInvite_Struct);

		memcpy(emu->name1, eq->invitee_name, sizeof(emu->name1));
		memcpy(emu->name2, eq->inviter_name, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupInvite2)
	{
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupInvite2. Forwarding");
		DECODE_FORWARD(OP_GroupInvite);
	}

	DECODE(OP_GuildDemote)
	{
		DECODE_LENGTH_EXACT(structs::GuildDemoteStruct);
		SETUP_DIRECT_DECODE(GuildDemoteStruct, structs::GuildDemoteStruct);

		strn0cpy(emu->target, eq->target, sizeof(emu->target));
		strn0cpy(emu->name, eq->name, sizeof(emu->name));
		// IN(rank);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildRemove)
	{
		DECODE_LENGTH_EXACT(structs::GuildCommand_Struct);
		SETUP_DIRECT_DECODE(GuildCommand_Struct, structs::GuildCommand_Struct);

		strn0cpy(emu->othername, eq->othername, sizeof(emu->othername));
		strn0cpy(emu->myname, eq->myname, sizeof(emu->myname));
		IN(guildeqid);
		IN(officer);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildStatus)
	{
		DECODE_LENGTH_EXACT(structs::GuildStatus_Struct);
		SETUP_DIRECT_DECODE(GuildStatus_Struct, structs::GuildStatus_Struct);

		memcpy(emu->Name, eq->Name, sizeof(emu->Name));

		FINISH_DIRECT_DECODE();
	}

	/*DECODE(OP_InspectAnswer)
	{
	DECODE_LENGTH_EXACT(structs::InspectResponse_Struct);
	SETUP_DIRECT_DECODE(InspectResponse_Struct, structs::InspectResponse_Struct);

	IN(TargetID);
	IN(playerid);

	int r;
	for (r = 0; r < 21; r++) {
	strn0cpy(emu->itemnames[r], eq->itemnames[r], sizeof(emu->itemnames[r]));
	}
	// Swap last 2 slots for Arrow and Power Source
	strn0cpy(emu->itemnames[22], eq->itemnames[21], sizeof(emu->itemnames[22]));
	strn0cpy(emu->itemnames[21], eq->unknown_zero, sizeof(emu->itemnames[21]));
	strn0cpy(emu->unknown_zero, eq->unknown_zero, sizeof(emu->unknown_zero));

	int k;
	for (k = 0; k < 21; k++) {
	IN(itemicons[k]);
	}
	// Swap last 2 slots for Arrow and Power Source
	emu->itemicons[22] = eq->itemicons[21];
	emu->itemicons[21] = eq->unknown_zero2;
	emu->unknown_zero2 = eq->unknown_zero2;
	strn0cpy(emu->text, eq->text, sizeof(emu->text));
	//emu->unknown1772 = 0;

	FINISH_DIRECT_DECODE();
	}*/

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
		for (r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			IN(augments[r]);
		}
		IN(link_hash);
		IN(icon);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemVerifyRequest)
	{
		DECODE_LENGTH_EXACT(structs::ItemVerifyRequest_Struct);
		SETUP_DIRECT_DECODE(ItemVerifyRequest_Struct, structs::ItemVerifyRequest_Struct);

		emu->slot = TDSToServerSlot(eq->slot);
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LoadSpellSet)
	{
		DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
		SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

		for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i)
		{
			if (eq->spell[i] == 0)
				emu->spell[i] = 0xFFFFFFFF;
			else
				emu->spell[i] = eq->spell[i];
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LootItem)
	{
		DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
		SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Moderate, Logs::Netcode, "TDS::DECODE(OP_LootItem)");

		IN(lootee);
		IN(looter);
		emu->slot_id = TDSToServerCorpseSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Moderate, Logs::Netcode, "TDS::DECODE(OP_MoveItem)");
		
		emu->from_slot = TDSToServerSlot(eq->from_slot);
		emu->to_slot = TDSToServerSlot(eq->to_slot);
		IN(number_in_stack);
		
		//LogNetcode("[RoF2] MoveItem Slot from [{}] to [{}], Number [{}]", emu->from_slot, emu->to_slot, emu->number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PetCommands)
	{
		DECODE_LENGTH_EXACT(structs::PetCommand_Struct);
		SETUP_DIRECT_DECODE(PetCommand_Struct, structs::PetCommand_Struct);

		IN(command);
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RaidInvite)
	{
		DECODE_LENGTH_ATLEAST(structs::RaidGeneral_Struct);

		// This is a switch on the RaidGeneral action
		switch (*(uint32 *)__packet->pBuffer) {
			case 35: { // raidMOTD
				// we don't have a nice macro for this
				structs::RaidMOTD_Struct *__eq_buffer = (structs::RaidMOTD_Struct *)__packet->pBuffer;
				__eq_buffer->motd[1023] = '\0';
				size_t motd_size = strlen(__eq_buffer->motd) + 1;
				__packet->size = sizeof(RaidMOTD_Struct) + motd_size;
				__packet->pBuffer = new unsigned char[__packet->size];
				RaidMOTD_Struct *emu = (RaidMOTD_Struct *)__packet->pBuffer;
				structs::RaidMOTD_Struct *eq = (structs::RaidMOTD_Struct *)__eq_buffer;
				strn0cpy(emu->general.player_name, eq->general.player_name, 64);
				strn0cpy(emu->motd, eq->motd, motd_size);
				IN(general.action);
				IN(general.parameter);
				FINISH_DIRECT_DECODE();
				break;
			 }
			case 36: { // raidPlayerNote unhandled
				break;
			}
			default: {
				DECODE_LENGTH_EXACT(structs::RaidGeneral_Struct);
				SETUP_DIRECT_DECODE(RaidGeneral_Struct, structs::RaidGeneral_Struct);
				strn0cpy(emu->leader_name, eq->leader_name, 64);
				strn0cpy(emu->player_name, eq->player_name, 64);
				IN(action);
				IN(parameter);
				FINISH_DIRECT_DECODE();
				break;
			}
		}
	}

	DECODE(OP_ReadBook)
	{
		DECODE_LENGTH_EXACT(structs::BookRequest_Struct);
		SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

		IN(type);
		IN(invslot);
		emu->window = (uint8)eq->window;
		strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RecipeAutoCombine)
	{
		DECODE_LENGTH_EXACT(structs::RecipeAutoCombine_Struct);
		SETUP_DIRECT_DECODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

		IN(object_type);
		IN(some_id);
		emu->unknown1 = TDSToServerSlot(eq->container_slot);
		IN(recipe_id);
		IN(reply_code);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RemoveBlockedBuffs) { DECODE_FORWARD(OP_BlockedBuffs); }

	DECODE(OP_RezzAnswer)
	{
		DECODE_LENGTH_EXACT(structs::Resurrect_Struct);
		SETUP_DIRECT_DECODE(Resurrect_Struct, structs::Resurrect_Struct);

		IN(zone_id);
		IN(instance_id);
		IN(y);
		IN(x);
		IN(z);
		memcpy(emu->your_name, eq->your_name, sizeof(emu->your_name));
		memcpy(emu->rezzer_name, eq->rezzer_name, sizeof(emu->rezzer_name));
		IN(spellid);
		memcpy(emu->corpse_name, eq->corpse_name, sizeof(emu->corpse_name));
		IN(action);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Save)
	{
		DECODE_LENGTH_EXACT(structs::Save_Struct);
		SETUP_DIRECT_DECODE(Save_Struct, structs::Save_Struct);

		memcpy(emu->unknown00, eq->unknown00, sizeof(emu->unknown00));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_SetServerFilter)
	{
		DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
		SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);

		int r;
		for (r = 0; r < 29; r++) {
			// Size 40 in RoF2
			IN(filters[r]);
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerBuy)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Sell_Struct);
		SETUP_DIRECT_DECODE(Merchant_Sell_Struct, structs::Merchant_Sell_Struct);

		IN(npcid);
		IN(playerid);
		IN(itemslot);
		IN(quantity);
		IN(price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerSell)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Struct);
		SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		IN(npcid);
		emu->itemslot = TDSToServerMainInvSlot(eq->itemslot);
		//IN(itemslot);
		IN(quantity);
		IN(price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopRequest)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Click_Struct);
		SETUP_DIRECT_DECODE(Merchant_Click_Struct, structs::Merchant_Click_Struct);

		IN(npcid);
		IN(playerid);
		IN(command);
		IN(rate);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Trader)
	{
		uint32 psize = __packet->size;
		if (psize == sizeof(structs::ClickTrader_Struct))
		{
			DECODE_LENGTH_EXACT(structs::ClickTrader_Struct);
			SETUP_DIRECT_DECODE(ClickTrader_Struct, structs::ClickTrader_Struct);

			emu->Code = eq->Code;
			// Live actually has 200 items now, but 80 is the most our internal struct supports
			for (uint32 i = 0; i < 80; i++)
			{
				emu->SerialNumber[i] = 0;	// eq->SerialNumber[i];
				emu->ItemCost[i] = eq->ItemCost[i];
			}

			FINISH_DIRECT_DECODE();
		}
		else if (psize == sizeof(structs::Trader_ShowItems_Struct))
		{
			DECODE_LENGTH_EXACT(structs::Trader_ShowItems_Struct);
			SETUP_DIRECT_DECODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);

			emu->Code = eq->Code;
			emu->TraderID = eq->TraderID;

			FINISH_DIRECT_DECODE();
		}
		else if (psize == sizeof(structs::TraderStatus_Struct))
		{
			DECODE_LENGTH_EXACT(structs::TraderStatus_Struct);
			SETUP_DIRECT_DECODE(TraderStatus_Struct, structs::TraderStatus_Struct);

			emu->Code = eq->Code;	// 11 = Start Trader, 2 = End Trader, 22 = ? - Guessing

			FINISH_DIRECT_DECODE();
		}
	}

	DECODE(OP_TraderBuy)
	{
		DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
		SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);

		IN(Action);
		IN(Price);
		IN(TraderID);
		memcpy(emu->ItemName, eq->ItemName, sizeof(emu->ItemName));
		IN(ItemID);
		IN(Quantity);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TraderShop)
	{
		uint32 psize = __packet->size;
		if (psize == sizeof(structs::TraderClick_Struct))
		{
			DECODE_LENGTH_EXACT(structs::TraderClick_Struct);
			SETUP_DIRECT_DECODE(TraderClick_Struct, structs::TraderClick_Struct);

			IN(Code);
			IN(TraderID);
			IN(Approval);
			LogTrading("DECODE(OP_TraderShop): TraderClick_Struct Code [{}], TraderID [{}], Approval [{}]",
				eq->Code, eq->TraderID, eq->Approval);

			FINISH_DIRECT_DECODE();
		}
		else if (psize == sizeof(structs::BazaarWelcome_Struct))
		{
			// Don't think this size gets used in RoF+ - Leaving for now...
			DECODE_LENGTH_EXACT(structs::BazaarWelcome_Struct);
			SETUP_DIRECT_DECODE(BazaarWelcome_Struct, structs::BazaarWelcome_Struct);

			emu->Beginning.Action = eq->Code;
			IN(Traders);
			IN(Items);
			LogTrading("DECODE(OP_TraderShop): BazaarWelcome_Struct Code [{}], Traders [{}], Items [{}]",
				eq->Code, eq->Traders, eq->Items);

			FINISH_DIRECT_DECODE();
		}
		else if (psize == sizeof(structs::TraderBuy_Struct))
		{

			DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
			SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);

			IN(Action);
			IN(Price);
			IN(TraderID);
			memcpy(emu->ItemName, eq->ItemName, sizeof(emu->ItemName));
			IN(ItemID);
			IN(Quantity);
			LogTrading("DECODE(OP_TraderShop): TraderBuy_Struct (Unknowns) Unknown004 [{}], Unknown008 [{}], Unknown012 [{}], Unknown076 [{}], Unknown276 [{}]",
				eq->Unknown004, eq->Unknown008, eq->Unknown012, eq->Unknown076, eq->Unknown276);
			LogTrading("DECODE(OP_TraderShop): TraderBuy_Struct Buy Action [{}], Price [{}], Trader [{}], ItemID [{}], Quantity [{}], ItemName, [{}]",
				eq->Action, eq->Price, eq->TraderID, eq->ItemID, eq->Quantity, eq->ItemName);

			FINISH_DIRECT_DECODE();
		}
		else if (psize == 4)
		{
			LogTrading("DECODE(OP_TraderShop): Forwarding packet as-is with size 4");
		}
		else
		{
			LogTrading("DECODE(OP_TraderShop): Decode Size Unknown ([{}])", psize);
		}
	}

	DECODE(OP_TradeSkillCombine)
	{
		DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
		SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

		int16 slot_id = TDSToServerSlot(eq->container_slot);
		if (slot_id == 4000) {
			slot_id = 1000; // legacy::SLOT_TRADESKILL;	// 1000
		}
		emu->container_slot = slot_id;
		emu->guildtribute_slot = TDSToServerSlot(eq->guildtribute_slot); // this should only return INVALID_INDEX until implemented

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = TDSToServerSlot(eq->slot);
		IN(quantity);
		IN(tribute_master_id);
		IN(tribute_points);

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
		IN(guildid);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_VetClaimRequest)
	{
		DECODE_LENGTH_EXACT(structs::VeteranClaim);
		SETUP_DIRECT_DECODE(VeteranClaim, structs::VeteranClaim);

		IN(claim_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ZoneChange)
	{
		DECODE_LENGTH_EXACT(structs::ZoneChange_Struct);
		SETUP_DIRECT_DECODE(ZoneChange_Struct, structs::ZoneChange_Struct);

		memcpy(emu->char_name, eq->char_name, sizeof(emu->char_name));
		IN(zoneID);
		IN(instanceID);
		IN(y);
		IN(x);
		IN(z)
		IN(zone_reason);
		IN(success);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ZoneEntry)
	{
		DECODE_LENGTH_EXACT(structs::ClientZoneEntry_Struct);
		SETUP_DIRECT_DECODE(ClientZoneEntry_Struct, structs::ClientZoneEntry_Struct);

		memcpy(emu->char_name, eq->char_name, sizeof(emu->char_name));

		FINISH_DIRECT_DECODE();
	}

// file scope helper methods
	uint32 NextItemInstSerialNumber = 1;
	uint32 MaxInstances = 2000000000;

	static inline int32 GetNextItemInstSerialNumber()
	{
		if (NextItemInstSerialNumber >= MaxInstances)
			NextItemInstSerialNumber = 1;
		else
			NextItemInstSerialNumber++;

		return NextItemInstSerialNumber;
	}

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance* inst, int16 slot_id_in, uint8 depth, ItemPacketType packet_type)
	{
		int ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
		uint8 null_term = 0;
		bool stackable = inst->IsStackable();
		uint32 merchant_slot = inst->GetMerchantSlot();
		uint32 charges = inst->GetCharges();
		if (!stackable && charges > 254)
			charges = 0xFFFFFFFF;

		std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

		const EQ::ItemData *item = inst->GetUnscaledItem();
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Serialize called for: %s", item->Name);

		TDS::structs::ItemSerializationHeader hdr;

		//sprintf(hdr.unknown000, "06e0002Y1W00");

		snprintf(hdr.unknown000, sizeof(hdr.unknown000), "%016d", item->ID);

		hdr.stacksize = stackable ? charges : 1;
		hdr.unknown004 = 0;

		structs::ItemSlotStruct slot_id = ServerToTDSSlot(slot_id_in, packet_type);

		hdr.slot_type = (merchant_slot == 0) ? slot_id.SlotType : 9; // 9 is merchant 20 is reclaim items?
		hdr.main_slot = (merchant_slot == 0) ? slot_id.MainSlot : merchant_slot;
		hdr.sub_slot = (merchant_slot == 0) ? slot_id.SubSlot : 0xffff;
		hdr.aug_slot = (merchant_slot == 0) ? slot_id.AugSlot : 0xffff;
		hdr.price = inst->GetPrice();
		hdr.merchant_slot = (merchant_slot == 0) ? 1 : inst->GetMerchantCount();
		hdr.scaled_value = inst->IsScaling() ? inst->GetExp() / 100 : 0;
		hdr.instance_id = (merchant_slot == 0) ? inst->GetSerialNumber() : merchant_slot;
		hdr.unknown028 = 0;
		hdr.last_cast_time = inst->GetRecastTimestamp();
		hdr.charges = (stackable ? (item->MaxCharges ? 1 : 0) : charges);
		hdr.inst_nodrop = inst->IsAttuned() ? 1 : 0;
		hdr.unknown044 = 0;
		hdr.unknown048 = 0;
		hdr.unknown052 = 0;
		hdr.isEvolving = item->EvolvingLevel > 0 ? 1 : 0;
		ss.write((const char*)&hdr, sizeof(TDS::structs::ItemSerializationHeader));

		if (item->EvolvingLevel > 0) {
			TDS::structs::EvolvingItem evotop;
			evotop.unknown001 = 0;
			evotop.unknown002 = 0;
			evotop.unknown003 = 0;
			evotop.unknown004 = 0;
			evotop.evoLevel = item->EvolvingLevel;
			evotop.progress = 95.512;
			evotop.Activated = 1;
			evotop.evomaxlevel = 7;
			ss.write((const char*)&evotop, sizeof(TDS::structs::EvolvingItem));
		}
		//ORNAMENT IDFILE / ICON
		uint32 ornaIcon = 0;
		uint32 heroModel = 0;

		if (inst->GetOrnamentationIDFile() && inst->GetOrnamentationIcon())
		{
			char tmp[30]; memset(tmp, 0x0, 30); sprintf(tmp, "IT%d", inst->GetOrnamentationIDFile());
			//Mainhand
			ss.write(tmp, strlen(tmp));
			ss.write((const char*)&null_term, sizeof(uint8));
			//Offhand
			ss.write(tmp, strlen(tmp));
			ss.write((const char*)&null_term, sizeof(uint8));
			ornaIcon = inst->GetOrnamentationIcon();
			heroModel = inst->GetOrnamentHeroModel(EQ::InventoryProfile::CalcMaterialFromSlot(slot_id_in));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8)); // no main hand Ornamentation
			ss.write((const char*)&null_term, sizeof(uint8)); // no off hand Ornamentation
		}

		TDS::structs::ItemSerializationHeaderFinish hdrf;
		hdrf.ornamentIcon = ornaIcon;
		hdrf.unknowna1 = 0xffffffff;
		hdrf.ornamentHeroModel = heroModel;
		hdrf.unknown063 = 0;
		hdrf.Copied = 0;
		hdrf.unknowna4 = 0xffffffff;
		hdrf.unknowna5 = 0;
		hdrf.ItemClass = item->ItemClass;

		ss.write((const char*)&hdrf, sizeof(TDS::structs::ItemSerializationHeaderFinish));

		if (strlen(item->Name) > 0)
		{
			ss.write(item->Name, strlen(item->Name));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		if (strlen(item->Lore) > 0)
		{
			ss.write(item->Lore, strlen(item->Lore));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		if (strlen(item->IDFile) > 0)
		{
			ss.write(item->IDFile, strlen(item->IDFile));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		ss.write((const char*)&null_term, sizeof(uint8));
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody struct is %i bytes", sizeof(TDS::structs::ItemBodyStruct));
		TDS::structs::ItemBodyStruct ibs;
		memset(&ibs, 0, sizeof(TDS::structs::ItemBodyStruct));

		ibs.id = item->ID;
		ibs.weight = item->Weight;
		ibs.norent = item->NoRent;
		ibs.nodrop = item->NoDrop;
		ibs.attune = item->Attuneable;
		ibs.size = item->Size;
		ibs.slots = item->Slots; //SwapBits21and22(item->Slots);
		ibs.price = item->Price;
		ibs.icon = item->Icon;
		ibs.unknown1 = 1;
		ibs.unknown2 = 1;
		ibs.BenefitFlag = item->BenefitFlag;
		ibs.tradeskills = item->Tradeskills;
		ibs.CR = item->CR;
		ibs.DR = item->DR;
		ibs.PR = item->PR;
		ibs.MR = item->MR;
		ibs.FR = item->FR;
		ibs.SVCorruption = item->SVCorruption;
		ibs.AStr = item->AStr;
		ibs.ASta = item->ASta;
		ibs.AAgi = item->AAgi;
		ibs.ADex = item->ADex;
		ibs.ACha = item->ACha;
		ibs.AInt = item->AInt;
		ibs.AWis = item->AWis;

		ibs.HP = item->HP;
		ibs.Mana = item->Mana;
		ibs.Endur = item->Endur;
		ibs.AC = item->AC;
		ibs.regen = item->Regen;
		ibs.mana_regen = item->ManaRegen;
		ibs.end_regen = item->EnduranceRegen;
		ibs.Classes = item->Classes;
		ibs.Races = item->Races;
		ibs.Deity = item->Deity;
		ibs.SkillModValue = item->SkillModValue;
		ibs.SkillModMax = 0xffffffff;
		ibs.SkillModType = (int8)(item->SkillModType);
		ibs.SkillModExtra = 0;
		ibs.BaneDmgRace = item->BaneDmgRace;
		ibs.BaneDmgBody = item->BaneDmgBody;
		ibs.BaneDmgRaceAmt = item->BaneDmgRaceAmt;
		ibs.BaneDmgAmt = item->BaneDmgAmt;
		ibs.Magic = item->Magic;
		ibs.CastTime_ = item->CastTime_;
		ibs.ReqLevel = item->ReqLevel;
		if (item->ReqLevel > 100)
			ibs.ReqLevel = 100;
		ibs.RecLevel = item->RecLevel;
		if (item->RecLevel > 100)
			ibs.RecLevel = 100;
		ibs.RecSkill = item->RecSkill;
		ibs.BardType = item->BardType;
		ibs.BardValue = item->BardValue;
		ibs.Light = item->Light;
		ibs.Delay = item->Delay;
		ibs.ElemDmgType = item->ElemDmgType;
		ibs.ElemDmgAmt = item->ElemDmgAmt;
		ibs.Range = item->Range;
		ibs.Damage = item->Damage;
		ibs.Color = item->Color;
		ibs.Prestige = 0;
		ibs.ItemType = item->ItemType;
		ibs.Material = item->Material;
		ibs.MaterialUnknown1 = 0;
		ibs.EliteMaterial = item->EliteMaterial;
		ibs.HerosForgeModel = item->HerosForgeModel;
		ibs.MaterialUnknown2 = 0;
		ibs.SellRate = item->SellRate;
		ibs.CombatEffects = item->CombatEffects;
		ibs.Shielding = item->Shielding;
		ibs.StunResist = item->StunResist;
		ibs.StrikeThrough = item->StrikeThrough;
		ibs.ExtraDmgSkill = item->ExtraDmgSkill;
		ibs.ExtraDmgAmt = item->ExtraDmgAmt;
		ibs.SpellShield = item->SpellShield;
		ibs.Avoidance = item->Avoidance;
		ibs.Accuracy = item->Accuracy;
		ibs.CharmFileID = item->CharmFileID;
		ibs.FactionAmt1 = item->FactionAmt1;
		ibs.FactionMod1 = item->FactionMod1;
		ibs.FactionAmt2 = item->FactionAmt2;
		ibs.FactionMod2 = item->FactionMod2;
		ibs.FactionAmt3 = item->FactionAmt3;
		ibs.FactionMod3 = item->FactionMod3;
		ibs.FactionAmt4 = item->FactionAmt4;
		ibs.FactionMod4 = item->FactionMod4;

		ss.write((const char*)&ibs, sizeof(TDS::structs::ItemBodyStruct));

		//charm text
		if (strlen(item->CharmFile) > 0)
		{
			ss.write((const char*)item->CharmFile, strlen(item->CharmFile));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody secondary struct is %i bytes", sizeof(TDS::structs::ItemSecondaryBodyStruct));
		TDS::structs::ItemSecondaryBodyStruct isbs;
		memset(&isbs, 0, sizeof(TDS::structs::ItemSecondaryBodyStruct));

		isbs.augtype = item->AugType;
		isbs.augrestrict2 = -1;
		isbs.augrestrict = item->AugRestrict;

		for (int x = invaug::SOCKET_BEGIN; x < invaug::SOCKET_END; ++x)
		{
			isbs.augslots[x].type = item->AugSlotType[x];
			isbs.augslots[x].visible = item->AugSlotVisible[x];
			isbs.augslots[x].unknown = item->AugSlotUnk2[x];
		}

		isbs.ldonpoint_type = item->PointType;
		isbs.ldontheme = item->LDoNTheme;
		isbs.ldonprice = item->LDoNPrice;
		isbs.ldonsellbackrate = item->LDoNSellBackRate;
		isbs.ldonsold = item->LDoNSold;

		isbs.bagtype = item->BagType;
		isbs.bagslots = item->BagSlots;
		isbs.bagsize = item->BagSize;
		isbs.wreduction = item->BagWR;

		isbs.book = item->Book;
		isbs.booktype = item->BookType;

		ss.write((const char*)&isbs, sizeof(TDS::structs::ItemSecondaryBodyStruct));

		if (strlen(item->Filename) > 0)
		{
			ss.write((const char*)item->Filename, strlen(item->Filename));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody tertiary struct is %i bytes", sizeof(TDS::structs::ItemTertiaryBodyStruct));
		TDS::structs::ItemTertiaryBodyStruct itbs;
		memset(&itbs, 0, sizeof(TDS::structs::ItemTertiaryBodyStruct));

		itbs.loregroup = item->LoreGroup;
		itbs.artifact = item->ArtifactFlag;
		itbs.summonedflag = item->SummonedFlag;
		itbs.favor = item->Favor;
		itbs.fvnodrop = item->FVNoDrop;
		itbs.dotshield = item->DotShielding;
		itbs.atk = item->Attack;
		itbs.haste = item->Haste;
		itbs.damage_shield = item->DamageShield;
		itbs.guildfavor = item->GuildFavor;
		itbs.augdistil = item->AugDistiller;
		itbs.unknown3 = 0xffffffff;
		itbs.unknown4 = 0;
		itbs.no_pet = item->NoPet;
		itbs.unknown5 = 0;

		itbs.potion_belt_enabled = item->PotionBelt;
		itbs.potion_belt_slots = item->PotionBeltSlots;
		itbs.stacksize = stackable ? item->StackSize : 0;
		itbs.no_transfer = item->NoTransfer;
		itbs.expendablearrow = item->ExpendableArrow;

		itbs.unknown8 = 0;
		itbs.unknown9 = 0;
		itbs.unknown10 = 0;
		itbs.unknown11 = 0;
		itbs.unknown12 = 0;
		itbs.unknown13 = 0;
		itbs.unknown14 = 0;

		ss.write((const char*)&itbs, sizeof(TDS::structs::ItemTertiaryBodyStruct));

		// Effect Structures Broken down to allow variable length strings for effect names
		int32 effect_unknown = 0;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody Click effect struct is %i bytes", sizeof(TDS::structs::ClickEffectStruct));
		TDS::structs::ClickEffectStruct ices;
		memset(&ices, 0, sizeof(TDS::structs::ClickEffectStruct));

		ices.effect = item->Click.Effect;
		ices.level2 = item->Click.Level2;
		ices.type = item->Click.Type;
		ices.level = item->Click.Level;
		ices.max_charges = item->MaxCharges;
		ices.cast_time = item->CastTime;
		ices.recast = item->RecastDelay;
		ices.recast_type = item->RecastType;

		ss.write((const char*)&ices, sizeof(TDS::structs::ClickEffectStruct));

		if (strlen(item->ClickName) > 0)
		{
			ss.write((const char*)item->ClickName, strlen(item->ClickName));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		ss.write((const char*)&effect_unknown, sizeof(int32));	// clickunk7

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody proc effect struct is %i bytes", sizeof(TDS::structs::ProcEffectStruct));
		TDS::structs::ProcEffectStruct ipes;
		memset(&ipes, 0, sizeof(TDS::structs::ProcEffectStruct));

		ipes.effect = item->Proc.Effect;
		ipes.level2 = item->Proc.Level2;
		ipes.type = item->Proc.Type;
		ipes.level = item->Proc.Level;
		ipes.procrate = item->ProcRate;

		ss.write((const char*)&ipes, sizeof(TDS::structs::ProcEffectStruct));

		if (strlen(item->ProcName) > 0)
		{
			ss.write((const char*)item->ProcName, strlen(item->ProcName));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown5

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody worn effect struct is %i bytes", sizeof(TDS::structs::WornEffectStruct));
		TDS::structs::WornEffectStruct iwes;
		memset(&iwes, 0, sizeof(TDS::structs::WornEffectStruct));

		iwes.effect = item->Worn.Effect;
		iwes.level2 = item->Worn.Level2;
		iwes.type = item->Worn.Type;
		iwes.level = item->Worn.Level;

		ss.write((const char*)&iwes, sizeof(TDS::structs::WornEffectStruct));

		if (strlen(item->WornName) > 0)
		{
			ss.write((const char*)item->WornName, strlen(item->WornName));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		TDS::structs::WornEffectStruct ifes;
		memset(&ifes, 0, sizeof(TDS::structs::WornEffectStruct));

		ifes.effect = item->Focus.Effect;
		ifes.level2 = item->Focus.Level2;
		ifes.type = item->Focus.Type;
		ifes.level = item->Focus.Level;

		ss.write((const char*)&ifes, sizeof(TDS::structs::WornEffectStruct));

		if (strlen(item->FocusName) > 0)
		{
			ss.write((const char*)item->FocusName, strlen(item->FocusName));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		TDS::structs::WornEffectStruct ises;
		memset(&ises, 0, sizeof(TDS::structs::WornEffectStruct));

		ises.effect = item->Scroll.Effect;
		ises.level2 = item->Scroll.Level2;
		ises.type = item->Scroll.Type;
		ises.level = item->Scroll.Level;

		ss.write((const char*)&ises, sizeof(TDS::structs::WornEffectStruct));

		if (strlen(item->ScrollName) > 0)
		{
			ss.write((const char*)item->ScrollName, strlen(item->ScrollName));
			ss.write((const char*)&null_term, sizeof(uint8));
		}
		else
		{
			ss.write((const char*)&null_term, sizeof(uint8));
		}

		ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		// Bard Effect?
		TDS::structs::WornEffectStruct ibes;
		memset(&ibes, 0, sizeof(TDS::structs::WornEffectStruct));

		ibes.effect = item->Bard.Effect;
		ibes.level2 = item->Bard.Level2;
		ibes.type = item->Bard.Type;
		ibes.level = item->Bard.Level;
		//ibes.unknown6 = 0xffffffff;

		ss.write((const char*)&ibes, sizeof(TDS::structs::WornEffectStruct));

		/*
		if(strlen(item->BardName) > 0)
		{
		ss.write((const char*)item->BardName, strlen(item->BardName));
		ss.write((const char*)&null_term, sizeof(uint8));
		}
		else */
		ss.write((const char*)&null_term, sizeof(uint8));

		ss.write((const char*)&effect_unknown, sizeof(int32));	// unknown6
		// End of Effects

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] ItemBody Quaternary effect struct is %i bytes", sizeof(TDS::structs::ItemQuaternaryBodyStruct));
		TDS::structs::ItemQuaternaryBodyStruct iqbs;
		memset(&iqbs, 0, sizeof(TDS::structs::ItemQuaternaryBodyStruct));

		iqbs.scriptfileid = item->ScriptFileID;
		iqbs.quest_item = item->QuestItemFlag;
		iqbs.Power = 0;
		iqbs.Purity = item->Purity;
		iqbs.unknown16 = 0;
		iqbs.BackstabDmg = item->BackstabDmg;
		iqbs.DSMitigation = item->DSMitigation;
		iqbs.HeroicStr = item->HeroicStr;
		iqbs.HeroicInt = item->HeroicInt;
		iqbs.HeroicWis = item->HeroicWis;
		iqbs.HeroicAgi = item->HeroicAgi;
		iqbs.HeroicDex = item->HeroicDex;
		iqbs.HeroicSta = item->HeroicSta;
		iqbs.HeroicCha = item->HeroicCha;
		iqbs.HeroicMR = item->HeroicMR;
		iqbs.HeroicFR = item->HeroicFR;
		iqbs.HeroicCR = item->HeroicCR;
		iqbs.HeroicDR = item->HeroicDR;
		iqbs.HeroicPR = item->HeroicPR;
		iqbs.HeroicSVCorrup = item->HeroicSVCorrup;
		iqbs.HealAmt = item->HealAmt;
		iqbs.SpellDmg = item->SpellDmg;
		iqbs.clairvoyance = item->Clairvoyance;

		//unknown18;	//Power Source Capacity or evolve filename?
		//evolve_string; // Some String, but being evolution related is just a guess

		iqbs.Heirloom = 0;
		iqbs.Placeable = 0;

		iqbs.unknown28 = -1;
		iqbs.unknown30 = -1;

		iqbs.NoZone = 0;
		iqbs.NoGround = 0;
		iqbs.unknown37a = 0;	// (guessed position) New to RoF2
		iqbs.unknown38 = 0;

		iqbs.unknown39 = 1;

		iqbs.subitem_count = 0;

		ss.write((const char*)&iqbs, sizeof(TDS::structs::ItemQuaternaryBodyStruct));
		EQ::OutBuffer::pos_type count_pos = ss.tellp();

		EQ::OutBuffer SubSerializations;
		//char *SubSerializations[10]; // <watch>

		uint32 SubLengths[10];

		for (int x = EQ::invslot::SLOT_BEGIN; x < EQ::invbag::SLOT_COUNT; ++x) {

			//SubSerializations[x] = nullptr;

			const EQ::ItemInstance* subitem = ((const EQ::ItemInstance*)inst)->GetItem(x);

			if (subitem) {

				int SubSlotNumber;

				iqbs.subitem_count++;

				if (slot_id_in >= EQ::invslot::GENERAL_BEGIN && slot_id_in <= EQ::invslot::GENERAL_END) // (< 30) - no cursor?
					//SubSlotNumber = (((slot_id_in + 3) * 10) + x + 1);
					SubSlotNumber = (((slot_id_in + 3) * EQ::invslot::GENERAL_COUNT) + x + 1);
				else if (slot_id_in >= EQ::invslot::BANK_BEGIN && slot_id_in <= EQ::invslot::BANK_END)
					//SubSlotNumber = (((slot_id_in - 2000) * 10) + 2030 + x + 1);
					SubSlotNumber = (((slot_id_in - EQ::invslot::BANK_BEGIN) * EQ::invbag::SLOT_COUNT) + EQ::invslot::BANK_BEGIN + x);
				else if (slot_id_in >= EQ::invslot::SHARED_BANK_BEGIN && slot_id_in <= EQ::invslot::SHARED_BANK_END)
					//SubSlotNumber = (((slot_id_in - 2500) * 10) + 2530 + x + 1);
					SubSlotNumber = (((slot_id_in - EQ::invslot::SHARED_BANK_BEGIN) * EQ::invbag::SLOT_COUNT) + EQ::invslot::SHARED_BANK_BEGIN + x);
				else
					SubSlotNumber = slot_id_in; // ???????

				/*
				// TEST CODE: <watch>
				SubSlotNumber = Inventory::CalcSlotID(slot_id_in, x);
				*/

				SerializeItem(SubSerializations, subitem, SubSlotNumber, depth + 1, packet_type);
				//SubSerializations[x] = SerializeItem(subitem, SubSlotNumber, &SubLengths[x], depth + 1, packet_type);
			}
		}

		ss.write((const char*)&iqbs, sizeof(TDS::structs::ItemQuaternaryBodyStruct));

		for (int x = EQ::invslot::SLOT_BEGIN; x < EQ::invbag::SLOT_COUNT; ++x) {

			if (SubSerializations) {

				ss.write((const char*)&x, sizeof(uint32));

				//ss.write(SubSerializations, SubLengths[x]);

				delete[] &SubSerializations;
			}
		}

		/*
		char* item_serial = new char[ss.tellp()];
		memset(item_serial, 0, ss.tellp());
		memcpy(item_serial, ss.str().c_str(), ss.tellp());

		*length = ss.tellp();
		*/
	}

	static inline structs::ItemSlotStruct ServerToTDSSlot(uint32 serverSlot, ItemPacketType PacketType)
	{
		structs::ItemSlotStruct TDSSlot;
		TDSSlot.SlotType = INVALID_INDEX;
		TDSSlot.Unknown02 = INULL;
		TDSSlot.MainSlot = INVALID_INDEX;
		TDSSlot.SubSlot = INVALID_INDEX;
		TDSSlot.AugSlot = INVALID_INDEX;
		TDSSlot.Unknown01 = INULL;

		uint32 TempSlot = 0;

		if (serverSlot < 56 || serverSlot == MainPowerSource) { // Main Inventory and Cursor
			if (PacketType == ItemPacketLoot)
			{
				TDSSlot.SlotType = maps::MapCorpse;
				TDSSlot.MainSlot = serverSlot - EQ::invslot::CORPSE_BEGIN;
			}
			else
			{
				TDSSlot.SlotType = maps::MapPossessions;
				TDSSlot.MainSlot = serverSlot;
			}

			if (serverSlot == MainPowerSource)
				TDSSlot.MainSlot = slots::MainPowerSource;

			else if (serverSlot >= MainCursor && PacketType != ItemPacketLoot) // Cursor and Extended Corpse Inventory
				TDSSlot.MainSlot += 3;

			else if (serverSlot >= MainAmmo) // (> 20)
				TDSSlot.MainSlot += 1;
		}

		/*else if (ServerSlot < 51) { // Cursor Buffer
		TDSSlot.SlotType = maps::MapLimbo;
		TDSSlot.MainSlot = ServerSlot - 31;
		}*/

		else if (serverSlot >= EQ::invbag::GENERAL_BAGS_BEGIN && serverSlot <= EQ::invbag::CURSOR_BAG_END) { // (> 250 && < 341) (251-360)
			TDSSlot.SlotType = maps::MapPossessions;
			TempSlot = serverSlot - 1;
			TDSSlot.MainSlot = int(TempSlot / EQ::invbag::SLOT_COUNT) - 2;
			TDSSlot.SubSlot = TempSlot - ((TDSSlot.MainSlot + 2) * EQ::invbag::SLOT_COUNT);

			if (TDSSlot.MainSlot >= slots::MainGeneral9) // (> 30)
				TDSSlot.MainSlot = slots::MainCursor;
		}

		else if (serverSlot >= EQ::invslot::TRIBUTE_BEGIN && serverSlot <= EQ::invslot::TRIBUTE_END) { // Tribute
			TDSSlot.SlotType = maps::MapTribute;
			TDSSlot.MainSlot = serverSlot - EQ::invslot::TRIBUTE_BEGIN;
		}

		else if (serverSlot >= EQ::invslot::BANK_BEGIN && serverSlot <= EQ::invslot::BANK_END) {
			TDSSlot.SlotType = maps::MapBank;
			TempSlot = serverSlot - EQ::invslot::BANK_BEGIN;
			TDSSlot.MainSlot = TempSlot;

			if (TempSlot > 30) { // (> 30)
				TDSSlot.MainSlot = int(TempSlot / EQ::invbag::SLOT_COUNT) - 3;
				TDSSlot.SubSlot = TempSlot - ((TDSSlot.MainSlot + 3) * EQ::invbag::SLOT_COUNT);
			}
		}

		else if (serverSlot >= EQ::invslot::SHARED_BANK_BEGIN && serverSlot <= EQ::invslot::SHARED_BANK_END) {
			TDSSlot.SlotType = maps::MapSharedBank;
			TempSlot = serverSlot - EQ::invslot::SHARED_BANK_BEGIN;
			TDSSlot.MainSlot = TempSlot;

			if (TempSlot > 30) { // (> 30)
				TDSSlot.MainSlot = int(TempSlot / EQ::invbag::SLOT_COUNT) - 3;
				TDSSlot.SubSlot = TempSlot - ((TDSSlot.MainSlot + 3) * EQ::invbag::SLOT_COUNT);
			}
		}

		else if (serverSlot >= EQ::invslot::TRADE_BEGIN && serverSlot <= EQ::invslot::TRADE_END) {
			TDSSlot.SlotType = maps::MapTrade;
			TempSlot = serverSlot - EQ::invslot::TRADE_BEGIN;
			TDSSlot.MainSlot = TempSlot;

			if (TempSlot > 30) {
				TDSSlot.MainSlot = int(TempSlot / EQ::invbag::SLOT_COUNT) - 3;
				TDSSlot.SubSlot = TempSlot - ((TDSSlot.MainSlot + 3) * EQ::invbag::SLOT_COUNT);
			}

			/*
			// OLD CODE:
			if (TempSlot > 99) {
			if (TempSlot > 100)
			TDSSlot.MainSlot = int((TempSlot - 100) / 10);

			else
			TDSSlot.MainSlot = 0;

			TDSSlot.SubSlot = TempSlot - (100 + TDSSlot.MainSlot);
			}
			*/
		}

		else if (serverSlot >= EQ::invslot::WORLD_BEGIN && serverSlot <= EQ::invslot::WORLD_END) {
			TDSSlot.SlotType = maps::MapWorld;
			TempSlot = serverSlot - EQ::invslot::WORLD_BEGIN;
			TDSSlot.MainSlot = TempSlot;
		}

		Log(Logs::Detail, Logs::Netcode, "Convert Server Slot %i to TDS Slot: Type %i, Unk2 %i, Main %i, Sub %i, Aug %i, Unk1 %i",
			serverSlot, TDSSlot.SlotType, TDSSlot.Unknown02, TDSSlot.MainSlot, TDSSlot.SubSlot, TDSSlot.AugSlot, TDSSlot.Unknown01);
		return TDSSlot;
	}

	static inline structs::MainInvItemSlotStruct ServerToTDSMainInvSlot(uint32 serverSlot)
	{
		structs::MainInvItemSlotStruct TDSSlot;
		TDSSlot.MainSlot = INVALID_INDEX;
		TDSSlot.SubSlot = INVALID_INDEX;
		TDSSlot.AugSlot = INVALID_INDEX;
		TDSSlot.Unknown01 = INULL;

		uint32 TempSlot = 0;

		if (serverSlot < 56 || serverSlot == MainPowerSource) { // (< 52)
			TDSSlot.MainSlot = serverSlot;

			if (serverSlot == MainPowerSource)
				TDSSlot.MainSlot = slots::MainPowerSource;

			else if (serverSlot >= MainCursor) // Cursor and Extended Corpse Inventory
				TDSSlot.MainSlot += 3;

			else if (serverSlot >= MainAmmo) // Ammo and Personl Inventory
				TDSSlot.MainSlot += 1;

			/*else if (ServerSlot >= MainCursor) { // Cursor
			TDSSlot.MainSlot = slots::MainCursor;

			if (ServerSlot > 30)
			TDSSlot.SubSlot = (ServerSlot + 3) - 33;
			}*/
		}

		else if (serverSlot >= EQ::invbag::GENERAL_BAGS_BEGIN && serverSlot <= EQ::invbag::CURSOR_BAG_END) {
			TempSlot = serverSlot - 1;
			TDSSlot.MainSlot = int(TempSlot / EQ::invbag::SLOT_COUNT) - 2;
			TDSSlot.SubSlot = TempSlot - ((TDSSlot.MainSlot + 2) * EQ::invbag::SLOT_COUNT);
		}

		Log(Logs::Detail, Logs::Netcode, "Convert Server Slot %i to TDS Slot: Main %i, Sub %i, Aug %i, Unk %i)",
			serverSlot, TDSSlot.MainSlot, TDSSlot.SubSlot, TDSSlot.AugSlot, TDSSlot.Unknown01);

		return TDSSlot;
	}

	static inline uint32 ServerToTDSCorpseSlot(uint32 serverCorpseSlot)
	{
		return (serverCorpseSlot - EQ::invslot::CORPSE_BEGIN + 1);
	}

	static inline uint32 TDSToServerSlot(structs::ItemSlotStruct tdsSlot, ItemPacketType PacketType)
	{
		uint32 ServerSlot = INVALID_INDEX;
		uint32 TempSlot = 0;

		if (tdsSlot.SlotType == maps::MapPossessions && tdsSlot.MainSlot < 57) { // Worn/Personal Inventory and Cursor (< 51)
			if (tdsSlot.MainSlot == slots::MainPowerSource)
				TempSlot = MainPowerSource;

			else if (tdsSlot.MainSlot >= slots::MainCursor) // Cursor and Extended Corpse Inventory
				TempSlot = tdsSlot.MainSlot - 3;

			/*else if (tdsSlot.MainSlot == slots::MainGeneral9 || tdsSlot.MainSlot == slots::MainGeneral10) { // 9th and 10th RoF2 inventory/corpse slots
			// Need to figure out what to do when we get these

			// The slot range of 0 - client_max is cross-utilized between player inventory and corpse inventory.
			// In the case of RoF2, player inventory is addressed as 0 - 33 and corpse inventory is addressed as 23 - 56.
			// We 'could' assign the two new inventory slots as 9997 and 9998, and then work around their bag
			// slot assignments, but doing so may disrupt our ability to utilize the corpse looting range properly.

			// For now, it's probably best to leave as-is and let this work itself out in the inventory rework.
			}*/

			else if (tdsSlot.MainSlot >= slots::MainAmmo) // Ammo and Main Inventory
				TempSlot = tdsSlot.MainSlot - 1;

			else // Worn Slots
				TempSlot = tdsSlot.MainSlot;

			if (tdsSlot.SubSlot >= EQ::invbag::GENERAL_BAGS_BEGIN) // Bag Slots
				TempSlot = ((TempSlot + 3) * EQ::invbag::SLOT_COUNT) + tdsSlot.SubSlot + 1;

			ServerSlot = TempSlot;
		}

		else if (tdsSlot.SlotType == maps::MapBank) {
			TempSlot = EQ::invslot::BANK_BEGIN;

			if (tdsSlot.SubSlot >= EQ::invbag::BANK_BAGS_BEGIN)
				TempSlot += ((tdsSlot.MainSlot + 3) * EQ::invbag::SLOT_COUNT) + tdsSlot.SubSlot + 1;

			else
				TempSlot += tdsSlot.MainSlot;

			ServerSlot = TempSlot;
		}

		else if (tdsSlot.SlotType == maps::MapSharedBank) {
			TempSlot = EQ::invslot::SHARED_BANK_BEGIN;

			if (tdsSlot.SubSlot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN)
				TempSlot += ((tdsSlot.MainSlot + 3) * EQ::invbag::SLOT_COUNT) + tdsSlot.SubSlot + 1;

			else
				TempSlot += tdsSlot.MainSlot;

			ServerSlot = TempSlot;
		}

		else if (tdsSlot.SlotType == maps::MapTrade) {
			TempSlot = EQ::invslot::TRADE_BEGIN;

			if (tdsSlot.SubSlot >= EQ::invbag::TRADE_BAGS_BEGIN)
				TempSlot += ((tdsSlot.MainSlot + 3) * EQ::invbag::SLOT_COUNT) + tdsSlot.SubSlot + 1;
			// OLD CODE:
			//TempSlot += 100 + (tdsSlot.MainSlot * EmuConstants::ITEM_CONTAINER_SIZE) + tdsSlot.SubSlot;

			else
				TempSlot += tdsSlot.MainSlot;

			ServerSlot = TempSlot;
		}

		else if (tdsSlot.SlotType == maps::MapWorld) {
			TempSlot = EQ::invslot::WORLD_BEGIN;

			if (tdsSlot.MainSlot >= EQ::invslot::WORLD_BEGIN)
				TempSlot += tdsSlot.MainSlot;

			ServerSlot = TempSlot;
		}

		/*else if (tdsSlot.SlotType == maps::MapLimbo) { // Cursor Buffer
		TempSlot = 31;

		if (tdsSlot.MainSlot >= 0)
		TempSlot += tdsSlot.MainSlot;

		ServerSlot = TempSlot;
		}*/

		else if (tdsSlot.SlotType == maps::MapGuildTribute) {
			ServerSlot = INVALID_INDEX;
		}

		else if (tdsSlot.SlotType == maps::MapCorpse) {
			ServerSlot = tdsSlot.MainSlot + EQ::invslot::CORPSE_BEGIN;
		}

		Log(Logs::Detail, Logs::Netcode, "Convert TDS Slot: Type: %i, Unk2 %i, Main %i, Sub %i, Aug %i, Unk1 %i to Server Slot %i",
			tdsSlot.SlotType, tdsSlot.Unknown02, tdsSlot.MainSlot, tdsSlot.SubSlot, tdsSlot.AugSlot, tdsSlot.Unknown01, ServerSlot);

		return ServerSlot;
	}

	static inline uint32 TDSToServerMainInvSlot(structs::MainInvItemSlotStruct tdsSlot)
	{
		uint32 ServerSlot = INVALID_INDEX;
		uint32 TempSlot = 0;

		if (tdsSlot.MainSlot < 57) { // Worn/Personal Inventory and Cursor (< 33)
			if (tdsSlot.MainSlot == slots::MainPowerSource)
				TempSlot = MainPowerSource;

			else if (tdsSlot.MainSlot >= slots::MainCursor) // Cursor and Extended Corpse Inventory
				TempSlot = tdsSlot.MainSlot - 3;

			/*else if (tdsSlot.MainSlot == slots::MainGeneral9 || tdsSlot.MainSlot == slots::MainGeneral10) { // 9th and 10th RoF2 inventory slots
			// Need to figure out what to do when we get these

			// Same as above
			}*/

			else if (tdsSlot.MainSlot >= slots::MainAmmo) // Main Inventory and Ammo Slots
				TempSlot = tdsSlot.MainSlot - 1;

			else
				TempSlot = tdsSlot.MainSlot;

			if (tdsSlot.SubSlot >= EQ::invbag::GENERAL_BAGS_BEGIN) // Bag Slots
				TempSlot = ((TempSlot + 3) * EQ::invbag::SLOT_COUNT) + tdsSlot.SubSlot + 1;

			ServerSlot = TempSlot;
		}

		Log(Logs::Detail, Logs::Netcode, "Convert TDS Slot: Main %i, Sub %i, Aug %i, Unk1 %i to Server Slot %i",
			tdsSlot.MainSlot, tdsSlot.SubSlot, tdsSlot.AugSlot, tdsSlot.Unknown01, ServerSlot);

		return ServerSlot;
	}

	static inline uint32 TDSToServerCorpseSlot(uint32 tdsCorpseSlot)
	{
		return (tdsCorpseSlot + EQ::invslot::CORPSE_BEGIN - 1);
	}

	static inline void ServerToTDSTextLink(std::string& tdsTextLink, const std::string& serverTextLink)
	{
		if ((constants::SAY_LINK_BODY_SIZE == EQ::constants::SAY_LINK_BODY_SIZE) || (serverTextLink.find('\x12') == std::string::npos)) {
			tdsTextLink = serverTextLink;
			return;
		}

		auto segments = SplitString(serverTextLink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= EQ::constants::SAY_LINK_BODY_SIZE) {
					tdsTextLink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// TDS:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// TDS:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// Diff:

				tdsTextLink.push_back('\x12');
				tdsTextLink.append(segments[segment_iter]);
				tdsTextLink.push_back('\x12');
			}
			else {
				tdsTextLink.append(segments[segment_iter]);
			}
		}
	}

	static inline void TDSToServerTextLink(std::string& serverTextLink, const std::string& tdsTextLink)
	{
		if ((EQ::constants::SAY_LINK_BODY_SIZE == constants::SAY_LINK_BODY_SIZE) || (tdsTextLink.find('\x12') == std::string::npos)) {
			serverTextLink = tdsTextLink;
			return;
		}

		auto segments = SplitString(tdsTextLink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= constants::SAY_LINK_BODY_SIZE) {
					serverTextLink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// TDS:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// TDS:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// Diff:

				serverTextLink.push_back('\x12');
				serverTextLink.append(segments[segment_iter]);
				serverTextLink.push_back('\x12');
			}
			else {
				serverTextLink.append(segments[segment_iter]);
			}
		}
	}
}
// end namespace TDS
