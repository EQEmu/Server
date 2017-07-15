#include "../common/string_util.h"

#include "aura.h"
#include "client.h"
#include "string_ids.h"
#include "raids.h"

Aura::Aura(NPCType *type_data, Mob *owner, AuraRecord &record)
    : NPC(type_data, 0, owner->GetPosition(), FlyMode3), spell_id(record.spell_id), distance(record.distance),
      remove_timer(record.duration), movement_timer(100), process_timer(100)
{
	GiveNPCTypeData(type_data); // we will delete this later on
	m_owner = owner->GetID();

	if (record.cast_time) {
		cast_timer.SetTimer(record.cast_time);
		cast_timer.Disable(); // we don't want to be enabled yet
	}

	if (record.aura_type < static_cast<int>(AuraType::Max))
		type = static_cast<AuraType>(record.aura_type);
	else
		type = AuraType::OnAllGroupMembers;

	if (record.spawn_type < static_cast<int>(AuraSpawns::Max))
		spawn_type = static_cast<AuraSpawns>(record.spawn_type);
	else
		spawn_type = AuraSpawns::GroupMembers;

	if (record.movement < static_cast<int>(AuraMovement::Max))
		movement_type = static_cast<AuraMovement>(record.movement);
	else
		movement_type = AuraMovement::Follow;

	switch (type) {
	case AuraType::OnAllFriendlies:
		process_func = &Aura::ProcessOnAllFriendlies;
		break;
	case AuraType::OnAllGroupMembers:
		process_func = &Aura::ProcessOnAllGroupMembers;
		break;
	case AuraType::OnGroupMembersPets:
		process_func = &Aura::ProcessOnGroupMembersPets;
		break;
	case AuraType::Totem:
		process_func = &Aura::ProcessTotem;
		break;
	case AuraType::EnterTrap:
		process_func = &Aura::ProcessEnterTrap;
		break;
	case AuraType::ExitTrap:
		process_func = &Aura::ProcessExitTrap;
		break;
	default:
		process_func = nullptr;
	}
}

void Aura::ProcessOnAllFriendlies(Mob *owner)
{
	Shout("Stub 1");
}

Mob *Aura::GetOwner()
{
	return entity_list.GetMob(m_owner);
}

void Aura::ProcessOnAllGroupMembers(Mob *owner)
{
	auto &mob_list = entity_list.GetMobList(); // read only reference so we can do it all inline
	std::set<int> delayed_remove;
	bool is_buff = IsBuffSpell(spell_id); // non-buff spells don't cast on enter

	if (owner->IsRaidGrouped() && owner->IsClient()) { // currently raids are just client, but safety check
		auto raid = owner->GetRaid();
		if (raid == nullptr) { // well shit
			Depop();
			return;
		}
		auto group_id = raid->GetGroup(owner->CastToClient());

		// some lambdas so the for loop is less horrible ...
		auto verify_raid_client = [&raid, &group_id, this](Client *c) {
			auto idx = raid->GetPlayerIndex(c);
			if (c->GetID() == m_owner) {
				return DistanceSquared(GetPosition(), c->GetPosition()) <= distance;
			} else if (idx == 0xFFFFFFFF || raid->members[idx].GroupNumber != group_id || raid->members[idx].GroupNumber == 0xFFFFFFFF) {
				return false;
			} else if (DistanceSquared(GetPosition(), c->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		auto verify_raid_client_pet = [&raid, &group_id, this](Mob *m) {
			auto idx = raid->GetPlayerIndex(m->GetOwner()->CastToClient());
			if (m->GetOwner()->GetID() == m_owner) {
				return DistanceSquared(GetPosition(), m->GetPosition()) <= distance;
			} else if (idx == 0xFFFFFFFF || raid->members[idx].GroupNumber != group_id || raid->members[idx].GroupNumber == 0xFFFFFFFF) {
				return false;
			} else if (DistanceSquared(GetPosition(), m->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		auto verify_raid_client_swarm = [&raid, &group_id, this](NPC *n) {
			auto owner = entity_list.GetMob(n->GetSwarmOwner());
			if (owner == nullptr)
				return false;
			auto idx = raid->GetPlayerIndex(owner->CastToClient());
			if (owner->GetID() == m_owner) {
				return DistanceSquared(GetPosition(), n->GetPosition()) <= distance;
			} else if (idx == 0xFFFFFFFF || raid->members[idx].GroupNumber != group_id || raid->members[idx].GroupNumber == 0xFFFFFFFF) {
				return false;
			} else if (DistanceSquared(GetPosition(), n->GetPosition()) > distance) {
				return false;
			}
			return true;
		};

		for (auto &e : mob_list) {
			auto mob = e.second;
			// step 1: check if we're already managing this NPC's buff
			auto it = casted_on.find(mob->GetID());
			if (it != casted_on.end()) {
				// verify still good!
				if (mob->IsClient()) {
					if (!verify_raid_client(mob->CastToClient()))
						delayed_remove.insert(mob->GetID());
				} else if (mob->IsPet() && mob->IsPetOwnerClient() && mob->GetOwner()) {
					if (!verify_raid_client_pet(mob))
						delayed_remove.insert(mob->GetID());
				} else if (mob->IsNPC() && mob->IsPetOwnerClient()) {
					auto npc = mob->CastToNPC();
					if (!verify_raid_client_swarm(npc))
						delayed_remove.insert(mob->GetID());
				}
			} else { // we're not on it!
				if (mob->IsClient() && verify_raid_client(mob->CastToClient())) {
					casted_on.insert(mob->GetID());
					if (is_buff)
						SpellFinished(spell_id, mob);
				} else if (mob->IsPet() && mob->IsPetOwnerClient() && mob->GetOwner() && verify_raid_client_pet(mob)) {
					casted_on.insert(mob->GetID());
					if (is_buff)
						SpellFinished(spell_id, mob);
				} else if (mob->IsNPC() && mob->IsPetOwnerClient()) {
					auto npc = mob->CastToNPC();
					if (verify_raid_client_swarm(npc)) {
						casted_on.insert(mob->GetID());
						if (is_buff)
							SpellFinished(spell_id, mob);
					}
				}
			}
		}
	} else if (owner->IsGrouped()) {
		auto group = owner->GetGroup();
		if (group == nullptr) { // uh oh
			Depop();
			return;
		}

		// lambdas to make for loop less ugly
		auto verify_group_pet = [&group, this](Mob *m) {
			auto owner = m->GetOwner();
			if (owner != nullptr && group->IsGroupMember(owner) && DistanceSquared(GetPosition(), m->GetPosition()) <= distance)
				return true;
			return false;
		};

		auto verify_group_swarm = [&group, this](NPC *n) {
			auto owner = entity_list.GetMob(n->GetSwarmOwner());
			if (owner != nullptr && group->IsGroupMember(owner) && DistanceSquared(GetPosition(), n->GetPosition()) <= distance)
				return true;
			return false;
		};

		for (auto &e : mob_list) {
			auto mob = e.second;
			auto it = casted_on.find(mob->GetID());

			if (it != casted_on.end()) { // make sure we're still valid
				if (mob->IsPet()) {
					if (!verify_group_pet(mob))
						delayed_remove.insert(mob->GetID());
				} else if (mob->IsNPC() && mob->CastToNPC()->GetSwarmInfo()) {
					if (!verify_group_swarm(mob->CastToNPC()))
						delayed_remove.insert(mob->GetID());
				} else if (!group->IsGroupMember(mob) || DistanceSquared(GetPosition(), mob->GetPosition()) > distance) {
					delayed_remove.insert(mob->GetID());
				}
			} else { // not on, check if we should be!
				if (mob->IsPet() && verify_group_pet(mob)) {
					casted_on.insert(mob->GetID());
					if (is_buff)
						SpellFinished(spell_id, mob);
				} else if (mob->IsNPC() && mob->CastToNPC()->GetSwarmInfo() && verify_group_swarm(mob->CastToNPC())) {
					casted_on.insert(mob->GetID());
					if (is_buff)
						SpellFinished(spell_id, mob);
				} else if (group->IsGroupMember(mob) && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
					casted_on.insert(mob->GetID());
					if (is_buff)
						SpellFinished(spell_id, mob);
				}
			}
		}
	} else {
		auto verify_solo = [&owner, this](Mob *m) {
			if (m->IsPet() && m->GetOwnerID() == owner->GetID())
				return true;
			else if (m->IsNPC() && m->CastToNPC()->GetSwarmOwner() == owner->GetID())
				return true;
			else if (m->GetID() == owner->GetID())
				return true;
			else
				return false;
		};
		for (auto &e : mob_list) {
			auto mob = e.second;
			auto it = casted_on.find(mob->GetID());
			bool good = verify_solo(mob);

			if (it != casted_on.end()) { // make sure still valid
				if (!good || DistanceSquared(GetPosition(), mob->GetPosition()) > distance) {
					delayed_remove.insert(mob->GetID());
				}
			} else if (good && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
				casted_on.insert(mob->GetID());
				if (is_buff)
					SpellFinished(spell_id, mob);
			}
		}
	}

	for (auto &e : delayed_remove) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr && is_buff) // some auras cast instant spells so no need to remove
			mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		casted_on.erase(e);
	}

	// so if we have a cast timer and our set isn't empty and timer is disabled we need to enable it
	if (cast_timer.GetDuration() > 0 && !cast_timer.Enabled() && !casted_on.empty())
		cast_timer.Start();

	if (!cast_timer.Enabled() || !cast_timer.Check())
		return;

	// some auras have to recast (DRU for example, non-buff too)
	for (auto &e : casted_on) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr)
			SpellFinished(spell_id, mob);
	}
}

void Aura::ProcessOnGroupMembersPets(Mob *owner)
{
	Shout("Stub 3");
}

void Aura::ProcessTotem(Mob *owner)
{
	auto &mob_list = entity_list.GetMobList(); // read only reference so we can do it all inline
	std::set<int> delayed_remove;
	bool is_buff = IsBuffSpell(spell_id); // non-buff spells don't cast on enter

	for (auto &e : mob_list) {
		auto mob = e.second;
		if (mob == this)
			continue;
		if (mob == owner)
			continue;
		if (owner->IsAttackAllowed(mob)) { // might need more checks ...
			bool in_range = DistanceSquared(GetPosition(), mob->GetPosition()) <= distance;
			auto it = casted_on.find(mob->GetID());
			if (it != casted_on.end()) {
				if (!in_range)
					delayed_remove.insert(mob->GetID());
			} else if (in_range) {
				casted_on.insert(mob->GetID());
				SpellFinished(spell_id, mob);
			}
		}
	}

	for (auto &e : delayed_remove) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr && is_buff) // some auras cast instant spells so no need to remove
			mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		casted_on.erase(e);
	}

	// so if we have a cast timer and our set isn't empty and timer is disabled we need to enable it
	if (cast_timer.GetDuration() > 0 && !cast_timer.Enabled() && !casted_on.empty())
		cast_timer.Start();

	if (!cast_timer.Enabled() || !cast_timer.Check())
		return;

	for (auto &e : casted_on) {
		auto mob = entity_list.GetMob(e);
		if (mob != nullptr)
			SpellFinished(spell_id, mob);
	}
}

void Aura::ProcessEnterTrap(Mob *owner)
{
	auto &mob_list = entity_list.GetMobList(); // read only reference so we can do it all inline

	for (auto &e : mob_list) {
		auto mob = e.second;
		if (mob == this)
			continue;
		// might need more checks ...
		if (owner->IsAttackAllowed(mob) && DistanceSquared(GetPosition(), mob->GetPosition()) <= distance) {
			SpellFinished(spell_id, mob);
			Depop(); // if we're a buff (ex. NEC) we don't want to strip :P
			break;
		}
	}
}

void Aura::ProcessExitTrap(Mob *owner)
{
	Shout("Stub 6");
}

bool Aura::Process()
{
	// Aura::Depop clears buffs
	if (p_depop)
		return false;

	auto owner = entity_list.GetMob(m_owner);
	if (owner == nullptr) {
		Depop();
		return true;
	}

	if (remove_timer.Check()) {
		Depop();
		return true;
	}

	if (movement_type == AuraMovement::Follow && GetPosition() != owner->GetPosition() && movement_timer.Check()) {
		m_Position = owner->GetPosition();
		SendPositionUpdate();
	}
	// TODO: waypoints?

	if (!process_timer.Check())
		return true;

	if (process_func)
		process_func(*this, owner);

	// TODO: quest calls
	return true;
}

void Aura::Depop(bool unused)
{
	if (IsBuffSpell(spell_id)) {
		for (auto &e : casted_on) {
			auto mob = entity_list.GetMob(e);
			if (mob != nullptr)
				mob->BuffFadeBySpellIDAndCaster(spell_id, GetID());
		}
	}
	casted_on.clear();
	p_depop = true;
}

void Mob::MakeAura(uint16 spell_id)
{
	// TODO: verify room in AuraMgr
	if (!IsValidSpell(spell_id))
		return;

	AuraRecord record;
	if (!database.GetAuraEntry(spell_id, record)) {
		Message(13, "Unable to find data for aura %s", spells[spell_id].name);
		Log(Logs::General, Logs::Error, "Unable to find data for aura %d, check auras table.", spell_id);
		return;
	}

	if (!IsValidSpell(record.spell_id)) {
		Message(13, "Casted spell (%d) is not valid for aura %s", record.spell_id, spells[spell_id].name);
		Log(Logs::General, Logs::Error, "Casted spell (%d) is not valid for aura %d, check auras table.",
		    record.spell_id, spell_id);
		return;
	}

	if (record.aura_type > static_cast<int>(AuraType::Max)) {
		return; // TODO: log
	}

	bool trap = false;

	switch (static_cast<AuraType>(record.aura_type)) {
	case AuraType::ExitTrap:
	case AuraType::EnterTrap:
	case AuraType::Totem:
		trap = true;
		break;
	default:
		trap = false;
		break;
	}

	if (!CanSpawnAura(trap))
		return;

	const auto base = database.LoadNPCTypesData(record.npc_type);
	if (base == nullptr) {
		Message(13, "Unable to load NPC data for aura %s", spells[spell_id].teleport_zone);
		Log(Logs::General, Logs::Error,
		    "Unable to load NPC data for aura %s (NPC ID %d), check auras and npc_types tables.",
		    spells[spell_id].teleport_zone, record.npc_type);
		return;
	}

	auto npc_type = new NPCType;
	memcpy(npc_type, base, sizeof(NPCType));

	strn0cpy(npc_type->name, record.name, 64);

	auto npc = new Aura(npc_type, this, record);
	entity_list.AddNPC(npc, true, true);

	if (trap)
		AddTrap(npc, record);
	else
		AddAura(npc, record);
}

bool ZoneDatabase::GetAuraEntry(uint16 spell_id, AuraRecord &record)
{
	auto query = StringFormat("SELECT npc_type, name, spell_id, distance, aura_type, spawn_type, movement, "
				  "duration, icon, cast_time FROM auras WHERE type='%d'",
				  spell_id);

	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	if (results.RowCount() != 1)
		return false;

	auto row = results.begin();

	record.npc_type = atoi(row[0]);
	strn0cpy(record.name, row[1], 64);
	record.spell_id = atoi(row[2]);
	record.distance = atoi(row[3]);
	record.distance *= record.distance; // so we can avoid sqrt
	record.aura_type = atoi(row[4]);
	record.spawn_type = atoi(row[5]);
	record.movement = atoi(row[6]);
	record.duration = atoi(row[7]) * 1000; // DB is in seconds
	record.icon = atoi(row[8]);
	record.cast_time = atoi(row[9]) * 1000; // DB is in seconds

	return true;
}

void Mob::AddAura(Aura *aura, AuraRecord &record)
{
	// this is called only when it's safe
	assert(aura != nullptr);
	strn0cpy(aura_mgr.auras[aura_mgr.count].name, aura->GetCleanName(), 64);
	aura_mgr.auras[aura_mgr.count].spawn_id = aura->GetID();
	if (record.icon == -1)
		aura_mgr.auras[aura_mgr.count].icon = spells[record.spell_id].new_icon;
	else
		aura_mgr.auras[aura_mgr.count].icon = record.icon;
	if (IsClient()) {
		auto outapp = new EQApplicationPacket(OP_UpdateAura, sizeof(AuraCreate_Struct));
		auto aura_create = (AuraCreate_Struct *)outapp->pBuffer;
		aura_create->action = 0;
		aura_create->type = 1; // this can be 0 sometimes too
		strn0cpy(aura_create->aura_name, aura_mgr.auras[aura_mgr.count].name, 64);
		aura_create->entity_id = aura_mgr.auras[aura_mgr.count].spawn_id;
		aura_create->icon = aura_mgr.auras[aura_mgr.count].icon;
		CastToClient()->FastQueuePacket(&outapp);
	}
	// we can increment this now
	aura_mgr.count++;
}

void Mob::AddTrap(Aura *aura, AuraRecord &record)
{
	// this is called only when it's safe
	assert(aura != nullptr);
	strn0cpy(trap_mgr.auras[trap_mgr.count].name, aura->GetCleanName(), 64);
	trap_mgr.auras[trap_mgr.count].spawn_id = aura->GetID();
	if (record.icon == -1)
		trap_mgr.auras[trap_mgr.count].icon = spells[record.spell_id].new_icon;
	else
		trap_mgr.auras[trap_mgr.count].icon = record.icon;
	// doesn't send to client
	trap_mgr.count++;
}

bool Mob::CanSpawnAura(bool trap)
{
	if (trap && !HasFreeTrapSlots()) {
		Message_StringID(MT_SpellFailure, NO_MORE_TRAPS);
		return false;
	} else if (!trap && !HasFreeAuraSlots()) {
		Message_StringID(MT_SpellFailure, NO_MORE_AURAS);
		return false;
	}

	return true;
}

