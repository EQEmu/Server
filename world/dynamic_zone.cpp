#include "dynamic_zone.h"
#include "cliententry.h"
#include "clientlist.h"
#include "dynamic_zone_manager.h"
#include "worlddb.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/instance_list_repository.h"

extern ClientList client_list;
extern ZSList zoneserver_list;

Database& DynamicZone::GetDatabase()
{
	return database;
}

bool DynamicZone::SendServerPacket(ServerPacket* packet)
{
	return zoneserver_list.SendPacket(packet);
}

DynamicZone* DynamicZone::FindDynamicZoneByID(uint32_t dz_id)
{
	auto dz = dynamic_zone_manager.dynamic_zone_cache.find(dz_id);
	if (dz != dynamic_zone_manager.dynamic_zone_cache.end())
	{
		return dz->second.get();
	}
	return nullptr;
}

void DynamicZone::ChooseNewLeader()
{
	if (m_members.empty() || !m_choose_leader_cooldown_timer.Check())
	{
		m_choose_leader_needed = true;
		return;
	}

	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const DynamicZoneMember& member) {
		if (member.id != GetLeaderID() && member.IsOnline()) {
			auto member_cle = client_list.FindCLEByCharacterID(member.id);
			return (member_cle && member_cle->GetOnline() == CLE_Status::InZone);
		}
		return false;
	});

	if (it == m_members.end())
	{
		// no online members found, fallback to choosing any member
		it = std::find_if(m_members.begin(), m_members.end(),
			[&](const DynamicZoneMember& member) { return member.id != GetLeaderID(); });
	}

	if (it != m_members.end() && SetNewLeader(it->id))
	{
		m_choose_leader_needed = false;
	}
}

bool DynamicZone::SetNewLeader(uint32_t member_id)
{
	auto new_leader = GetMemberData(member_id);
	if (!new_leader.IsValid())
	{
		return false;
	}

	LogDynamicZonesDetail("Replacing dz [{}] leader [{}] with [{}]", GetID(), GetLeaderName(), new_leader.name);
	SetLeader(new_leader, true);
	SendZonesLeaderChanged();
	return true;
}

void DynamicZone::CheckLeader()
{
	if (m_choose_leader_needed)
	{
		ChooseNewLeader();
	}
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

	if (GetType() == DynamicZoneType::Expedition && status != DynamicZoneStatus::ExpiredEmpty)
	{
		CheckExpireWarning();
		CheckLeader();
	}

	return status;
}

void DynamicZone::SendZonesDynamicZoneDeleted()
{
	uint32_t pack_size = sizeof(ServerDzID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzDeleted, pack_size);
	auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	zoneserver_list.SendPacket(pack.get());
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

void DynamicZone::SendZonesLeaderChanged()
{
	uint32_t pack_size = sizeof(ServerDzLeaderID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzLeaderChanged, pack_size);
	auto buf = reinterpret_cast<ServerDzLeaderID_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->leader_id = GetLeaderID();
	zoneserver_list.SendPacket(pack.get());
}

void DynamicZone::HandleZoneMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzCreated:
	{
		dynamic_zone_manager.CacheNewDynamicZone(pack);
		break;
	}
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
	case ServerOP_DzAddRemoveMember:
	{
		auto buf = reinterpret_cast<ServerDzMember_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			auto status = static_cast<DynamicZoneMemberStatus>(buf->character_status);
			dz->ProcessMemberAddRemove({ buf->character_id, buf->character_name, status }, buf->removed);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzSwapMembers:
	{
		auto buf = reinterpret_cast<ServerDzMemberSwap_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			// we add first in world so new member can be chosen if leader is removed
			auto status = static_cast<DynamicZoneMemberStatus>(buf->add_character_status);
			dz->ProcessMemberAddRemove({ buf->add_character_id, buf->add_character_name, status }, false);
			dz->ProcessMemberAddRemove({ buf->remove_character_id, buf->remove_character_name }, true);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	case ServerOP_DzRemoveAllMembers:
	{
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->ProcessRemoveAllMembers();
		}
		zoneserver_list.SendPacket(pack);
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
	case ServerOP_DzGetMemberStatuses:
	{
		auto buf = reinterpret_cast<ServerDzID_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			dz->SendZoneMemberStatuses(buf->sender_zone_id, buf->sender_instance_id);
		}
		break;
	}
	case ServerOP_DzUpdateMemberStatus:
	{
		auto buf = reinterpret_cast<ServerDzMemberStatus_Struct*>(pack->pBuffer);
		auto dz = DynamicZone::FindDynamicZoneByID(buf->dz_id);
		if (dz)
		{
			auto status = static_cast<DynamicZoneMemberStatus>(buf->status);
			dz->ProcessMemberStatusChange(buf->character_id, status);
		}
		zoneserver_list.SendPacket(pack);
		break;
	}
	};
}

void DynamicZone::ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed)
{
	DynamicZoneBase::ProcessMemberAddRemove(member, removed);

	if (GetType() == DynamicZoneType::Expedition && removed && member.id == GetLeaderID())
	{
		ChooseNewLeader();
	}
}

bool DynamicZone::ProcessMemberStatusChange(uint32_t character_id, DynamicZoneMemberStatus status)
{
	bool changed = DynamicZoneBase::SetInternalMemberStatus(character_id, status);
	if (changed && GetType() == DynamicZoneType::Expedition)
	{
		// any member status update will trigger a leader fix if leader was offline
		if (GetLeader().status == DynamicZoneMemberStatus::Offline && GetMemberCount() > 1)
		{
			ChooseNewLeader();
		}
	}
	return changed;
}

void DynamicZone::SendZonesExpireWarning(uint32_t minutes_remaining)
{
	uint32_t pack_size = sizeof(ServerDzExpireWarning_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzExpireWarning, pack_size);
	auto buf = reinterpret_cast<ServerDzExpireWarning_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->minutes_remaining = minutes_remaining;
	zoneserver_list.SendPacket(pack.get());
}

void DynamicZone::SendZoneMemberStatuses(uint16_t zone_id, uint16_t instance_id)
{
	uint32_t members_count = static_cast<uint32_t>(m_members.size());
	uint32_t entries_size = sizeof(ServerDzMemberStatusEntry_Struct) * members_count;
	uint32_t pack_size = sizeof(ServerDzMemberStatuses_Struct) + entries_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_DzGetMemberStatuses, pack_size);
	auto buf = reinterpret_cast<ServerDzMemberStatuses_Struct*>(pack->pBuffer);
	buf->dz_id = GetID();
	buf->count = members_count;

	for (int i = 0; i < m_members.size(); ++i)
	{
		buf->entries[i].character_id = m_members[i].id;
		buf->entries[i].online_status = static_cast<uint8_t>(m_members[i].status);
	}

	zoneserver_list.SendPacket(zone_id, instance_id, pack.get());
}

void DynamicZone::CacheMemberStatuses()
{
	if (m_has_member_statuses)
	{
		return;
	}

	// called when a new dz is cached to fill member statuses
	std::string zone_name{};
	std::vector<ClientListEntry*> all_clients;
	all_clients.reserve(client_list.GetClientCount());
	client_list.GetClients(zone_name.c_str(), all_clients);

	for (const auto& member : m_members)
	{
		auto it = std::find_if(all_clients.begin(), all_clients.end(),
			[&](const ClientListEntry* cle) { return (cle && cle->CharID() == member.id); });

		auto status = DynamicZoneMemberStatus::Offline;
		if (it != all_clients.end())
		{
			status = DynamicZoneMemberStatus::Online;
			if (IsSameDz((*it)->zone(), (*it)->instance()))
			{
				status = DynamicZoneMemberStatus::InDynamicZone;
			}
		}

		SetInternalMemberStatus(member.id, status);
	}

	m_has_member_statuses = true;
}

void DynamicZone::CheckExpireWarning()
{
	if (m_warning_cooldown_timer.Check(false))
	{
		using namespace std::chrono_literals;
		auto remaining = GetDurationRemaining();
		if ((remaining > 14min && remaining < 15min) ||
		    (remaining > 4min && remaining < 5min) ||
		    (remaining > 0min && remaining < 1min))
		{
			int minutes = std::chrono::duration_cast<std::chrono::minutes>(remaining).count() + 1;
			SendZonesExpireWarning(minutes);
			m_warning_cooldown_timer.Start(120000); // 2 minute cooldown after a warning
		}
	}
}
