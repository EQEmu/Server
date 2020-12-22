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
#include "../common/repositories/criteria/content_filter_criteria.h"

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
	chkarea_timer(1000),
	reset_timer(5000),
	m_Position(glm::vec3())
{
	trap_id = 0;
	db_id = 0;
	maxzdiff = 0;
	radius = 0;
	effect = 0;
	effectvalue = 0;
	effectvalue2 = 0;
	skill = 0;
	level = 0;
	respawn_timer.Disable();
	reset_timer.Disable();
	detected = false;
	disarmed = false;
	respawn_time = 0;
	respawn_var = 0;
	hiddenTrigger = nullptr;
	ownHiddenTrigger = false;
	chance = 0;
	triggered_number = 0;
	times_triggered = 0;
	group = 0;
	despawn_when_triggered = false;
	charid = 0;
	undetectable = false;
}

Trap::~Trap()
{
	//don't need to clean up mob as traps are always cleaned up same time as NPCs
	//cleaning up mob here can actually cause a crash via race condition
}

bool Trap::Process()
{
	if (chkarea_timer.Enabled() && chkarea_timer.Check() && !reset_timer.Enabled())
	{
		Mob* trigger = entity_list.GetTrapTrigger(this);
		if (trigger && !(trigger->IsClient() && trigger->CastToClient()->GetGM()))
		{
			Trigger(trigger);
		}
	}
	else if (reset_timer.Enabled() && reset_timer.Check())
	{
		Log(Logs::General, Logs::Traps, "Reset timer disabled in Reset Check Process for trap %d.", trap_id);
		reset_timer.Disable();
		charid = 0;
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
	Log(Logs::General, Logs::Traps, "Trap %d triggered by %s for the %d time!", trap_id, trigger->GetName(), times_triggered + 1);

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
				hiddenTrigger->SpellFinished(effectvalue, trigger, EQ::spells::CastingSlot::Item, 0, -1, spells[effectvalue].ResistDiff);
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

			entity_list.SendAlarm(this,trigger, effectvalue2);
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
				if ((tmp = content_db.LoadNPCTypesData(effectvalue)))
				{
					auto randomOffset = glm::vec4(zone->random.Int(-5, 5),zone->random.Int(-5, 5),zone->random.Int(-5, 5), zone->random.Int(0, 249));
					auto spawnPosition = randomOffset + glm::vec4(m_Position, 0.0f);
					auto new_npc = new NPC(tmp, nullptr, spawnPosition, GravityBehavior::Flying);
					new_npc->AddLootTable();
					if (new_npc->DropsGlobalLoot())
						new_npc->CheckGlobalLootTables();
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
				if ((tmp = content_db.LoadNPCTypesData(effectvalue)))
				{
					auto randomOffset = glm::vec4(zone->random.Int(-2, 2), zone->random.Int(-2, 2), zone->random.Int(-2, 2), zone->random.Int(0, 249));
					auto spawnPosition = randomOffset + glm::vec4(m_Position, 0.0f);
					auto new_npc = new NPC(tmp, nullptr, spawnPosition, GravityBehavior::Flying);
					new_npc->AddLootTable();
					if (new_npc->DropsGlobalLoot())
						new_npc->CheckGlobalLootTables();
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
				a->hit_heading = 0.0f;
				a->source = GetHiddenTrigger()!=nullptr ? GetHiddenTrigger()->GetID() : trigger->GetID();
				a->spellid = 0;
				a->target = trigger->GetID();
				a->type = 253;
				trigger->CastToClient()->QueuePacket(outapp);
				safe_delete(outapp);
			}
	}

	if (trigger && trigger->IsClient())
	{
		trigger->CastToClient()->trapid = trap_id;
		charid = trigger->CastToClient()->CharacterID();
	}

	bool update = false;
	if (despawn_when_triggered)
	{
		Log(Logs::General, Logs::Traps, "Trap %d is despawning after being triggered.", trap_id);
		update = true;
	}
	else
	{
		reset_timer.Start(5000);
	}

	if (triggered_number > 0)
		++times_triggered;

	if (triggered_number > 0 && triggered_number <= times_triggered)
	{
		Log(Logs::General, Logs::Traps, "Triggered number for trap %d reached. %d/%d", trap_id, times_triggered, triggered_number);
		update = true;
	}

	if (update)
	{
		UpdateTrap();
	}
}

Trap* EntityList::FindNearbyTrap(Mob* searcher, float max_dist, float &trap_curdist, bool detected)
{
	float dist = 999999;
	Trap* current_trap = nullptr;

	float max_dist2 = max_dist*max_dist;
	Trap *cur;

	for (auto it = trap_list.begin(); it != trap_list.end(); ++it) {
		cur = it->second;
		if(cur->disarmed || (detected && !cur->detected) || cur->undetectable)
			continue;

		auto diff = glm::vec3(searcher->GetPosition()) - cur->m_Position;
		float curdist = diff.x*diff.x + diff.y*diff.y;
		diff.z = std::abs(diff.z);

		if (curdist < max_dist2 && curdist < dist && diff.z <= cur->maxzdiff)
		{
			Log(Logs::General, Logs::Traps, "Trap %d is curdist %0.1f", cur->db_id, curdist);
			dist = curdist;
			current_trap = cur;
		}
	}

	if (current_trap != nullptr)
	{
		Log(Logs::General, Logs::Traps, "Trap %d is the closest trap.", current_trap->db_id);
		trap_curdist = dist;
	}
	else
		 trap_curdist = INVALID_INDEX;

	return current_trap;
}

Mob* EntityList::GetTrapTrigger(Trap* trap)
{

	float maxdist = trap->radius * trap->radius;
	for (auto it = client_list.begin(); it != client_list.end(); ++it)
	{
		Client* cur = it->second;

		auto diff = glm::vec3(cur->GetPosition()) - trap->m_Position;
		diff.z = std::abs(diff.z);

		if ((diff.x*diff.x + diff.y*diff.y) <= maxdist
			&& diff.z <= trap->maxzdiff)
		{
			//This prevents the trap from triggering on players while zoning.
			if (strcmp(cur->GetName(), "No name") == 0)
				continue;

			if (cur->trapid == 0 && !cur->GetGM() && (trap->chance == 0 || zone->random.Roll(trap->chance)))
			{
				Log(Logs::General, Logs::Traps, "%s is about to trigger trap %d of chance %d. diff: %0.2f maxdist: %0.2f zdiff: %0.2f maxzdiff: %0.2f", cur->GetName(), trap->trap_id, trap->chance, (diff.x*diff.x + diff.y*diff.y), maxdist, diff.z, trap->maxzdiff);
				return cur;
			}
		}
		else
		{
			if (cur->trapid == trap->trap_id)
			{
				Log(Logs::General, Logs::Traps, "%s is clearing trapid for trap %d", cur->GetName(), trap->trap_id);
				cur->trapid = 0;
			}
		}
	}

	return nullptr;
}

bool EntityList::IsTrapGroupSpawned(uint32 trap_id, uint8 group)
{
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap() && cur->group == group && cur->trap_id != trap_id)
		{
			return true;
		}
		++it;
	}

	return false;
}

void EntityList::UpdateAllTraps(bool respawn, bool repopnow)
{
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap())
		{
			cur->UpdateTrap(respawn, repopnow);
		}
		++it;
	}

	Log(Logs::General, Logs::Traps, "All traps updated.");
}

void EntityList::GetTrapInfo(Client* client)
{
	uint8 count = 0;
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap())
		{
			bool isset = (cur->chkarea_timer.Enabled() && !cur->reset_timer.Enabled());
			client->Message(Chat::Default, " Trap: (%d) found at %0.2f,%0.2f,%0.2f. Times Triggered: %d Is Active: %d Group: %d Message: %s", cur->trap_id, cur->m_Position.x, cur->m_Position.y, cur->m_Position.z, cur->times_triggered, isset, cur->group, cur->message.c_str());
			++count;
		}
		++it;
	}

	client->Message(Chat::Default, "%d traps found.", count);
}

void EntityList::ClearTrapPointers()
{
	auto it = trap_list.begin();
	while (it != trap_list.end())
	{
		Trap* cur = it->second;
		if (cur->IsTrap())
		{
			cur->DestroyHiddenTrigger();
		}
		++it;
	}
}


bool ZoneDatabase::LoadTraps(const char* zonename, int16 version) {

	std::string query = StringFormat(
		"SELECT id, x, y, z, effect, effectvalue, effectvalue2, skill, "
		"maxzdiff, radius, chance, message, respawn_time, respawn_var, level, "
		"`group`, triggered_number, despawn_when_triggered, undetectable  FROM traps WHERE zone='%s' AND version=%u %s",
		zonename,
		version,
		ContentFilterCriteria::apply().c_str()
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		uint32 tid = atoi(row[0]);
		uint8 grp = atoi(row[15]);

		if (grp > 0)
		{
			// If a member of our group is already spawned skip loading this trap.
			if (entity_list.IsTrapGroupSpawned(tid, grp))
			{
				continue;
			}
		}
		auto trap = new Trap();
		trap->trap_id = tid;
		trap->db_id = tid;
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
		trap->group = grp;
		trap->triggered_number = atoi(row[16]);
		trap->despawn_when_triggered = atobool(row[17]);
		trap->undetectable = atobool(row[18]);
		entity_list.AddTrap(trap);
		trap->CreateHiddenTrigger();
		Log(Logs::General, Logs::Traps, "Trap %d successfully loaded.", trap->trap_id);
	}

	return true;
}

void Trap::CreateHiddenTrigger()
{
	if(hiddenTrigger)
		return;

	const NPCType *base_type = content_db.LoadNPCTypesData(500);
	auto make_npc = new NPCType;
	memcpy(make_npc, base_type, sizeof(NPCType));
	make_npc->max_hp = 100000;
	make_npc->current_hp = 100000;
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
	NPC* npca = new NPC(make_npc, nullptr, glm::vec4(m_Position, 0.0f), GravityBehavior::Flying);
	npca->GiveNPCTypeData(make_npc);
	entity_list.AddNPC(npca);

	hiddenTrigger = npca;
	ownHiddenTrigger = true;
}
bool ZoneDatabase::SetTrapData(Trap* trap, bool repopnow) {

	uint32 dbid = trap->db_id;
	std::string query;

	if (trap->group > 0)
	{
		query = StringFormat("SELECT id, x, y, z, effect, effectvalue, effectvalue2, skill, "
			"maxzdiff, radius, chance, message, respawn_time, respawn_var, level, "
			"triggered_number, despawn_when_triggered, undetectable FROM traps WHERE zone='%s' AND `group`=%d AND id != %d ORDER BY RAND() LIMIT 1", zone->GetShortName(), trap->group, dbid);
	}
	else
	{
		// We could just use the existing data here, but querying the DB is not expensive, and allows content developers to change traps without rebooting.
		query = StringFormat("SELECT id, x, y, z, effect, effectvalue, effectvalue2, skill, "
			"maxzdiff, radius, chance, message, respawn_time, respawn_var, level, "
			"triggered_number, despawn_when_triggered, undetectable FROM traps WHERE zone='%s' AND id = %d", zone->GetShortName(), dbid);
	}

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {

		trap->db_id = atoi(row[0]);
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
		trap->triggered_number = atoi(row[15]);
		trap->despawn_when_triggered = atobool(row[16]);
		trap->undetectable = atobool(row[17]);
		trap->CreateHiddenTrigger();

		if (repopnow)
		{
			trap->chkarea_timer.Enable();
		}
		else
		{
			trap->respawn_timer.Start((trap->respawn_time + zone->random.Int(0, trap->respawn_var)) * 1000);
		}

		if (trap->trap_id != trap->db_id)
			Log(Logs::General, Logs::Traps, "Trap (%d) DBID has changed from %d to %d", trap->trap_id, dbid, trap->db_id);

		return true;
	}

	return false;
}

void Trap::UpdateTrap(bool respawn, bool repopnow)
{
	respawn_timer.Disable();
	chkarea_timer.Disable();
	reset_timer.Disable();
	if (hiddenTrigger)
	{
		hiddenTrigger->Depop();
		hiddenTrigger = nullptr;
	}
	times_triggered = 0;
	Client* trigger = entity_list.GetClientByCharID(charid);
	if (trigger)
	{
		trigger->trapid = 0;
	}
	charid = 0;
	if (respawn)
	{
		content_db.SetTrapData(this, repopnow);
	}
}
