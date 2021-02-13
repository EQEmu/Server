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

#ifndef DYNAMIC_ZONE_H
#define DYNAMIC_ZONE_H

#include "../common/dynamic_zone_base.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Database;
class ServerPacket;

extern const char* const CREATE_NOT_ALL_ADDED;

class DynamicZone : public DynamicZoneBase
{
public:
	using DynamicZoneBase::DynamicZoneBase; // inherit base constructors

	DynamicZone() = default;
	DynamicZone(uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type);

	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id);
	static void HandleWorldMessage(ServerPacket* pack);

	void SetSecondsRemaining(uint32_t seconds_remaining) override;

	bool IsCurrentZoneDzInstance() const;
	void SetUpdatedDuration(uint32_t seconds);

protected:
	uint16_t GetCurrentInstanceID() override;
	uint16_t GetCurrentZoneID() override;
	Database& GetDatabase() override;
	void ProcessCompassChange(const DynamicZoneLocation& location) override;
	void SendInstanceAddRemoveCharacter(uint32_t character_id, bool remove) override;
	void SendInstanceRemoveAllCharacters() override;
	void SendGlobalLocationChange(uint16_t server_opcode, const DynamicZoneLocation& location) override;

private:
	static void StartAllClientRemovalTimers();
	void SendCompassUpdateToZoneMembers();
};

#endif
