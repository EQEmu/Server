#include <vector>
#include "zone.h"
#include "../common/repositories/npc_faction_repository.h"
#include "../common/repositories/npc_faction_entries_repository.h"

void Zone::LoadNPCFactions(const std::vector<uint32> &npc_faction_ids)
{
	LogFaction(
		"Load for Faction IDs [{}]",
		Strings::Join(npc_faction_ids, ", ")
	);

	if (npc_faction_ids.empty()) {
		LogFactionDetail("No NPC factions to load.");
		return;
	}

	// Narrow the list sent in, to new npc_faction_ids that are being loaded
	// as the result of a new spawn.  Ignore those already loaded.

	std::vector<uint32> new_npc_faction_ids = { };

	for (const auto& e : npc_faction_ids) {
		bool found = false;

		for (const auto& nf : m_npc_factions) {
			found = (nf.id == e);
			if (found) {
				LogFaction("Already loaded npc_faction [{}]", nf.id);
				break;
			}
		}

		// This one is new
		if (!found) {
			new_npc_faction_ids.emplace_back(e);
		}
	}

	if (new_npc_faction_ids.empty()) {
		LogFactionDetail("No New NPC factions to load.");
		return;
	}

	auto npc_factions = NpcFactionRepository::GetWhere(
		content_db,
		fmt::format(
			"`id` IN ({})",
			Strings::Join(new_npc_faction_ids, ", ")
		)
	);

	auto npc_faction_entries = NpcFactionEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"`npc_faction_id` IN ({})",
			Strings::Join(new_npc_faction_ids, ", ")
		)
	);

	for (const auto& e : npc_factions) {
		m_npc_factions.emplace_back(e);

		for (const auto& f : npc_faction_entries) {
			m_npc_faction_entries.emplace_back(f);
		}
	}

	if (npc_factions.size() > 1) {
		LogFaction("Loaded [{}] Factions", npc_factions.size());
	}
}

void Zone::LoadNPCFaction(const uint32 npc_faction_id)
{
	if (!npc_faction_id) {
		return;
	}

	LogFaction("LoadNPCFaction for [{}]", npc_faction_id);
	LoadNPCFactions({ npc_faction_id });
}

void Zone::ClearNPCFactions()
{
	m_npc_factions.clear();
	m_npc_faction_entries.clear();
}

void Zone::ReloadNPCFactions()
{
	LogFaction("Reloading NPC Factions");

	ClearNPCFactions();

	std::vector<uint32> npc_faction_ids = { };

	for (const auto& n : entity_list.GetNPCList()) {
		if (n.second->GetNPCFactionID() != 0) {
			if (
				std::find(
					npc_faction_ids.begin(),
					npc_faction_ids.end(),
					n.second->GetNPCFactionID()
				) == npc_faction_ids.end()
			) {
				npc_faction_ids.emplace_back(n.second->GetNPCFactionID());
			}
		}
	}

	LoadNPCFactions(npc_faction_ids);
}

NpcFactionRepository::NpcFaction* Zone::GetNPCFaction(const uint32 npc_faction_id)
{
	for (auto& e : m_npc_factions) {
		if (e.id == npc_faction_id) {
			return &e;
		}
	}

	return nullptr;
}

std::vector<NpcFactionEntriesRepository::NpcFactionEntries> Zone::GetNPCFactionEntries(const uint32 npc_faction_id) const
{
	std::vector<NpcFactionEntriesRepository::NpcFactionEntries> npc_faction_entries = { };

	std::vector<uint32> faction_ids;

	for (auto e : m_npc_faction_entries) {
		if (
			e.npc_faction_id == npc_faction_id &&
			std::find(
				faction_ids.begin(),
				faction_ids.end(),
				e.faction_id
			) == faction_ids.end()
		) {
			faction_ids.emplace_back(e.faction_id);
			npc_faction_entries.emplace_back(e);
		}
	}

	return npc_faction_entries;
}

void Zone::LoadNPCFactionAssociations(const std::vector<uint32>& npc_faction_ids)
{
	LogFaction(
		"Load Associations for NPC Faction IDs [{}]",
		Strings::Join(npc_faction_ids, ", ")
	);

	if (npc_faction_ids.empty()) {
		LogFactionDetail("No Faction Associations to load.");
		return;
	}

	std::vector<uint32> faction_ids = { };

	for (const auto& e : npc_faction_ids) {
		for (const auto& f : m_npc_factions) {
			bool found = false;
			if (e == f.id && f.primaryfaction > 0) {
				for (const auto& a : m_faction_associations) {
					if (a.id == f.primaryfaction) {
						found = true;
						LogError("Association [{}] already loaded", a.id);
					break;
					}
				}

				if (!found) {
					faction_ids.emplace_back(f.primaryfaction);
				}
			}
		}
	}

	if (faction_ids.empty()) {
		LogFactionDetail("No New Faction Associations to load.");
		return;
	}

	LogFaction(
		"These are the primary faction IDs [{}]",
		Strings::Join(faction_ids, ", ")
	);

	const auto& faction_associations = FactionAssociationRepository::GetWhere(
		content_db,
		fmt::format(
			"`id` IN ({})",
			Strings::Join(faction_ids, ", ")
		)
	);

	if (faction_associations.empty()) {
		LogFaction(
			"No Faction Association Entries to load for Faction IDs [{}]",
			Strings::Join(faction_ids, ", ")
		);
		return;
	}

	uint32 num_associations= 0;

	for (const auto& e : faction_associations) {
			if (e.id_1) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_1, e.mod_1);
			}
			if (e.id_2) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_2, e.mod_2);
			}
			if (e.id_3) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_3, e.mod_3);
			}
			if (e.id_4) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_4, e.mod_4);
			}
			if (e.id_5) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_5, e.mod_5);
			}
			if (e.id_6) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_6, e.mod_6);
			}
			if (e.id_7) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_7, e.mod_7);
			}
			if (e.id_8) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_8, e.mod_8);
			}
			if (e.id_9) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_9, e.mod_9);
			}
			if (e.id_10) {
				num_associations++;
				LogFaction("Association [{}] -> [{}]", e.id_10, e.mod_10);
			}

			m_faction_associations.emplace_back(e);
		}

	if (num_associations)
			LogFaction("Loaded [{}] Faction Associations.", num_associations);
	}

void Zone::LoadNPCFactionAssociation(const uint32 npc_faction_id)
	{
		if (!npc_faction_id) {
			return;
		}

		LoadNPCFactionAssociations({ npc_faction_id });
	}

void Zone::ClearFactionAssociations()
	{
		m_faction_associations.clear();
	}

void Zone::ReloadFactionAssociations()
	{
		ClearFactionAssociations();

		std::vector<uint32> npc_faction_ids = { };

		for (const auto& n : entity_list.GetNPCList()) {
			if (n.second->GetNPCFactionID() != 0) {
				if (
					std::find(
						npc_faction_ids.begin(),
						npc_faction_ids.end(),
						n.second->GetNPCFactionID()
					) == npc_faction_ids.end()
				) {
					npc_faction_ids.emplace_back(n.second->GetNPCFactionID());
				}
			}
		}

		LogFaction("Reloading Faction Associations");
		LoadNPCFactionAssociations(npc_faction_ids);
	}

FactionAssociationRepository::FactionAssociation* Zone::GetFactionAssociation(const uint32 faction_id)
	{
		for (auto& e : m_faction_associations) {
			if (e.id == faction_id) {
				return &e;
			}
		}

	return nullptr;
}
