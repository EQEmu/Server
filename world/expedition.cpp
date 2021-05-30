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

#include "expedition.h"
#include "dynamic_zone.h"
#include "zonelist.h"
#include "zoneserver.h"

extern ZSList zoneserver_list;

Expedition::Expedition(DynamicZone* dz) :
	m_dynamic_zone(dz)
{
	assert(m_dynamic_zone != nullptr); // dz must remain valid for lifetime of expedition
}

void Expedition::SendZonesExpeditionDeleted()
{
	uint32_t pack_size = sizeof(ServerExpeditionID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionDeleted, pack_size);
	auto buf = reinterpret_cast<ServerExpeditionID_Struct*>(pack->pBuffer);
	buf->expedition_id = GetID();
	zoneserver_list.SendPacket(pack.get());
}
