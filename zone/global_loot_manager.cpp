#include "global_loot_manager.h"
#include "npc.h"
#include "client.h"

std::vector<int> GlobalLootManager::GetGlobalLootTables(NPC *mob) const
{
	// we may be able to add a cache here if performance is an issue, but for now
	// just return NRVO'd vector
	// The cache would have to be keyed by NPCType and level (for NPCs with Max Level set)
	std::vector<int> tables;

	for (auto &e : m_entries) {
		if (e.PassesRules(mob)) {
			tables.push_back(e.GetLootTableID());
		}
	}

	return tables;
}

void GlobalLootManager::ShowZoneGlobalLoot(Client *to) const
{
	for (auto &e : m_entries)
		to->Message(Chat::White, " %s : %d table %d", e.GetDescription().c_str(), e.GetID(), e.GetLootTableID());
}

void GlobalLootManager::ShowNPCGlobalLoot(Client *to, NPC *who) const
{
	for (auto &e : m_entries) {
		if (e.PassesRules(who))
			to->Message(Chat::White, " %s : %d table %d", e.GetDescription().c_str(), e.GetID(), e.GetLootTableID());
	}
}

bool GlobalLootEntry::PassesRules(NPC *mob) const
{
	bool bRace = false;
	bool bPassesRace = false;
	bool bBodyType = false;
	bool bPassesBodyType = false;
	bool bClass = false;
	bool bPassesClass = false;

	for (auto &r : m_rules) {
		switch (r.type) {
		case GlobalLoot::RuleTypes::LevelMin:
			if (mob->GetLevel() < r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::LevelMax:
			if (mob->GetLevel() > r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::Raid: // value == 0 must not be raid, value != 0 must be raid
			if (mob->IsRaidTarget() && !r.value)
				return false;
			if (!mob->IsRaidTarget() && r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::Rare:
			if (mob->IsRareSpawn() && !r.value)
				return false;
			if (!mob->IsRareSpawn() && r.value)
				return false;
			break;
		case GlobalLoot::RuleTypes::Race: // can have multiple races per rule set
			bRace = true; // we must pass race
			if (mob->GetRace() == r.value)
				bPassesRace = true;
			break;
		case GlobalLoot::RuleTypes::Class: // can have multiple classes per rule set
			bClass = true; // we must pass class
			if (mob->GetClass() == r.value)
				bPassesClass = true;
			break;
		case GlobalLoot::RuleTypes::BodyType: // can have multiple bodytypes per rule set
			bBodyType = true; // we must pass BodyType
			if (mob->GetBodyType() == r.value)
				bPassesBodyType = true;
			break;
		default:
			break;
		}
	}

	if (bRace && !bPassesRace)
		return false;

	if (bClass && !bPassesClass)
		return false;

	if (bBodyType && !bPassesBodyType)
		return false;

	// we abort as early as possible if we fail a rule, so if we get here, we passed
	return true;
}

