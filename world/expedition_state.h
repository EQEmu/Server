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

#ifndef WORLD_EXPEDITION_STATE_H
#define WORLD_EXPEDITION_STATE_H

#include "../common/rulesys.h"
#include "../common/timer.h"
#include <cstdint>
#include <vector>

extern class ExpeditionState expedition_state;

class Expedition;

class ExpeditionState
{
public:
	void AddExpedition(uint32_t expedition_id);
	Expedition* GetExpedition(uint32_t expedition_id);
	Expedition* GetExpeditionByDynamicZoneID(uint32_t dz_id);
	void LoadActiveExpeditions();
	void MemberChange(uint32_t expedition_id, uint32_t character_id, bool remove);
	void Process();
	void RemoveAllMembers(uint32_t expedition_id);
	void RemoveExpedition(uint32_t expedition_id);

private:
	std::vector<Expedition> m_expeditions;
	Timer m_process_throttle_timer{static_cast<uint32_t>(RuleI(DynamicZone, WorldProcessRate))};
};

#endif
