#include <vector>
#include "zone.h"
#include "../common/repositories/loottable_repository.h"
#include "../common/repositories/loottable_entries_repository.h"
#include "../common/repositories/lootdrop_repository.h"
#include "../common/repositories/lootdrop_entries_repository.h"

void Zone::LoadLootTables(const std::vector<uint32> &loottable_ids)
{
	// check if table is already loaded
	for (const auto &e: loottable_ids) {
		for (const auto &f: m_loottables) {
			if (e == f.id) {
				LogLoot("Loottable [{}] already loaded", e);
				return;
			}
		}
	}

	if (loottable_ids.empty()) {
		LogLoot("No loottables to load");
		return;
	}

	auto loottables = LoottableRepository::GetWhere(
		database,
		fmt::format(
			"id IN ({})",
			Strings::Join(loottable_ids, ",")
		)
	);

	auto loottable_entries = LoottableEntriesRepository::GetWhere(
		database,
		fmt::format(
			"loottable_id IN ({})",
			Strings::Join(loottable_ids, ",")
		)
	);

	std::vector<uint32> lootdrop_ids;
	for (const auto     &e: loottable_entries) {
		if (std::find(
			lootdrop_ids.begin(),
			lootdrop_ids.end(),
			e.lootdrop_id
		) == lootdrop_ids.end()) {
			lootdrop_ids.push_back(e.lootdrop_id);
		}
	}

	if (lootdrop_ids.empty()) {
		LogLoot("No lootdrops to load");
		return;
	}

	auto lootdrops = LootdropRepository::GetWhere(
		database,
		fmt::format(
			"id IN ({})",
			Strings::Join(lootdrop_ids, ",")
		)
	);

	auto lootdrop_entries = LootdropEntriesRepository::GetWhere(
		database,
		fmt::format(
			"lootdrop_id IN ({})",
			Strings::Join(lootdrop_ids, ",")
		)
	);

	// emplace back tables to m_loottables if not exists
	for (const auto &e: loottables) {
		bool has_table = false;

		for (const auto &l: m_loottables) {
			if (e.id == l.id) {
				has_table = true;
				break;
			}
		}
		if (!has_table) {
			// add loottable
			m_loottables.emplace_back(e);

			// add loottable entries
			for (const auto &f: loottable_entries) {
				if (e.id == f.loottable_id) {
					m_loottable_entries.emplace_back(f);

					// add lootdrop
					for (const auto &g: lootdrops) {
						if (f.lootdrop_id == g.id) {
							m_lootdrops.emplace_back(g);

							// add lootdrop entries
							for (const auto &h: lootdrop_entries) {
								if (g.id == h.lootdrop_id) {
									m_lootdrop_entries.emplace_back(h);
								}
							}
						}
					}
				}
			}
		}
	}

	if (loottable_ids.size() > 1) {
		LogLoot("Loaded [{}] loottables", m_loottables.size());
	}
}

void Zone::LoadLootTable(const uint32 loottable_id)
{
	LoadLootTables({loottable_id});
}

void Zone::ClearLootTables()
{
	m_loottables.clear();
	m_loottable_entries.clear();
	m_lootdrops.clear();
	m_lootdrop_entries.clear();
}

LoottableRepository::Loottable *Zone::GetLootTable(const uint32 loottable_id)
{
	for (auto &e: m_loottables) {
		if (e.id == loottable_id) {
			return &e;
		}
	}

	return nullptr;
}

std::vector<LoottableEntriesRepository::LoottableEntries> Zone::GetLootTableEntries(const uint32 loottable_id) const
{
	std::vector<LoottableEntriesRepository::LoottableEntries> entries = {};
	for (const auto &e: m_loottable_entries) {
		if (e.loottable_id == loottable_id) {
			entries.emplace_back(e);
		}
	}

	return entries;
}

LootdropRepository::Lootdrop Zone::GetLootdrop(const uint32 lootdrop_id) const
{
	for (const auto &e: m_lootdrops) {
		if (e.id == lootdrop_id) {
			return e;
		}
	}

	return {};
}

std::vector<LootdropEntriesRepository::LootdropEntries> Zone::GetLootdropEntries(const uint32 lootdrop_id) const
{
	std::vector<LootdropEntriesRepository::LootdropEntries> entries = {};
	for (const auto &e: m_lootdrop_entries) {
		if (e.lootdrop_id == lootdrop_id) {
			entries.emplace_back(e);
		}
	}

	return entries;
}
