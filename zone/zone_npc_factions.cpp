#include <vector>
#include "zone.h"
#include "../common/repositories/npc_faction_repository.h"
#include "../common/repositories/npc_faction_entries_repository.h"

void Zone::LoadNPCFactions(const std::vector<uint32> &faction_ids)
{
	for (const auto& e : faction_ids) {
		for (const auto& f : m_npc_factions) {
			if (e == f.id) {
				LogFactionDetail("Faction [{}] already loaded.", e);
				return;
			}
		}
	}

	if (faction_ids.empty()) {
		LogFactionDetail("No NPC factions to load.");
		return;
	}

	std::vector<uint32> npc_faction_ids;

	auto npc_factions = NpcFactionRepository::GetWhere(
		content_db,
		fmt::format(
			"`id` IN ({})",
			Strings::Join(faction_ids, ", ")
		)
	);

	for (const auto& e : npc_factions) {
		if (
			std::find(
				npc_faction_ids.begin(),
				npc_faction_ids.end(),
				e.id
			) == npc_faction_ids.end()
		) {
			npc_faction_ids.emplace_back(e.id);
		}
	}

	if (npc_faction_ids.empty()) {
		LogFaction(
			"No NPC Faction IDs to load for Faction IDs [{}]",
			Strings::Join(faction_ids, ", ")
		);
		return;
	}

	auto npc_faction_entries = NpcFactionEntriesRepository::GetWhere(
		content_db,
		fmt::format(
			"`npc_faction_id` IN ({})",
			Strings::Join(npc_faction_ids, ", ")
		)
	);

	for (const auto& e : npc_factions) {
		bool has_faction = false;

		for (const auto& f : m_npc_factions) {
			if (e.primaryfaction == f.primaryfaction) {
				has_faction = true;
				break;
			}
		}

		if (!has_faction) {
			m_npc_factions.emplace_back(e);

			for (const auto& f : npc_faction_entries) {
				m_npc_faction_entries.emplace_back(f);
			}
		}
	}

	if (faction_ids.size() > 1) {
		LogFaction("Loaded [{}] Factions", faction_ids.size());
	}
}

void Zone::LoadNPCFaction(const uint32 faction_id)
{
	if (!faction_id) {
		return;
	}

	LoadNPCFactions({ faction_id });
}

void Zone::ClearNPCFactions()
{
	m_npc_factions.clear();
	m_npc_faction_entries.clear();
}

void Zone::ReloadNPCFactions()
{
	ClearNPCFactions();

	std::vector<uint32> faction_ids = { };

	for (const auto& n : entity_list.GetNPCList()) {
		if (n.second->GetNPCFactionID() != 0) {
			if (
				std::find(
					faction_ids.begin(),
					faction_ids.end(),
					n.second->GetNPCFactionID()
				) == faction_ids.end()
			) {
				faction_ids.emplace_back(n.second->GetNPCFactionID());
			}
		}
	}

	LoadNPCFactions(faction_ids);
}

NpcFactionRepository::NpcFaction* Zone::GetNPCFaction(const uint32 faction_id)
{
	for (auto& e : m_npc_factions) {
		if (e.primaryfaction == faction_id) {
			return &e;
		}
	}

	return nullptr;
}

std::vector<NpcFactionEntriesRepository::NpcFactionEntries> Zone::GetNPCFactionEntries(const uint32 faction_id) const
{
	std::vector<NpcFactionEntriesRepository::NpcFactionEntries> npc_faction_entries = { };

	for (const auto& e : m_npc_faction_entries) {
		if (e.npc_faction_id == faction_id) {
			npc_faction_entries.emplace_back(e);
		}
	}

	return npc_faction_entries;
}

void Zone::LoadFactionAssociations(const std::vector<uint32>& faction_ids)
{
	for (const auto& e : faction_ids) {
		for (const auto& f : m_faction_associations) {
			if (f.id_1 == e) {
				LogFaction("Faction Association for Faction ID [{}] already loaded.", e);
				return;
			}
		}
	}

	if (faction_ids.empty()) {
		LogFactionDetail("No Faction Associations to load.");
		return;
	}

	const auto& faction_associations = FactionAssociationRepository::GetWhere(
		content_db,
		fmt::format(
			"`id_1` IN ({})",
			Strings::Join(faction_ids, ", ")
		)
	);

	std::vector<uint32> faction_association_ids;

	for (const auto& e : faction_associations) {
		if (
			std::find(
				faction_association_ids.begin(),
				faction_association_ids.end(),
				e.id_1
			) == faction_association_ids.end()
		) {
			faction_association_ids.emplace_back(e.id_1);
		}
	}

	if (faction_association_ids.empty()) {
		LogFaction(
			"No Faction Association Entries to load for Faction IDs [{}]",
			Strings::Join(faction_ids, ", ")
		);
		return;
	}

	for (const auto& e : faction_associations) {
		bool has_association = false;

		for (const auto& f : m_faction_associations) {
			if (e.id_1 == f.id_1) {
				has_association = true;
				break;
			}
		}

		if (!has_association) {
			m_faction_associations.emplace_back(e);
		}
	}

	if (faction_ids.size() > 1) {
		LogFaction("Loaded [{}] Faction Associations.", faction_ids.size());
	}
}

void Zone::LoadFactionAssociation(const uint32 faction_id)
{
	if (!faction_id) {
		return;
	}

	LoadFactionAssociations({ faction_id });
}

void Zone::ClearFactionAssociations()
{
	m_faction_associations.clear();
}

void Zone::ReloadFactionAssociations()
{
	ClearFactionAssociations();

	std::vector<uint32> faction_ids = { };

	for (const auto& n : entity_list.GetNPCList()) {
		if (n.second->GetNPCFactionID() != 0) {
			if (
				std::find(
					faction_ids.begin(),
					faction_ids.end(),
					n.second->GetNPCFactionID()
				) == faction_ids.end()
			) {
				faction_ids.emplace_back(n.second->GetNPCFactionID());
			}
		}
	}

	LoadFactionAssociations(faction_ids);
}

FactionAssociationRepository::FactionAssociation* Zone::GetFactionAssociation(const uint32 faction_id)
{
	for (auto& e : m_faction_associations) {
		if (e.id_1 == faction_id) {
			return &e;
		}
	}

	return nullptr;
}
