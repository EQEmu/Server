/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "expedition_state.h"
#include "dynamic_zone.h"
#include "expedition.h"
#include "expedition_database.h"
#include "worlddb.h"
#include "../common/dynamic_zone_base.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/dynamic_zone_members_repository.h"
#include <algorithm>

ExpeditionState expedition_state;

Expedition* ExpeditionState::GetExpedition(uint32_t expedition_id)
{
	auto it = std::find_if(m_expeditions.begin(), m_expeditions.end(),
		[&](const std::unique_ptr<Expedition>& expedition) {
			return expedition->GetID() == expedition_id;
		});

	return (it != m_expeditions.end()) ? it->get() : nullptr;
}

Expedition* ExpeditionState::GetExpeditionByDynamicZoneID(uint32_t dz_id)
{
	auto it = std::find_if(m_expeditions.begin(), m_expeditions.end(),
		[&](const std::unique_ptr<Expedition>& expedition) {
			return expedition->GetDynamicZoneID() == dz_id;
		});

	return (it != m_expeditions.end()) ? it->get() : nullptr;
}

void ExpeditionState::RemoveExpeditionByID(uint32_t expedition_id)
{
	m_expeditions.erase(std::remove_if(m_expeditions.begin(), m_expeditions.end(),
		[&](const std::unique_ptr<Expedition>& expedition) {
			return expedition->GetID() == expedition_id;
		}
	), m_expeditions.end());
}

void ExpeditionState::CacheFromDatabase(uint32_t expedition_id)
{
	if (expedition_id == 0 || GetExpedition(expedition_id))
	{
		return;
	}

	auto expedition = ExpeditionsRepository::GetWhere(database, fmt::format("id = {}", expedition_id));
	CacheExpeditions({ std::move(expedition) });
}

void ExpeditionState::CacheAllFromDatabase()
{
	BenchTimer benchmark;

	auto expeditions = ExpeditionsRepository::All(database);
	m_expeditions.clear();
	m_expeditions.reserve(expeditions.size());

	CacheExpeditions(std::move(expeditions));

	LogExpeditions("Caching [{}] expedition(s) took [{}s]", m_expeditions.size(), benchmark.elapsed());
}

void ExpeditionState::CacheExpeditions(
	std::vector<ExpeditionsRepository::Expeditions>&& expedition_entries)
{
	for (auto& entry : expedition_entries)
	{
		auto dynamic_zone = DynamicZone::FindDynamicZoneByID(entry.dynamic_zone_id);
		if (!dynamic_zone)
		{
			LogExpeditions("[Warning] Expedition [{}] dz [{}] not found during caching", entry.id, entry.dynamic_zone_id);
			continue;
		}

		// we still need expeditions cached in world for some expedition messages
		auto expedition = std::make_unique<Expedition>(dynamic_zone);
		expedition->LoadRepositoryResult(entry);

		m_expeditions.emplace_back(std::move(expedition));
	}
}
