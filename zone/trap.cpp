/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/types.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "trap.h"

/*

Schema:
CREATE TABLE traps (
	id int(11) NOT NULL auto_increment,
	zone varchar(16) NOT NULL default '',
	x int(11) NOT NULL default '0',
	y int(11) NOT NULL default '0',
	z int(11) NOT NULL default '0',
	chance tinyint NOT NULL default '0',
	maxzdiff float NOT NULL default '0',
	radius float NOT NULL default '0',
	effect int(11) NOT NULL default '0',
	effectvalue int(11) NOT NULL default '0',
	effectvalue2 int(11) NOT NULL default '0',
	message varcahr(200) NOT NULL;
	skill int(11) NOT NULL default '0',
	spawnchance int(11) NOT NULL default '0',
	PRIMARY KEY (id)
) TYPE=MyISAM;


*/

Trap::Trap() :
	Entity(),
	respawn_timer(600000),
	chkarea_timer(500),
	m_Position(glm::vec3())
{
	trap_id = 0;
	maxzdiff = 0;
	radius = 0;
	effect = 0;
	effectvalue = 0;
	effectvalue2 = 0;
	skill = 0;
	level = 0;
	respawn_timer.Disable();
	detected = false;
	disarmed = false;
	respawn_time = 0;
	respawn_var = 0;
	hiddenTrigger = nullptr;
	ownHiddenTrigger = false;
}

Trap::~Trap()
{
	//don't need to clean up mob as traps are always cleaned up same time as NPCs
	//cleaning up mob here can actually cause a crash via race condition
}

bool Trap::Process()
{
	if (chkarea_timer.Enabled() && chkarea_timer.Check()
		/*&& zone->GetClientCount() > 0*/ )
	{
		Mob* trigger = entity_list.GetTrapTrigger(this);
		if (trigger && !(trigger->IsClient() && trigger->CastToClient()->GetGM()))
		{
			Trigger(trigger);
		}
	}
	if (respawn_timer.Enabled() && respawn_timer.Check())
	{
		detected = false;
		disarmed = false;
		chkarea_timer.Enable();
		respawn_timer.Disable();
	}
	return true;
}

void Trap::Trigger(Mob* trigger)
{
	int i = 0;
	const NPCType* tmp = 0;
	switch (effect)
	{
		case trapTypeDebuff:
			if(message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"%s triggers a trap!",trigger->GetName());
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}
			if(hiddenTrigger){
				hiddenTrigger->SpellFinished(effectvalue, trigger, 10, 0, -1, spells[effectvalue].ResistDiff);
			}
			break;
		case trapTypeAlarm:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,effectvalue,13,"A loud alarm rings out through the air...");
			}
			else
			{
				entity_list.MessageClose(trigger,false,effectvalue,13,"%s",message.c_str());
			}

			entity_list.SendAlarm(this,trigger,effectvalue);
			break;
		case trapTypeMysticSpawn:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"The air shimmers...");
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}

			for (i = 0; i < effectvalue2; i++)
			{
				if ((tmp = database.LoadNPCTypesData(effectvalue)))
				{
					auto randomOffset = glm::vec4(zone->random.Int(-5, 5),zone->random.Int(-5, 5),zone->random.Int(-5, 5), zone->random.Int(0, 249));
					auto spawnPosition = randomOffset + glm::vec4(m_Position, 0.0f);
					auto new_npc = new NPC(tmp, nullptr, spawnPosition, FlyMode3);
					new_npc->AddLootTable();
					entity_list.AddNPC(new_npc);
					new_npc->AddToHateList(trigger,1);
				}
			}
			break;
		case trapTypeBanditSpawn:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"A bandit leaps out from behind a tree!");
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}

			for (i = 0; i < effectvalue2; i++)
			{
				if ((tmp = database.LoadNPCTypesData(effectvalue)))
				{
					auto randomOffset = glm::vec4(zone->random.Int(-2, 2), zone->random.Int(-2, 2), zone->random.Int(-2, 2), zone->random.Int(0, 249));
					auto spawnPosition = randomOffset + glm::vec4(m_Position, 0.0f);
					auto new_npc = new NPC(tmp, nullptr, spawnPosition, FlyMode3);
					new_npc->AddLootTable();
					entity_list.AddNPC(new_npc);
					new_npc->AddToHateList(trigger,1);
				}
			}
			break;
		case trapTypeDamage:
			if (message.empty())
			{
				entity_list.MessageClose(trigger,false,100,13,"%s triggers a trap!",trigger->GetName());
			}
			else
			{
				entity_list.MessageClose(trigger,false,100,13,"%s",message.c_str());
			}
			if(trigger->IsClient())
			{
				auto outapp = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
				CombatDamage_Struct* a = (CombatDamage_Struct*)outapp->pBuffer;
				int dmg = zone->random.Int(effectvalue, effectvalue2);
				trigger->SetHP(trigger->GetHP() - dmg);
				a->damage = dmg;
				a->meleepush_xy = zone->random.Int(0, 1234567);
				a->source = GetHiddenTrigger()!=nullptr ? GetHiddenTrigger()->GetID() : trigger->GetID();
				a->spellid = 0;
				a->target = trigger->GetID();
				a->type = 253;
				trigger->CastToClient()->QueuePacket(outapp);
				safe_delete(outapp);
			}
	}
	respawn_timer.Start((respawn_time + zone->random.Int(0, respawn_var)) * 1000);
	chkarea_timer.Disable();
	disarmed = true;
}

Trap* EntityList::FindNearbyTrap(Mob* searcher, float max_dist) {
	float dist = 999999;
	Trap* current_trap = nullptr;

	float max_dist2 = max_dist*max_dist;
	Trap *cur;

	for (auto it = trap_list.begin(); it != trap_list.end(); ++it) {
		cur = it->second;
		if(cur->disarmed)
			continue;

		auto diff = glm::vec3(searcher->GetPosition()) - cur->m_Position;
		float curdist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

		if (curdist < max_dist2 && curdist < dist)
		{
			dist = curdist;
			current_trap = cur;
		}
	}

	return current_trap;
}

Mob* EntityList::GetTrapTrigger(Trap* trap) {
	Mob* savemob = 0;

	float maxdist = trap->radius * trap->radius;

	for (auto it = client_list.begin(); it != client_list.end(); ++it) {
		Client* cur = it->second;

		auto diff = glm::vec3(cur->GetPosition()) - trap->m_Position;
		diff.z = std::abs(diff.z);

		if ((diff.x*diff.x + diff.y*diff.y) <= maxdist
			&& diff.z < trap->maxzdiff)
		{
			if (zone->random.Roll(trap->chance))
				return(cur);
			else
				savemob = cur;
		}

	}

	return savemob;
}

//todo: rewrite this to not need direct access to trap members.
bool ZoneDatabase::LoadTraps(const char* zonename, int16 version) {

	std::string query = StringFormat("SELECT id, x, y, z, effect, effectvalue, effectvalue2, skill, "
									"maxzdiff, radius, chance, message, respawn_time, respawn_var, level "
									"FROM traps WHERE zone='%s' AND version=%u", zonename, version);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto trap = new Trap();
		trap->trap_id = atoi(row[0]);
		trap->m_Position = glm::vec3(atof(row[1]), atof(row[2]), atof(row[3]));
		trap->effect = atoi(row[4]);
		trap->effectvalue = atoi(row[5]);
		trap->effectvalue2 = atoi(row[6]);
		trap->skill = atoi(row[7]);
		trap->maxzdiff = atof(row[8]);
		trap->radius = atof(row[9]);
		trap->chance = atoi(row[10]);
		trap->message = row[11];
		trap->respawn_time = atoi(row[12]);
		trap->respawn_var = atoi(row[13]);
		trap->level = atoi(row[14]);
		entity_list.AddTrap(trap);
		trap->CreateHiddenTrigger();
	}

	return true;
}

void Trap::CreateHiddenTrigger()
{
	if(hiddenTrigger)
		return;

	const NPCType *base_type = database.LoadNPCTypesData(500);
	auto make_npc = new NPCType;
	memcpy(make_npc, base_type, sizeof(NPCType));
	make_npc->max_hp = 100000;
	make_npc->cur_hp = 100000;
	strcpy(make_npc->name, "a_trap");
	make_npc->runspeed = 0.0f;
	make_npc->bodytype = BT_Special;
	make_npc->race = 127;
	make_npc->gender = 0;
	make_npc->loottable_id = 0;
	make_npc->npc_spells_id = 0;
	make_npc->d_melee_texture1 = 0;
	make_npc->d_melee_texture2 = 0;
	make_npc->trackable = 0;
	make_npc->level = level;
	strcpy(make_npc->special_abilities, "19,1^20,1^24,1^25,1");
	NPC* npca = new NPC(make_npc, nullptr, glm::vec4(m_Position, 0.0f), FlyMode3);
	npca->GiveNPCTypeData(make_npc);
	entity_list.AddNPC(npca);

	hiddenTrigger = npca;
	ownHiddenTrigger = true;
}
