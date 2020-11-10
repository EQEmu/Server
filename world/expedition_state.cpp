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
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/eqemu_logsys.h"
#include <algorithm>

extern ZSList zoneserver_list;

ExpeditionState expedition_state;

Expedition* ExpeditionState::GetExpedition(uint32_t expedition_id)
{
	auto it = std::find_if(m_expeditions.begin(), m_expeditions.end(),
		[&](const Expedition& expedition) { return expedition.GetID() == expedition_id; });

	return (it != m_expeditions.end()) ? &(*it) : nullptr;
}

void ExpeditionState::LoadActiveExpeditions()
{
	BenchTimer benchmark;

	m_expeditions = ExpeditionDatabase::LoadExpeditions();

	auto elapsed = benchmark.elapsed();
	LogExpeditions("World caching [{}] expeditions took [{}s]", m_expeditions.size(), elapsed);
}

void ExpeditionState::AddExpedition(uint32_t expedition_id)
{
	if (expedition_id == 0)
	{
		return;
	}

	auto expedition = ExpeditionDatabase::LoadExpedition(expedition_id);

	if (expedition.IsValid())
	{
		auto existing_expedition = GetExpedition(expedition_id);
		if (!existing_expedition)
		{
			m_expeditions.emplace_back(expedition);
		}
	}
}

void ExpeditionState::RemoveExpedition(uint32_t expedition_id)
{
	m_expeditions.erase(std::remove_if(m_expeditions.begin(), m_expeditions.end(),
		[&](const Expedition& expedition) {
			return expedition.GetID() == expedition_id;
		}
	), m_expeditions.end());
}

void ExpeditionState::MemberChange(uint32_t expedition_id, uint32_t character_id, bool remove)
{
	auto expedition = GetExpedition(expedition_id);
	if (expedition)
	{
		if (remove) {
			expedition->RemoveMember(character_id);
		} else {
			expedition->AddMember(character_id);
		}
	}
}

void ExpeditionState::RemoveAllMembers(uint32_t expedition_id)
{
	auto expedition = GetExpedition(expedition_id);
	if (expedition)
	{
		expedition->RemoveAllMembers();
	}
}

void ExpeditionState::SetSecondsRemaining(uint32_t expedition_id, uint32_t seconds_remaining)
{
	auto expedition = GetExpedition(expedition_id);
	if (expedition)
	{
		expedition->UpdateDzSecondsRemaining(seconds_remaining);
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
		bool is_deleted = false;

		if (it->IsEmpty() || it->IsExpired())
		{
			// don't delete expedition until its dz instance is empty. this prevents
			// an exploit where all members leave expedition and complete an event
			// before being kicked from removal timer. the lockout could never be
			// applied because the zone expedition cache was already invalidated.
			auto dz_zoneserver = zoneserver_list.FindByInstanceID(it->GetInstanceID());
			if (!dz_zoneserver || dz_zoneserver->NumPlayers() == 0)
			{
				LogExpeditions("Expedition [{}] expired or empty, notifying zones and deleting", it->GetID());
				expedition_ids.emplace_back(it->GetID());
				it->SendZonesExpeditionDeleted();
				is_deleted = true;
			}

			if (it->IsEmpty() && !it->IsPendingDelete() && RuleB(Expedition, EmptyDzShutdownEnabled))
			{
				it->UpdateDzSecondsRemaining(RuleI(Expedition, EmptyDzShutdownDelaySeconds));
			}

			it->SetPendingDelete(true);
		}
		else
		{
			it->CheckExpireWarning();
		}

		it = is_deleted ? m_expeditions.erase(it) : it + 1;
	}

	if (!expedition_ids.empty())
	{
		ExpeditionDatabase::MoveMembersToSafeReturn(expedition_ids);
		ExpeditionDatabase::DeleteExpeditions(expedition_ids);
	}
}
