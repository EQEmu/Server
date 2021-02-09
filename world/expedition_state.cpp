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
#include "expedition.h"
#include "expedition_database.h"
#include "worlddb.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/expedition_members_repository.h"
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
			return expedition->GetDynamicZone().GetID() == dz_id;
		});

	return (it != m_expeditions.end()) ? it->get() : nullptr;
}

void ExpeditionState::CacheFromDatabase(uint32_t expedition_id)
{
	if (expedition_id == 0 || GetExpedition(expedition_id))
	{
		return;
	}

	auto expedition = ExpeditionsRepository::GetWithLeaderName(database, expedition_id);
	CacheExpeditions({ std::move(expedition) });
}

void ExpeditionState::CacheAllFromDatabase()
{
	BenchTimer benchmark;

	auto expeditions = ExpeditionsRepository::GetAllWithLeaderName(database);
	m_expeditions.clear();
	m_expeditions.reserve(expeditions.size());

	CacheExpeditions(std::move(expeditions));

	LogExpeditions("Caching [{}] expedition(s) took [{}s]", m_expeditions.size(), benchmark.elapsed());
}

void ExpeditionState::CacheExpeditions(
	std::vector<ExpeditionsRepository::ExpeditionWithLeader>&& expedition_entries)
{
	// bulk load expedition dzs and members before caching
	std::vector<uint32_t> expedition_ids;
	std::vector<uint32_t> dynamic_zone_ids;
	for (const auto& entry : expedition_entries)
	{
		expedition_ids.emplace_back(entry.id);
		dynamic_zone_ids.emplace_back(entry.dynamic_zone_id);
	}

	auto dynamic_zones = DynamicZonesRepository::GetWithInstance(database, dynamic_zone_ids);
	auto expedition_members = ExpeditionMembersRepository::GetWithNames(database, expedition_ids);

	for (auto& entry : expedition_entries)
	{
		auto expedition = std::make_unique<Expedition>();
		expedition->LoadRepositoryResult(std::move(entry));

		auto dz_entry_iter = std::find_if(dynamic_zones.begin(), dynamic_zones.end(),
			[&](const DynamicZonesRepository::DynamicZoneInstance& dz_entry) {
				return dz_entry.id == entry.dynamic_zone_id;
			});

		if (dz_entry_iter != dynamic_zones.end())
		{
			expedition->SetDynamicZone(std::move(*dz_entry_iter));
		}

		for (auto& member : expedition_members)
		{
			if (member.expedition_id == expedition->GetID())
			{
				expedition->AddMemberFromRepositoryResult(std::move(member));
			}
		}

		expedition->CacheMemberStatuses();

		m_expeditions.emplace_back(std::move(expedition));
	}
}

void ExpeditionState::MemberChange(
	uint32_t expedition_id, const DynamicZoneMember& member, bool remove)
{
	auto expedition = GetExpedition(expedition_id);
	if (expedition)
	{
		if (remove) {
			expedition->RemoveMember(member.id);
		} else {
			expedition->AddInternalMember(member);
		}
	}
}

void ExpeditionState::RemoveAllMembers(uint32_t expedition_id)
{
	auto expedition = GetExpedition(expedition_id);
	if (expedition)
	{
		expedition->ClearInternalMembers();
	}
}

void ExpeditionState::Process()
{
	if (!m_process_throttle_timer.Check())
	{
		return;
	}

	std::vector<uint32_t> expedition_ids;

	for (auto it = m_expeditions.begin(); it != m_expeditions.end();)
	{
		bool is_deleted = (*it)->Process();
		if (is_deleted)
		{
			expedition_ids.emplace_back((*it)->GetID());
		}
		it = is_deleted ? m_expeditions.erase(it) : it + 1;
	}

	if (!expedition_ids.empty())
	{
		ExpeditionDatabase::MoveMembersToSafeReturn(expedition_ids);
		ExpeditionDatabase::DeleteExpeditions(expedition_ids);
	}
}
