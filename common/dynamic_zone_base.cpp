#include "dynamic_zone_base.h"
#include "database.h"
#include "eqemu_logsys.h"
#include "repositories/instance_list_repository.h"
#include "repositories/instance_list_player_repository.h"
#include "servertalk.h"

DynamicZoneBase::DynamicZoneBase(DynamicZonesRepository::DynamicZoneInstance&& entry)
{
	LoadRepositoryResult(std::move(entry));
}

uint32_t DynamicZoneBase::Create()
{
	if (m_id != 0)
	{
		return m_id;
	}

	if (GetInstanceID() == 0)
	{
		CreateInstance();
	}

	m_id = SaveToDatabase();

	return m_id;
}

uint32_t DynamicZoneBase::CreateInstance()
{
	if (m_instance_id)
	{
		LogDynamicZones("CreateInstance failed, instance id [{}] already created", m_instance_id);
		return 0;
	}

	if (!m_zone_id)
	{
		LogDynamicZones("CreateInstance failed, invalid zone id [{}]", m_zone_id);
		return 0;
	}

	uint16_t unused_instance_id = 0;
	if (!GetDatabase().GetUnusedInstanceID(unused_instance_id)) // todo: doesn't this race with insert?
	{
		LogDynamicZones("Failed to find unused instance id");
		return 0;
	}

	m_start_time = std::chrono::system_clock::now();
	m_expire_time = m_start_time + m_duration;

	auto insert_instance = InstanceListRepository::NewEntity();
	insert_instance.id = unused_instance_id;
	insert_instance.zone = m_zone_id;
	insert_instance.version = m_zone_version;
	insert_instance.start_time = static_cast<int>(std::chrono::system_clock::to_time_t(m_start_time));
	insert_instance.duration = static_cast<int>(m_duration.count());
	insert_instance.never_expires = m_never_expires;

	auto instance = InstanceListRepository::InsertOne(GetDatabase(), insert_instance);
	if (instance.id == 0)
	{
		LogDynamicZones("Failed to create instance [{}] for zone [{}]", unused_instance_id, m_zone_id);
		return 0;
	}

	m_instance_id = instance.id;

	return m_instance_id;
}

void DynamicZoneBase::LoadRepositoryResult(DynamicZonesRepository::DynamicZoneInstance&& dz_entry)
{
	m_id                 = dz_entry.id;
	m_instance_id        = dz_entry.instance_id;
	m_type               = static_cast<DynamicZoneType>(dz_entry.type);
	m_compass.zone_id    = dz_entry.compass_zone_id;
	m_compass.x          = dz_entry.compass_x;
	m_compass.y          = dz_entry.compass_y;
	m_compass.z          = dz_entry.compass_z;
	m_safereturn.zone_id = dz_entry.safe_return_zone_id;
	m_safereturn.x       = dz_entry.safe_return_x;
	m_safereturn.y       = dz_entry.safe_return_y;
	m_safereturn.z       = dz_entry.safe_return_z;
	m_safereturn.heading = dz_entry.safe_return_heading;
	m_zonein.x           = dz_entry.zone_in_x;
	m_zonein.y           = dz_entry.zone_in_y;
	m_zonein.z           = dz_entry.zone_in_z;
	m_zonein.heading     = dz_entry.zone_in_heading;
	m_has_zonein         = (dz_entry.has_zone_in != 0);
	// instance_list portion
	m_zone_id            = dz_entry.zone;
	m_zone_version       = dz_entry.version;
	m_start_time         = std::chrono::system_clock::from_time_t(dz_entry.start_time);
	m_duration           = std::chrono::seconds(dz_entry.duration);
	m_never_expires      = (dz_entry.never_expires != 0);
	m_expire_time        = m_start_time + m_duration;
}

uint32_t DynamicZoneBase::SaveToDatabase()
{
	LogDynamicZonesDetail("Saving dz instance [{}] to database", m_instance_id);

	if (m_instance_id != 0)
	{
		auto insert_dz = DynamicZonesRepository::NewEntity();
		insert_dz.instance_id         = m_instance_id,
		insert_dz.type                = static_cast<int>(m_type);
		insert_dz.compass_zone_id     = m_compass.zone_id;
		insert_dz.compass_x           = m_compass.x;
		insert_dz.compass_y           = m_compass.y;
		insert_dz.compass_z           = m_compass.z;
		insert_dz.safe_return_zone_id = m_safereturn.zone_id;
		insert_dz.safe_return_x       = m_safereturn.x;
		insert_dz.safe_return_y       = m_safereturn.y;
		insert_dz.safe_return_z       = m_safereturn.z;
		insert_dz.safe_return_heading = m_safereturn.heading;
		insert_dz.zone_in_x           = m_zonein.x;
		insert_dz.zone_in_y           = m_zonein.y;
		insert_dz.zone_in_z           = m_zonein.z;
		insert_dz.zone_in_heading     = m_zonein.heading;
		insert_dz.has_zone_in         = m_has_zonein;

		auto inserted_dz = DynamicZonesRepository::InsertOne(GetDatabase(), insert_dz);
		return inserted_dz.id;
	}
	return 0;
}

void DynamicZoneBase::AddCharacter(uint32_t character_id)
{
	GetDatabase().AddClientToInstance(m_instance_id, character_id);
	SendInstanceAddRemoveCharacter(character_id, false); // stops client kick timer
}

void DynamicZoneBase::RemoveCharacter(uint32_t character_id)
{
	GetDatabase().RemoveClientFromInstance(m_instance_id, character_id);
	SendInstanceAddRemoveCharacter(character_id, true); // start client kick timer
}

void DynamicZoneBase::RemoveAllCharacters(bool enable_removal_timers)
{
	if (GetInstanceID() == 0)
	{
		return;
	}

	if (enable_removal_timers)
	{
		SendInstanceRemoveAllCharacters();
	}

	GetDatabase().RemoveClientsFromInstance(GetInstanceID());
}

void DynamicZoneBase::SaveInstanceMembersToDatabase(const std::vector<uint32_t>& character_ids)
{
	LogDynamicZonesDetail("Saving [{}] members for instance [{}]", character_ids.size(), m_instance_id);

	std::vector<InstanceListPlayerRepository::InstanceListPlayer> insert_players;

	for (const auto& character_id : character_ids)
	{
		InstanceListPlayerRepository::InstanceListPlayer entry{};
		entry.id = static_cast<int>(m_instance_id);
		entry.charid = static_cast<int>(character_id);
		insert_players.emplace_back(entry);
	}

	InstanceListPlayerRepository::InsertMany(GetDatabase(), insert_players);
}

void DynamicZoneBase::SetCompass(const DynamicZoneLocation& location, bool update_db)
{
	ProcessCompassChange(location);

	if (update_db)
	{
		LogDynamicZonesDetail("Saving [{}] compass zone: [{}] xyz: ([{}], [{}], [{}])",
			m_id, m_compass.zone_id, m_compass.x, m_compass.y, m_compass.z);

		DynamicZonesRepository::UpdateCompass(GetDatabase(),
			m_id, m_compass.zone_id, m_compass.x, m_compass.y, m_compass.z);

		SendGlobalLocationChange(ServerOP_DzSetCompass, location);
	}
}

void DynamicZoneBase::SetCompass(uint32_t zone_id, float x, float y, float z, bool update_db)
{
	SetCompass({ zone_id, x, y, z, 0.0f }, update_db);
}

void DynamicZoneBase::SetSafeReturn(const DynamicZoneLocation& location, bool update_db)
{
	m_safereturn = location;

	if (update_db)
	{
		LogDynamicZonesDetail("Saving [{}] safereturn zone: [{}] xyzh: ([{}], [{}], [{}], [{}])",
			m_id, m_safereturn.zone_id, m_safereturn.x, m_safereturn.y, m_safereturn.z, m_safereturn.heading);

		DynamicZonesRepository::UpdateSafeReturn(GetDatabase(), m_id, m_safereturn.zone_id,
			m_safereturn.x, m_safereturn.y, m_safereturn.z, m_safereturn.heading);

		SendGlobalLocationChange(ServerOP_DzSetSafeReturn, location);
	}
}

void DynamicZoneBase::SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db)
{
	SetSafeReturn({ zone_id, x, y, z, heading }, update_db);
}

void DynamicZoneBase::SetZoneInLocation(const DynamicZoneLocation& location, bool update_db)
{
	m_zonein = location;
	m_has_zonein = true;

	if (update_db)
	{
		LogDynamicZonesDetail("Saving [{}] zonein zone: [{}] xyzh: ([{}], [{}], [{}], [{}])",
			m_id, m_zone_id, m_zonein.x, m_zonein.y, m_zonein.z, m_zonein.heading);

		DynamicZonesRepository::UpdateZoneIn(GetDatabase(), m_id, m_zone_id,
			m_zonein.x, m_zonein.y, m_zonein.z, m_zonein.heading, m_has_zonein);

		SendGlobalLocationChange(ServerOP_DzSetZoneIn, location);
	}
}

void DynamicZoneBase::SetZoneInLocation(float x, float y, float z, float heading, bool update_db)
{
	SetZoneInLocation({ 0, x, y, z, heading }, update_db);
}

uint32_t DynamicZoneBase::GetSecondsRemaining() const
{
	auto remaining = std::chrono::duration_cast<std::chrono::seconds>(GetDurationRemaining()).count();
	return std::max(0, static_cast<int>(remaining));
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerAddRemoveCharacterPacket(
	uint32_t character_id, bool removed)
{
	constexpr uint32_t pack_size = sizeof(ServerDzCharacter_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzAddRemoveCharacter, pack_size);
	auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
	buf->zone_id = GetZoneID();
	buf->instance_id = GetInstanceID();
	buf->remove = removed;
	buf->character_id = character_id;

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerRemoveAllCharactersPacket()
{
	constexpr uint32_t pack_size = sizeof(ServerDzCharacter_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzRemoveAllCharacters, pack_size);
	auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
	buf->zone_id = GetZoneID();
	buf->instance_id = GetInstanceID();
	buf->remove = true;
	buf->character_id = 0;

	return pack;
}

std::unique_ptr<ServerPacket> DynamicZoneBase::CreateServerDzLocationPacket(
	uint16_t server_opcode, const DynamicZoneLocation& location)
{
	constexpr uint32_t pack_size = sizeof(ServerDzLocation_Struct);
	auto pack = std::make_unique<ServerPacket>(server_opcode, pack_size);
	auto buf = reinterpret_cast<ServerDzLocation_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->sender_zone_id = GetCurrentZoneID();
	buf->sender_instance_id = GetCurrentInstanceID();
	buf->zone_id = location.zone_id;
	buf->x = location.x;
	buf->y = location.y;
	buf->z = location.z;
	buf->heading = location.heading;

	return pack;
}
