/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/linked_list.h"
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "horse.h"
#include "mob.h"

std::map<uint16, const NPCType *> Horse::horse_types;
LinkedList<NPCType *> horses_auto_delete;

Horse::Horse(Client *_owner, uint16 spell_id, const glm::vec4& position)
 : NPC(GetHorseType(spell_id), nullptr, position, GravityBehavior::Water)
{
	//give the horse its proper name.
	strn0cpy(name, _owner->GetCleanName(), 55);
	strcat(name,"`s_Mount00");

	is_horse = true;

	owner = _owner;
}

void Horse::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	NPC::FillSpawnStruct(ns, ForWho);

//	ns->spawn.texture = NPCTypedata->mount_color;
	ns->spawn.petOwnerId = 0;

	//dunno why we do these, they should allready be set right.
	ns->spawn.runspeed = NPCTypedata->runspeed;
}

bool Horse::IsHorseSpell(uint16 spell_id) {
	//written in terms of a function which does a ton more work
	//than we need to to figure out if this is a horse spell.
	//the logic is that people calling this function will post
	//likely immediately summon the horse, so we need the extra anyways.
	return(GetHorseType(spell_id) != nullptr);
}

const NPCType *Horse::GetHorseType(uint16 spell_id) {
	if(horse_types.count(spell_id) == 1)
		return(horse_types[spell_id]);
	//cache invalid spell IDs as nullptr entries
	const NPCType *ret;
	horse_types[spell_id] = ret = BuildHorseType(spell_id);
	return(ret);
}

const NPCType *Horse::BuildHorseType(uint16 spell_id) {

	const char* fileName = spells[spell_id].teleport_zone;

	std::string query = StringFormat("SELECT race, gender, texture, mountspeed FROM horses WHERE filename = '%s'", fileName);
	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return nullptr;
	}

	if (results.RowCount() != 1) {
		LogError("No Database entry for mount: [{}], check the horses table", fileName);
		return nullptr;
	}

    auto row = results.begin();

    auto npc_type = new NPCType;
    memset(npc_type, 0, sizeof(NPCType));
    strcpy(npc_type->name, "Unclaimed_Mount"); // this should never get used

    strcpy(npc_type->special_abilities, "19,1^20,1^24,1");
    npc_type->current_hp = 1;
    npc_type->max_hp = 1;
    npc_type->race = atoi(row[0]);
    npc_type->gender = atoi(row[1]); // Drogmor's are female horses. Yuck.
    npc_type->class_ = 1;
    npc_type->deity = 1;
    npc_type->level = 1;
    npc_type->npc_id = 0;
    npc_type->loottable_id = 0;
    npc_type->texture = atoi(row[2]);     // mount color
    npc_type->helmtexture = atoi(row[2]); // mount color
    npc_type->runspeed = atof(row[3]);

    npc_type->light = 0;
    npc_type->STR = 75;
    npc_type->STA = 75;
    npc_type->DEX = 75;
    npc_type->AGI = 75;
    npc_type->INT = 75;
    npc_type->WIS = 75;
    npc_type->CHA = 75;
    horses_auto_delete.Insert(npc_type);

    return npc_type;
}

void Client::SummonHorse(uint16 spell_id) {
	if (GetHorseId() != 0) {
		Message(Chat::Red,"You already have a Horse. Get off, Fatbutt!");
		return;
	}
	if(!Horse::IsHorseSpell(spell_id)) {
		LogError("[{}] tried to summon an unknown horse, spell id [{}]", GetName(), spell_id);
		return;
	}

	// No Horse, lets get them one.

	auto horse = new Horse(this, spell_id, GetPosition());

	//we want to manage the spawn packet ourself.
	//another reason is we dont want quests executing on it.
	entity_list.AddNPC(horse, false);

	// Okay, lets say they have a horse now.


	EQApplicationPacket outapp;
	horse->CreateHorseSpawnPacket(&outapp, GetName(), GetID());
/*	// Doodman: Kludged in here instead of adding a field to PCType. FIXME!
	NewSpawn_Struct* ns=(NewSpawn_Struct*)outapp->pBuffer;
	ns->spawn.texture=mount_color;
	ns->spawn.pet_owner_id=0;
	ns->spawn.walkspeed=npc_type->walkspeed;
	ns->spawn.runspeed=npc_type->runspeed;
*/
	entity_list.QueueClients(horse, &outapp);


	uint16 tmpID = horse->GetID();
	SetHorseId(tmpID);
	BuffFadeBySitModifier();

}

void Client::SetHorseId(uint16 horseid_in) {
	//if its the same, do nothing
	if(horseId == horseid_in)
		return;

	//otherwise it changed.
	//if we have a horse, get rid of it no matter what.
	if(horseId) {
		Mob *horse = entity_list.GetMob(horseId);
		if(horse != nullptr)
			horse->Depop();
	}

	//now we take whatever they gave us.
	horseId = horseid_in;
}

void Mob::CreateHorseSpawnPacket(EQApplicationPacket* app, const char* ownername, uint16 ownerid, Mob* ForWho) {
	app->SetOpcode(OP_NewSpawn);
	app->pBuffer = new uchar[sizeof(NewSpawn_Struct)];
	app->size = sizeof(NewSpawn_Struct);
	memset(app->pBuffer, 0, sizeof(NewSpawn_Struct));
	NewSpawn_Struct* ns = (NewSpawn_Struct*)app->pBuffer;
	FillSpawnStruct(ns, ForWho);

#if (EQDEBUG >= 11)
	printf("Horse Spawn Packet - Owner: %s\n", ownername);
	DumpPacket(app);
#endif
}

