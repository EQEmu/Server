
#include "../debug.h"
#include "Client62.h"
#include "../opcodemgr.h"
#include "../logsys.h"
#include "../EQStreamIdent.h"
#include "../crc32.h"

#include "../eq_packet_structs.h"
#include "../MiscFunctions.h"
#include "../Item.h"
#include "../clientversions.h"
#include "Client62_structs.h"

namespace Client62 {

static const char *name = "6.2";
static OpcodeManager *opcodes = nullptr;
static Strategy struct_strategy;

char *SerializeItem(const ItemInst *inst, int16 slot_id, uint32 *length, uint8 depth);
	
void Register(EQStreamIdentifier &into) {
	//create our opcode manager if we havent already
	if(opcodes == nullptr) {
		//TODO: get this file name from the config file
		string opfile = "patch_";
		opfile += name;
		opfile += ".conf";
		//load up the opcode manager.
		//TODO: figure out how to support shared memory with multiple patches...
		opcodes = new RegularOpcodeManager();
		if(!opcodes->LoadOpcodes(opfile.c_str())) {
			_log(NET__OPCODES, "Error loading opcodes file %s. Not registering patch %s.", opfile.c_str(), name);
			return;
		}
	}
	
	//ok, now we have what we need to register.
	
	EQStream::Signature signature;
	string pname;
	
	//register our world signature.
	pname = string(name) + "_world";
	signature.ignore_eq_opcode = 0;
	signature.first_length = sizeof(structs::LoginInfo_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_SendLoginInfo);
	into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);
	
	//register our zone signature.
	pname = string(name) + "_zone";
	signature.ignore_eq_opcode = opcodes->EmuToEQ(OP_AckPacket);
	signature.first_length = sizeof(structs::ClientZoneEntry_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_ZoneEntry);
	into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);
	
	_log(NET__IDENTIFY, "Registered patch %s", name);
}

void Reload() {
	
	//we have a big problem to solve here when we switch back to shared memory
	//opcode managers because we need to change the manager pointer, which means
	//we need to go to every stream and replace it's manager.
	
	if(opcodes != nullptr) {
		//TODO: get this file name from the config file
		string opfile = "patch_";
		opfile += name;
		opfile += ".conf";
		if(!opcodes->ReloadOpcodes(opfile.c_str())) {
			_log(NET__OPCODES, "Error reloading opcodes file %s for patch %s.", opfile.c_str(), name);
			return;
		}
		_log(NET__OPCODES, "Reloaded opcodes for patch %s", name);
	}
}



Strategy::Strategy()
: StructStrategy()
{
	//all opcodes default to passthrough.
	#include "SSRegister.h"
	#include "Client62_ops.h"
}

std::string Strategy::Describe() const {
	std::string r;
	r += "Patch ";
	r += name;
	return(r);
}

const EQClientVersion Strategy::ClientVersion() const
{
	return EQClient62;
}

#include "SSDefine.h"


EAT_ENCODE(OP_ZoneServerReady)
EAT_ENCODE(OP_GuildMemberLevelUpdate)

ENCODE(OP_SendCharInfo) {
	ENCODE_LENGTH_EXACT(CharacterSelect_Struct);
	SETUP_DIRECT_ENCODE(CharacterSelect_Struct, structs::CharacterSelect_Struct);
	int r;
	for(r = 0; r < 10; r++) {
		OUT(zone[r]);
		OUT(eyecolor1[r]);
		OUT(eyecolor2[r]);
		OUT(hairstyle[r]);
		OUT(primary[r]);
		OUT(race[r]);
		OUT(class_[r]);
		OUT_str(name[r]);
		OUT(gender[r]);
		OUT(level[r]);
		OUT(secondary[r]);
		OUT(face[r]);
		OUT(beard[r]);
		int k;
		for(k = 0; k < 9; k++) {
			OUT(equip[r][k]);
			OUT(cs_colors[r][k].color);
		}
		OUT(haircolor[r]);
		OUT(gohome[r]);
		OUT(deity[r]);
		OUT(beardcolor[r]);
	}
	FINISH_ENCODE();
}

ENCODE(OP_SendAATable) {
	ENCODE_LENGTH_ATLEAST(SendAA_Struct);
	
	SETUP_VAR_ENCODE(SendAA_Struct);
	ALLOC_VAR_ENCODE(structs::SendAA_Struct, sizeof(structs::SendAA_Struct) + emu->total_abilities*sizeof(structs::AA_Ability));
	
	// Check clientver field to verify this AA should be sent for SoF
	// clientver 1 is for all clients and 2 is for 6.2
	if (emu->clientver <= 2 )
	{
		OUT(id);
		OUT(hotkey_sid);
		OUT(hotkey_sid2);
		OUT(title_sid);
		OUT(desc_sid);
		OUT(class_type);
		OUT(cost);
		OUT(seq);
		OUT(current_level);
		OUT(prereq_skill);
		OUT(prereq_minpoints);
		OUT(type);
		OUT(spellid);
		OUT(spell_type);
		OUT(spell_refresh);
		OUT(classes);
		OUT(berserker);
		OUT(max_level);
		OUT(last_id);
		OUT(next_id);
		OUT(cost2);
		OUT(unknown80[0]);
		OUT(unknown80[1]);
		OUT(total_abilities);
		unsigned int r;
		for(r = 0; r < emu->total_abilities; r++) {
			OUT(abilities[r].skill_id);
			OUT(abilities[r].base1);
			OUT(abilities[r].base2);
			OUT(abilities[r].slot);
		}
	}
	
	FINISH_ENCODE();
}

ENCODE(OP_LeadershipExpUpdate) {
	SETUP_DIRECT_ENCODE(LeadershipExpUpdate_Struct, structs::LeadershipExpUpdate_Struct);
	OUT(group_leadership_exp);
	OUT(group_leadership_points);
	OUT(raid_leadership_exp);
	OUT(raid_leadership_points);
	FINISH_ENCODE();
}

ENCODE(OP_DeleteSpawn) {
	SETUP_DIRECT_ENCODE(DeleteSpawn_Struct, structs::DeleteSpawn_Struct);
	OUT(spawn_id);
	FINISH_ENCODE();
}

ENCODE(OP_PlayerProfile) {
	SETUP_DIRECT_ENCODE(PlayerProfile_Struct, structs::PlayerProfile_Struct);
	
	uint32 r;
	
	memset(eq->unknown3224, 0xff, 448);
	memset(eq->unknown3704, 0xff, 32);
	
//	OUT(checksum);
	OUT(gender);
	OUT(race);
	OUT(class_);
	OUT(level);
	eq->level2 = emu->level;
	
	eq->bind_zone_id = emu->binds[0].zoneId;
	eq->bind_x[0] = emu->binds[0].x;
	eq->bind_y[0] = emu->binds[0].y;
	eq->bind_z[0] = emu->binds[0].z;
	eq->bind_heading[0] = emu->binds[0].heading;
	//just making this up base on organization of struct:
	eq->zone_safe_x = emu->binds[4].x;
	eq->zone_safe_y = emu->binds[4].y;
	eq->zone_safe_z = emu->binds[4].z;
	eq->zone_safe_heading = emu->binds[4].heading;
	
	
	OUT(deity);
	OUT(intoxication);
	OUT_array(spellSlotRefresh, structs::MAX_PP_MEMSPELL);
	OUT(abilitySlotRefresh);
//	OUT(unknown0166[4]);
	OUT(haircolor);
	OUT(beardcolor);
	OUT(eyecolor1);
	OUT(eyecolor2);
	OUT(hairstyle);
	OUT(beard);
	for(r = 0; r < 9; r++) {
		OUT(item_material[r]);
		OUT(item_tint[r].color);
	}
	for(r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
		OUT(aa_array[r].AA);
		OUT(aa_array[r].value);
	}
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
	OUT_array(spell_book, structs::MAX_PP_SPELLBOOK);
	OUT_array(mem_spells, structs::MAX_PP_MEMSPELL);
	OUT(platinum);
	OUT(gold);
	OUT(silver);
	OUT(copper);
	OUT(platinum_cursor);
	OUT(gold_cursor);
	OUT(silver_cursor);
	OUT(copper_cursor);
	OUT_array(skills, structs::MAX_PP_SKILL);
	OUT(toxicity);
	OUT(thirst_level);
	OUT(hunger_level);
	for(r = 0; r < structs::BUFF_COUNT; r++) {
		OUT(buffs[r].slotid);
		OUT(buffs[r].level);
		OUT(buffs[r].bard_modifier);
		OUT(buffs[r].effect);
		OUT(buffs[r].spellid);
		OUT(buffs[r].duration);
		OUT(buffs[r].counters);
//		OUT(buffs[r].player_id);
	}
	for(r = 0; r < structs::MAX_PP_DISCIPLINES; r++) {
		OUT(disciplines.values[r]);
	}
//	OUT_array(recastTimers, structs::MAX_RECAST_TYPES);
	OUT(endurance);
	OUT(aapoints_spent);
	OUT(aapoints);
	for(r = 0; r < structs::MAX_PLAYER_BANDOLIER; r++) {
		OUT_str(bandoliers[r].name);
		uint32 k;
		for(k = 0; k < structs::MAX_PLAYER_BANDOLIER_ITEMS; k++) {
			OUT(bandoliers[r].items[k].item_id);
			OUT(bandoliers[r].items[k].icon);
			OUT_str(bandoliers[r].items[k].item_name);
		}
	}
	for(r = 0; r < structs::MAX_PLAYER_BANDOLIER_ITEMS; r++) {
		OUT(potionbelt.items[r].item_id);
		OUT(potionbelt.items[r].icon);
		OUT_str(potionbelt.items[r].item_name);
	}
//	OUT(available_slots);
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
	OUT(exp);
	OUT_array(languages, structs::MAX_PP_LANGUAGE);
	OUT(x);
	OUT(y);
	OUT(z);
	OUT(heading);
	OUT(platinum_bank);
	OUT(gold_bank);
	OUT(silver_bank);
	OUT(copper_bank);
	OUT(platinum_shared);
	OUT(expansions);
	OUT(autosplit);
	OUT(zone_id);
	OUT(zoneInstance);
	for(r = 0; r < structs::MAX_GROUP_MEMBERS; r++) {
		OUT_str(groupMembers[r]);
	}
//	OUT_str(groupLeader);	//this is prolly right after groupMembers, but I dont feel like checking.
//	OUT(leadAAActive);
	OUT(ldon_points_guk);
	OUT(ldon_points_mir);
	OUT(ldon_points_mmc);
	OUT(ldon_points_ruj);
	OUT(ldon_points_tak);
	OUT(ldon_points_available);
	OUT(tribute_time_remaining);
	OUT(career_tribute_points);
	OUT(tribute_points);
	OUT(tribute_active);
	for(r = 0; r < structs::MAX_PLAYER_TRIBUTES; r++) {
		OUT(tributes[r].tribute);
		OUT(tributes[r].tier);
	}
	OUT(group_leadership_exp);
	OUT(raid_leadership_exp);
	OUT(group_leadership_points);
	OUT(raid_leadership_points);
	OUT_array(leader_abilities.ranks, structs::MAX_LEADERSHIP_AA_ARRAY);
	OUT(air_remaining);
	OUT(entityid);
	OUT(leadAAActive);
	OUT(expAA);
	OUT(currentRadCrystals);
	OUT(careerRadCrystals);
	OUT(currentEbonCrystals);
	OUT(careerEbonCrystals);
	OUT(groupAutoconsent);
	OUT(raidAutoconsent);
	OUT(guildAutoconsent);
//	OUT(showhelm);
	
	//set the checksum...
	CRC32::SetEQChecksum(__packet->pBuffer, sizeof(structs::PlayerProfile_Struct)-4);
	
	FINISH_ENCODE();
}

ENCODE(OP_Track)
{

	EQApplicationPacket *in = *p;
	*p = nullptr;

	unsigned char *__emu_buffer = in->pBuffer;
	Track_Struct *emu = (Track_Struct *) __emu_buffer;

	int EntryCount = in->size / sizeof(Track_Struct);

	if(EntryCount == 0 || ((in->size % sizeof(Track_Struct))) != 0)
	{
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Track_Struct));
		delete in;
		return;
	}

	in->size = sizeof(structs::Track_Struct) * EntryCount;
	in->pBuffer = new unsigned char[in->size];
	structs::Track_Struct *eq = (structs::Track_Struct *) in->pBuffer;

	for(int i = 0; i < EntryCount; ++i, ++eq, ++emu)
	{
		OUT(entityid);
		OUT(padding002);
		OUT(distance);
	}

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_NewSpawn) {  ENCODE_FORWARD(OP_ZoneSpawns); }
ENCODE(OP_ZoneEntry){  ENCODE_FORWARD(OP_ZoneSpawns); }
ENCODE(OP_ZoneSpawns) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	Spawn_Struct *emu = (Spawn_Struct *) __emu_buffer;
	
	//determine and verify length
	int entrycount = in->size / sizeof(Spawn_Struct);
	if(entrycount == 0 || (in->size % sizeof(Spawn_Struct)) != 0) {
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
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
	for(r = 0; r < entrycount; r++, eq++, emu++) {
		eq->gm = emu->gm;
		eq->aaitle = emu->aaitle;
		eq->anon = emu->anon;
		eq->face = emu->face;
		strcpy(eq->name, emu->name);
		eq->deity = emu->deity;
		eq->size = emu->size;
		eq->NPC = emu->NPC;
		eq->invis = emu->invis;
		eq->haircolor = emu->haircolor;
		eq->curHp = emu->curHp;
		eq->max_hp = emu->max_hp;
		eq->findable = emu->findable;
		eq->deltaHeading = emu->deltaHeading;
		eq->x = emu->x;
		eq->y = emu->y;
		eq->animation = emu->animation;
		eq->z = emu->z;
		eq->deltaY = emu->deltaY;
		eq->deltaX = emu->deltaX;
		eq->heading = emu->heading;
		eq->deltaZ = emu->deltaZ;
		eq->eyecolor1 = emu->eyecolor1;
//		eq->showhelm = emu->showhelm;
		eq->is_npc = emu->is_npc;
		eq->hairstyle = emu->hairstyle;
		eq->beard = emu->beard;
		eq->level = emu->level;
		eq->beardcolor = emu->beardcolor;
		strcpy(eq->suffix, emu->suffix);
		eq->petOwnerId = emu->petOwnerId;
		eq->guildrank = emu->guildrank;
		for(k = 0; k < 9; k++) {
			eq->equipment[k] = emu->equipment[k];
			eq->colors[k].color = emu->colors[k].color;
		}
		for(k = 0; k < 8; k++) {
			eq->set_to_0xFF[k] = 0xFF;
		}
		eq->runspeed = emu->runspeed;
		eq->afk = emu->afk;
		eq->guildID = emu->guildID;
		strcpy(eq->title, emu->title);
		eq->helm = emu->helm;
		eq->race = emu->race;
		strcpy(eq->lastName, emu->lastName);
		eq->walkspeed = emu->walkspeed;
		eq->is_pet = emu->is_pet;
		eq->light = emu->light;
		eq->class_ = emu->class_;
		eq->eyecolor2 = emu->eyecolor2;
		eq->gender = emu->gender;
		eq->bodytype = emu->bodytype;
		eq->equip_chest2 = emu->equip_chest2;
		eq->spawnId = emu->spawnId;
		eq->lfg = emu->lfg;
		eq->flymode = emu->flymode;
	}
	
	//kill off the emu structure and send the eq packet.
	delete[] __emu_buffer;
	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_ItemLinkResponse) {  ENCODE_FORWARD(OP_ItemPacket); }
ENCODE(OP_ItemPacket) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	ItemPacket_Struct *old_item_pkt=(ItemPacket_Struct *)__emu_buffer;
	InternalSerializedItem_Struct *int_struct=(InternalSerializedItem_Struct *)(old_item_pkt->SerializedItem);

	uint32 length;
	char *serialized=SerializeItem((const ItemInst *)int_struct->inst,int_struct->slot_id,&length,0);

	if (!serialized) {
		_log(NET__STRUCTS, "Serialization failed on item slot %d.",int_struct->slot_id);
		delete in;
		return;
	}
	in->size = length+5;	// ItemPacketType + Serialization + \0
	in->pBuffer = new unsigned char[in->size];
	ItemPacket_Struct *new_item_pkt=(ItemPacket_Struct *)in->pBuffer;
	new_item_pkt->PacketType=old_item_pkt->PacketType;
	memcpy(new_item_pkt->SerializedItem,serialized,length+1);

	delete[] __emu_buffer;
	safe_delete_array(serialized);
	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_CharInventory) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;

	int itemcount = in->size / sizeof(InternalSerializedItem_Struct);
	if(itemcount == 0 || (in->size % sizeof(InternalSerializedItem_Struct)) != 0) {
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(InternalSerializedItem_Struct));
		delete in;
		return;
	}
	InternalSerializedItem_Struct *eq = (InternalSerializedItem_Struct *) in->pBuffer;
	
	//do the transform...
	int r;
	string serial_string;
	for(r = 0; r < itemcount; r++, eq++) {
		uint32 length;
		char *serialized=SerializeItem((ItemInst*)eq->inst,eq->slot_id,&length,0);
		if (serialized) {
			serial_string.append(serialized,length+1);
			safe_delete_array(serialized);
		} else {
			_log(NET__STRUCTS, "Serialization failed on item slot %d during OP_CharInventory.  Item skipped.",eq->slot_id);
		}
	
	}

	in->size = serial_string.length();
	in->pBuffer = new unsigned char[in->size];
	memcpy(in->pBuffer,serial_string.c_str(),serial_string.length());

	delete[] __emu_buffer;
	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_GuildMemberList) {
	//consume the packet
	EQApplicationPacket *in = *p;
	*p = nullptr;
	
	//store away the emu struct
	unsigned char *__emu_buffer = in->pBuffer;
	Internal_GuildMembers_Struct *emu = (Internal_GuildMembers_Struct *) in->pBuffer;
	
	
	
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
	*((uint32 *) buffer) = htonl( emu->count );
	buffer += sizeof(uint32);
	
	if(emu->count > 0) {
		Internal_GuildMemberEntry_Struct *emu_e = emu->member;
		const char *emu_name = (const char *) (__emu_buffer + 
				sizeof(Internal_GuildMembers_Struct) + //skip header
				emu->count * sizeof(Internal_GuildMemberEntry_Struct)	//skip static length member data
				);
		const char *emu_note = (emu_name +
				emu->name_length + //skip name contents
				emu->count	//skip string terminators
				);
		
		structs::GuildMemberEntry_Struct *e = (structs::GuildMemberEntry_Struct *) buffer;
		
		uint32 r;
		for(r = 0; r < emu->count; r++, emu_e++) {
			
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
#define PutFieldN(field) \
		e->field = htonl(emu_e->field)
			
			SlideStructString( name, emu_name );
			PutFieldN(level);
			PutFieldN(banker);
			PutFieldN(class_);
			PutFieldN(rank);
			PutFieldN(time_last_on);
			PutFieldN(tribute_enable);
			PutFieldN(total_tribute);
			PutFieldN(last_tribute);
			SlideStructString( public_note, emu_note );
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

ENCODE(OP_ReadBook) {

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

ENCODE(OP_Illusion) {
	ENCODE_LENGTH_EXACT(Illusion_Struct);
	SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);
	OUT(spawnid);
	OUT_str(charname);
	if(emu->race > 473){
		eq->race = 1;
	}
	else {
		OUT(race);
	}
	OUT(gender);
	OUT(texture);
	OUT(helmtexture);
	OUT(face);
	OUT(hairstyle);
	OUT(haircolor);
	OUT(beard);
	OUT(beardcolor);
	OUT(size);

	FINISH_ENCODE();
}

ENCODE(OP_BazaarSearch)
{
	EQApplicationPacket *in = *p;
	*p = nullptr;

	char *Buffer = (char *)in->pBuffer;

	uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);

	if(SubAction != BazaarSearchResults)
	{
		dest->FastQueuePacket(&in, ack_req);

		return;
	}

	unsigned char *__emu_buffer = in->pBuffer;

	BazaarSearchResults_Struct *emu = (BazaarSearchResults_Struct *) __emu_buffer;

	int EntryCount = in->size / sizeof(BazaarSearchResults_Struct);

	if(EntryCount == 0 || (in->size % sizeof(BazaarSearchResults_Struct)) != 0)
	{
		_log(NET__STRUCTS, "Wrong size on outbound %s: Got %d, expected multiple of %d", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(BazaarSearchResults_Struct));
		delete in;
		return;
	}
	in->size = EntryCount * sizeof(structs::BazaarSearchResults_Struct);

	in->pBuffer = new unsigned char[in->size];

	memset(in->pBuffer, 0, in->size);

	structs::BazaarSearchResults_Struct *eq = (structs::BazaarSearchResults_Struct *)in->pBuffer;

	for(int i = 0; i < EntryCount; ++i, ++emu, ++eq)
	{
		OUT(Beginning.Action);
		OUT(NumItems);
		OUT(SerialNumber);
		OUT(SellerID);
		OUT(Cost);
		OUT(ItemStat);
		memcpy(eq->ItemName, emu->ItemName, sizeof(eq->ItemName));
	}

	delete[] __emu_buffer;

	dest->FastQueuePacket(&in, ack_req);
}

ENCODE(OP_RespondAA) {
	ENCODE_LENGTH_EXACT(AATable_Struct);
	SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

	unsigned int r;
	for(r = 0; r < MAX_PP_AA_ARRAY; r++) {
		OUT(aa_list[r].aa_skill);
		OUT(aa_list[r].aa_value);
	}

	FINISH_ENCODE();
}

ENCODE(OP_WearChange) {
	ENCODE_LENGTH_EXACT(WearChange_Struct);
	SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);
	OUT(spawn_id);
	OUT(material);
	OUT(color.color);
	OUT(wear_slot_id);
	FINISH_ENCODE();
}

ENCODE(OP_Action) {
	ENCODE_LENGTH_EXACT(Action_Struct);
	SETUP_DIRECT_ENCODE(Action_Struct, structs::Action_Struct);
	OUT(target);
	OUT(source);
	OUT(level);
	OUT(instrument_mod);
	OUT(sequence);
	OUT(type);
	//OUT(damage);
	OUT(spell);
	OUT(buff_unknown); // if this is 4, a buff icon is made
	FINISH_ENCODE();
}

ENCODE(OP_BecomeTrader)
{
	ENCODE_LENGTH_EXACT(BecomeTrader_Struct);
	SETUP_DIRECT_ENCODE(BecomeTrader_Struct, structs::BecomeTrader_Struct);
	OUT(ID);
	OUT(Code);
	FINISH_ENCODE();
}

ENCODE(OP_PetBuffWindow)
{
	ENCODE_LENGTH_EXACT(PetBuff_Struct);
	SETUP_DIRECT_ENCODE(PetBuff_Struct, PetBuff_Struct);

	OUT(petid);
	OUT(buffcount);

	int EQBuffSlot = 0;

	for(uint32 EmuBuffSlot = 0; EmuBuffSlot < BUFF_COUNT; ++EmuBuffSlot)
	{
		if(emu->spellid[EmuBuffSlot])
		{
			eq->spellid[EQBuffSlot] = emu->spellid[EmuBuffSlot];
			eq->ticsremaining[EQBuffSlot++] = emu->ticsremaining[EmuBuffSlot];
		}
	}

	FINISH_ENCODE();
}

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

DECODE(OP_WearChange) {
	DECODE_LENGTH_EXACT(structs::WearChange_Struct);
	SETUP_DIRECT_DECODE(WearChange_Struct, structs::WearChange_Struct);
	IN(spawn_id);
	IN(material);
	IN(color.color);
	IN(wear_slot_id);
	FINISH_DIRECT_DECODE();
}

DECODE(OP_ItemLinkClick) {
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

DECODE(OP_SetServerFilter) {
	DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
	SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);
	int r;
	for(r = 0; r < 25; r++) {
		IN(filters[r]);
	}
	emu->filters[25] = 1;
	emu->filters[26] = 1;
	emu->filters[27] = 1;
	emu->filters[28] = 1;
	FINISH_DIRECT_DECODE();
}

DECODE(OP_CharacterCreate) {
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
	FINISH_DIRECT_DECODE();
}

DECODE(OP_WhoAllRequest) {
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

DECODE(OP_ReadBook) {
	DECODE_LENGTH_ATLEAST(structs::BookRequest_Struct);
	SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

	IN(window);
	IN(type);
	strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

	FINISH_DIRECT_DECODE();
}

DECODE(OP_FaceChange) {

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

char *SerializeItem(const ItemInst *inst, int16 slot_id, uint32 *length, uint8 depth) {
	char *serialization = nullptr;
	char *instance = nullptr;
	const char *protection=(const char *)"\\\\\\\\\\";
	char *sub_items[10] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	bool stackable=inst->IsStackable();
	uint32 merchant_slot=inst->GetMerchantSlot();
	int16 charges=inst->GetCharges();
	const Item_Struct *item=inst->GetItem();
	int i;
	uint32 sub_length;

	MakeAnyLenString(&instance,
		"%i|%i|%i|%i|%i|%i|%i|%i|%i|%i|",
		stackable ? charges : 1,
		0,
		(merchant_slot==0) ? slot_id : merchant_slot,
		inst->GetPrice(),
		(merchant_slot==0) ? 1 : inst->GetMerchantCount(),
		0,
		//merchant_slot,	//instance ID, bullshit for now
		// The 'Merchant Slot' needs to be some unique id for bazaar to work properly
		(merchant_slot==0) ? inst->GetSerialNumber() : merchant_slot,
		inst->IsInstNoDrop() ? 1 : 0,		//not sure where this field is
		(stackable ? ((inst->GetItem()->ItemType == ItemTypePotion) ? charges : 0) : charges),
		0
	);

	for(i=0;i<10;i++) {
		ItemInst *sub=inst->GetItem(i);
		if (sub) {
			sub_items[i]=SerializeItem(sub,0,&sub_length,depth+1);
		}
	}

	
	*length=MakeAnyLenString(&serialization,
		"%.*s%s"	// For leading quotes (and protection) if a subitem;
		"%s"		// Instance data
		"%.*s\""	// Quotes (and protection, if needed) around static data
		"%i"		// item->ItemClass so we can do |%s instead of %s|
#define I(field) "|%i"
#define C(field) "|%s"
#define S(field) "|%s"
#define F(field) "|%f"
#include "Client62_itemfields.h"
		"%.*s\""	// Quotes (and protection, if needed) around static data
		"|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s"	// Sub items
		"%.*s%s"	// For trailing quotes (and protection) if a subitem;
		,depth ? depth-1 : 0,protection,(depth) ? "\"" : ""
		,instance
		,depth,protection
		,item->ItemClass
#define I(field) ,item->field
#define C(field) ,field
#define S(field) ,item->field
#define F(field) ,item->field
#include "Client62_itemfields.h"
		,depth,protection
		,sub_items[0] ? sub_items[0] : ""
		,sub_items[1] ? sub_items[1] : ""
		,sub_items[2] ? sub_items[2] : ""
		,sub_items[3] ? sub_items[3] : ""
		,sub_items[4] ? sub_items[4] : ""
		,sub_items[5] ? sub_items[5] : ""
		,sub_items[6] ? sub_items[6] : ""
		,sub_items[7] ? sub_items[7] : ""
		,sub_items[8] ? sub_items[8] : ""
		,sub_items[9] ? sub_items[9] : ""
		,(depth) ? depth-1 : 0,protection,(depth) ? "\"" : ""
	);

	for(i=0;i<10;i++) {
		if (sub_items[i])
			safe_delete_array(sub_items[i]);
	}

	safe_delete_array(instance);

	return serialization;
}


} //end namespace Client62






