#include "dynamic_zone.h"
#include "expedition.h"
#include "expedition_state.h"
#include "worlddb.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/instance_list_repository.h"

extern ZSList zoneserver_list;

Database& DynamicZone::GetDatabase()
{
	return database;
}

DynamicZone* DynamicZone::FindDynamicZoneByID(uint32_t dz_id)
{
	auto expedition = expedition_state.GetExpeditionByDynamicZoneID(dz_id);
	if (expedition)
	{
		return &expedition->GetDynamicZone();
	}
	// todo: other system caches
	return nullptr;
}

DynamicZoneStatus DynamicZone::Process()
{
	DynamicZoneStatus status = DynamicZoneStatus::Normal;

	// force expire if no members
	if (!HasMembers() || IsExpired())
	{
		status = DynamicZoneStatus::Expired;

		auto dz_zoneserver = zoneserver_list.FindByInstanceID(GetInstanceID());
		if (!dz_zoneserver || dz_zoneserver->NumPlayers() == 0) // no clients inside dz
		{
			status = DynamicZoneStatus::ExpiredEmpty;

			if (!HasMembers() && !m_is_pending_early_shutdown && RuleB(DynamicZone, EmptyShutdownEnabled))
			{
				SetSecondsRemaining(RuleI(DynamicZone, EmptyShutdownDelaySeconds));
				m_is_pending_early_shutdown = true;
			}
		}
	}

	return status;
}

void DynamicZone::SetSecondsRemaining(uint32_t seconds_remaining)
{
	auto now = std::chrono::system_clock::now();
	auto new_remaining = std::chrono::seconds(seconds_remaining);

	auto current_remaining = m_expire_time - now;
	if (current_remaining > new_remaining) // reduce only
	{
		LogDynamicZonesDetail("Updating dynamic zone [{}] instance [{}] seconds remaining to [{}]s",
			GetID(), GetInstanceID(), seconds_remaining);

		// preserve original start time and adjust duration instead
		m_expire_time = now + new_remaining;
		m_duration = std::chrono::duration_cast<std::chrono::seconds>(m_expire_time - m_start_time);

		InstanceListRepository::UpdateDuration(database,
			GetInstanceID(), static_cast<uint32_t>(m_duration.count()));

		SendZonesDurationUpdate(); // update zone caches and actual instance's timer
	}
}

void DynamicZone::SendZonesDurationUpdate()
{
	constexpr uint32_t packsize = sizeof(ServerDzSetDuration_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzDurationUpdate, packsize);
	auto packbuf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
	packbuf->dz_id = GetID();
	packbuf->seconds = static_cast<uint32_t>(m_duration.count());
	zoneserver_list.SendPacket(pack.get());
}

void DynamicZone::HandleZoneMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzSetCompass:
	case ServerOP_DzSetSafeReturn:
	case ServerOP_DzSetZoneIn:
	{
		auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			if (pack->opcode == ServerOP_DzSetCompass)
			{
				dz->SetCompass(buf->zone_id, buf->x, buf->y, buf->z, false);
			}
			else if (pack->opcode == ServerOP_DzSetSafeReturn)
			{
				dz->SetSafeReturn(buf->zone_id, buf->x, buf->y, buf->z, buf->heading, false);
			}
			else if (pack->opcode == ServerOP_DzSetZoneIn)
			{
				dz->SetZoneInLocation(buf->x, buf->y, buf->z, buf->heading, false);
			}
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzAddRemoveCharacter:
	case ServerOP_DzRemoveAllCharacters:
	{
		auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
		ZoneServer* instance_zs = zoneserver_list.FindByInstanceID(buf->instance_id);
		if (instance_zs)
		{
			instance_zs->SendPacket(pack);
		}
		break;
	}
	case ServerOP_DzSetSecondsRemaining:
	{
		auto buf = reinterpret_cast<ServerDzSetDuration_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->SetSecondsRemaining(buf->seconds);
		}
		break;
	}
	};
}

void DynamicZone::SendInstanceAddRemoveCharacter(uint32_t character_id, bool remove)
{
	ZoneServer* instance_zs = zoneserver_list.FindByInstanceID(GetInstanceID());
	if (instance_zs)
	{
		auto pack = CreateServerAddRemoveCharacterPacket(character_id, remove);
		instance_zs->SendPacket(pack.get());
	}
}

void DynamicZone::SendInstanceRemoveAllCharacters()
{
	ZoneServer* instance_zs = zoneserver_list.FindByInstanceID(GetInstanceID());
	if (instance_zs)
	{
		auto pack = CreateServerRemoveAllCharactersPacket();
		instance_zs->SendPacket(pack.get());
	}
}

void DynamicZone::SendGlobalLocationChange(uint16_t server_opcode, const DynamicZoneLocation& location)
{
	auto pack = CreateServerDzLocationPacket(server_opcode, location);
	zoneserver_list.SendPacket(pack.get());
}
