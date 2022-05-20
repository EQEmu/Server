/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "expedition.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zone.h"

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;
extern WorldServer worldserver;
extern Zone* zone;

#include "../common/repositories/zone_repository.h"
#include "../common/content/world_content_service.h"


void Client::Handle_OP_ZoneChange(const EQApplicationPacket *app) {
#ifdef BOTS
	// This block is necessary to clean up any bot objects owned by a Client
	Bot::ProcessClientZoneChange(this);
#endif

	bZoning = true;
	if (app->size != sizeof(ZoneChange_Struct)) {
		LogDebug("Wrong size: OP_ZoneChange, size=[{}], expected [{}]", app->size, sizeof(ZoneChange_Struct));
		return;
	}

#if EQDEBUG >= 5
	LogDebug("Zone request from [{}]", GetName());
	DumpPacket(app);
#endif
	ZoneChange_Struct* zc=(ZoneChange_Struct*)app->pBuffer;

	uint16 target_zone_id = 0;
	uint16 target_instance_id = zc->instanceID;
	ZonePoint* zone_point = nullptr;

	//figure out where they are going.
	if(zc->zoneID == 0) {
		//client dosent know where they are going...
		//try to figure it out for them.

		switch(zone_mode) {
		case EvacToSafeCoords:
		case ZoneToSafeCoords:
			//going to safe coords, but client dosent know where?
			//assume it is this zone for now.
			target_zone_id = zone->GetZoneID();
			break;
		case GMSummon:
			target_zone_id = zonesummon_id;
			break;
		case GateToBindPoint:
			target_zone_id = m_pp.binds[0].zone_id;
			target_instance_id = m_pp.binds[0].instance_id;
			break;
		case ZoneToBindPoint:
			target_zone_id = m_pp.binds[0].zone_id;
			target_instance_id = m_pp.binds[0].instance_id;
			break;
		case ZoneSolicited: //we told the client to zone somewhere, so we know where they are going.
			target_zone_id = zonesummon_id;
			break;
		case ZoneUnsolicited: //client came up with this on its own.
			zone_point = zone->GetClosestZonePointWithoutZone(GetX(), GetY(), GetZ(), this, ZONEPOINT_NOZONE_RANGE);
			if(zone_point) {
				//we found a zone point, which is a reasonable distance away
				//assume that is the one were going with.
				target_zone_id = zone_point->target_zone_id;
				target_instance_id = zone_point->target_zone_instance;
			} else {
				//unable to find a zone point... is there anything else
				//that can be a valid un-zolicited zone request?

				Message(Chat::Red, "Invalid unsolicited zone request.");
				LogError("Zoning [{}]: Invalid unsolicited zone request to zone id [{}]", GetName(), target_zone_id);
				if (GetBindZoneID() == target_zone_id) {
					cheat_manager.CheatDetected(MQGate, glm::vec3(zc->x, zc->y, zc->z));
				}
				else {
					cheat_manager.CheatDetected(MQZone, glm::vec3(zc->x, zc->y, zc->z));
				}
				SendZoneCancel(zc);
				return;
			}
			break;
		default:
			break;
		};
	}
	else {
		// This is to allow both 6.2 and Titanium clients to perform a proper zoning of the client when evac/succor
		// WildcardX 27 January 2008
		if(zone_mode == EvacToSafeCoords && zonesummon_id > 0)
			target_zone_id = zonesummon_id;
		else
			target_zone_id = zc->zoneID;

		//if we are zoning to a specific zone unsolicied,
		//then until otherwise determined, they must be zoning
		//on a zone line.
		if(zone_mode == ZoneUnsolicited)
		{
			if(target_zone_id == zone->GetZoneID())
			{
				SendZoneCancel(zc);
				return;
			}

			zone_point = zone->GetClosestZonePoint(glm::vec3(GetPosition()), target_zone_id, this, ZONEPOINT_ZONE_RANGE);
			//if we didnt get a zone point, or its to a different zone,
			//then we assume this is invalid.
			if(!zone_point || zone_point->target_zone_id != target_zone_id) {
				LogError("Zoning [{}]: Invalid unsolicited zone request to zone id [{}]", GetName(), target_zone_id);
				if (GetBindZoneID() == target_zone_id) {
					cheat_manager.CheatDetected(MQGate, glm::vec3(zc->x, zc->y, zc->z));
				}
				else {
					cheat_manager.CheatDetected(MQZone, glm::vec3(zc->x, zc->y, zc->z));
				}
				SendZoneCancel(zc);
				return;
			}
		}
	}

	if(target_instance_id > 0)
	{
		//make sure we are in it and it's unexpired.
		if(!database.VerifyInstanceAlive(target_instance_id, CharacterID()))
		{
			Message(Chat::Red, "Instance ID was expired or you were not in it.");
			SendZoneCancel(zc);
			return;
		}

		if(!database.VerifyZoneInstance(target_zone_id, target_instance_id))
		{
			Message(Chat::Red, "Instance ID was %u does not go with zone id %u", target_instance_id, target_zone_id);
			SendZoneCancel(zc);
			return;
		}
	}

	/* Check for Valid Zone */
	const char *target_zone_name = ZoneName(target_zone_id);
	if(target_zone_name == nullptr) {
		//invalid zone...
		Message(Chat::Red, "Invalid target zone ID.");
		LogError("Zoning [{}]: Unable to get zone name for zone id [{}]", GetName(), target_zone_id);
		SendZoneCancel(zc);
		return;
	}

	/* Load up the Safe Coordinates, restrictions and verify the zone name*/
	float safe_x, safe_y, safe_z, safe_heading;
	int16 min_status = AccountStatus::Player;
	uint8 min_level = 0;
	char flag_needed[128];
	if(!content_db.GetSafePoints(
		target_zone_name,
		database.GetInstanceVersion(target_instance_id),
		&safe_x,
		&safe_y,
		&safe_z,
		&safe_heading,
		&min_status,
		&min_level,
		flag_needed
	)) {
		//invalid zone...
		Message(Chat::Red, "Invalid target zone while getting safe points.");
		LogError("Zoning [{}]: Unable to get safe coordinates for zone [{}]", GetName(), target_zone_name);
		SendZoneCancel(zc);
		return;
	}

	std::string export_string = fmt::format(
		"{} {}",
		zone->GetZoneID(),
		target_zone_id
	);
	if (parse->EventPlayer(EVENT_ZONE, this, export_string, 0) != 0) {
		SendZoneCancel(zc);
		return;
	}

	//handle circumvention of zone restrictions
	//we need the value when creating the outgoing packet as well.
	uint8 ignore_restrictions = zonesummon_ignorerestrictions;
	zonesummon_ignorerestrictions = 0;

	float target_x = 0, target_y = 0, target_z = 0, target_heading = 0;
	switch(zone_mode) {
	case EvacToSafeCoords:
	case ZoneToSafeCoords:
		LogDebug(
			"Zoning [{}] to safe coords ([{}], [{}], [{}], [{}]) in [{}] ([{}])",
			GetName(),
			safe_x,
			safe_y,
			safe_z,
			safe_heading,
			target_zone_name,
			target_zone_id
		);
		target_x = safe_x;
		target_y = safe_y;
		target_z = safe_z;
		target_heading = safe_heading;
		break;
	case GMSummon:
		target_x = m_ZoneSummonLocation.x;
		target_y = m_ZoneSummonLocation.y;
		target_z = m_ZoneSummonLocation.z;
		target_heading = m_ZoneSummonLocation.w;
		ignore_restrictions = 1;
		break;
	case GateToBindPoint:
		target_x = m_pp.binds[0].x;
		target_y = m_pp.binds[0].y;
		target_z = m_pp.binds[0].z;
		target_heading = m_pp.binds[0].heading;
		break;
	case ZoneToBindPoint:
		target_x = m_pp.binds[0].x;
		target_y = m_pp.binds[0].y;
		target_z = m_pp.binds[0].z;
		target_heading = m_pp.binds[0].heading;
		ignore_restrictions = 1;	//can always get to our bind point? seems exploitable
		break;
	case ZoneSolicited: //we told the client to zone somewhere, so we know where they are going.
		//recycle zonesummon variables
		target_x = m_ZoneSummonLocation.x;
		target_y = m_ZoneSummonLocation.y;
		target_z = m_ZoneSummonLocation.z;
		target_heading = m_ZoneSummonLocation.w;
		break;
	case ZoneUnsolicited: //client came up with this on its own.
		//client requested a zoning... what are the cases when this could happen?

		//Handle zone point case:
		if(zone_point != nullptr) {
			//they are zoning using a valid zone point, figure out coords

			//999999 is a placeholder for 'same as where they were from'
			if(zone_point->target_x == 999999)
				target_x = GetX();
			else
				target_x = zone_point->target_x;

			if(zone_point->target_y == 999999)
				target_y = GetY();
			else
				target_y = zone_point->target_y;

			if(zone_point->target_z == 999999)
				target_z = GetZ();
			else
				target_z = zone_point->target_z;

			if(zone_point->target_heading == 999)
				target_heading = GetHeading();
			else
				target_heading = zone_point->target_heading;

			break;
		}

		//for now, there are no other cases...

		//could not find a valid reason for them to be zoning, stop it.
		if (GetBindZoneID() == target_zone_id) {
			cheat_manager.CheatDetected(MQGate, glm::vec3(zc->x, zc->y, zc->z));
		}
		else {
			cheat_manager.CheatDetected(MQZone, glm::vec3(zc->x, zc->y, zc->z));
		}
		LogError("Zoning [{}]: Invalid unsolicited zone request to zone id [{}]. Not near a zone point", GetName(), target_zone_name);
		SendZoneCancel(zc);
		return;
	default:
		break;
	};

	//OK, now we should know where were going...

	//Check some rules first.
	int8 myerror = 1;		//1 is succes

	//not sure when we would use ZONE_ERROR_NOTREADY

	//enforce min status and level
	if (!ignore_restrictions && (Admin() < min_status || GetLevel() < min_level))
	{
		myerror = ZONE_ERROR_NOEXPERIENCE;
	}

	if(!ignore_restrictions && flag_needed[0] != '\0') {
		//the flag needed string is not empty, meaning a flag is required.
		if(Admin() < minStatusToIgnoreZoneFlags && !HasZoneFlag(target_zone_id))
		{
			Message(Chat::Red, "You do not have the flag to enter %s.", target_zone_name);
			myerror = ZONE_ERROR_NOEXPERIENCE;
		}
	}

	//TODO: ADVENTURE ENTRANCE CHECK

	/**
	 * Expansion check
	 */
	if (content_service.GetCurrentExpansion() >= Expansion::Classic && !GetGM()) {

		/**
		 * Hit the zone cache first so we're not hitting the database every time someone attempts to zone
		 */
		bool      meets_zone_expansion_check = false;
		bool      found_zone                 = false;
		for (auto &z: zone_store.zones) {
			if (z.short_name == target_zone_name && z.version == 0) {
				found_zone = true;
				if (z.expansion <= (content_service.GetCurrentExpansion() + 1)) {
					meets_zone_expansion_check = true;
					break;
				}
			}
		}

		/**
		 * If we fail to find a cached zone lookup because someone just so happened to change some data, second attempt
		 * In 99% of cases we would never get here and this would be fallback
		 */
		if (!found_zone) {
			auto zones = ZoneRepository::GetWhere(content_db,
				fmt::format(
					"expansion <= {} AND short_name = '{}' and version = 0",
					(content_service.GetCurrentExpansion() + 1),
					target_zone_name
				)
			);

			meets_zone_expansion_check = !zones.empty();
		}

		LogInfo(
			"Checking zone request [{}] for expansion [{}] ({}) success [{}]",
			target_zone_name,
			(content_service.GetCurrentExpansion() + 1),
			content_service.GetCurrentExpansionName(),
			meets_zone_expansion_check ? "true" : "false"
		);

		if (!meets_zone_expansion_check) {
			myerror = ZONE_ERROR_NOEXPANSION;
		}
	}

	if (content_service.GetCurrentExpansion() >= Expansion::Classic && GetGM()) {
		LogInfo("[{}] Bypassing Expansion zone checks because GM status is set", GetCleanName());
	}

	if(myerror == 1) {
		//we have successfully zoned
		DoZoneSuccess(zc, target_zone_id, target_instance_id, target_x, target_y, target_z, target_heading, ignore_restrictions);
	} else {
		LogError("Zoning [{}]: Rules prevent this char from zoning into [{}]", GetName(), target_zone_name);
		SendZoneError(zc, myerror);
	}
}

void Client::SendZoneCancel(ZoneChange_Struct *zc) {
	//effectively zone them right back to where they were
	//unless we find a better way to stop the zoning process.
	cheat_manager.SetExemptStatus(Port, true);
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
	ZoneChange_Struct *zc2 = (ZoneChange_Struct*)outapp->pBuffer;
	strcpy(zc2->char_name, zc->char_name);
	zc2->zoneID = zone->GetZoneID();
	zc2->success = 1;
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	//reset to unsolicited.
	zone_mode = ZoneUnsolicited;
	// reset since we're not zoning anymore
	bZoning = false;
}

void Client::SendZoneError(ZoneChange_Struct *zc, int8 err)
{
	LogError("Zone [{}] is not available because target wasn't found or character insufficent level", zc->zoneID);
	cheat_manager.SetExemptStatus(Port, true);
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
	ZoneChange_Struct *zc2 = (ZoneChange_Struct*)outapp->pBuffer;
	strcpy(zc2->char_name, zc->char_name);
	zc2->zoneID = zc->zoneID;
	zc2->success = err;
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	//reset to unsolicited.
	zone_mode = ZoneUnsolicited;
	// reset since we're not zoning anymore
	bZoning = false;
}

void Client::DoZoneSuccess(ZoneChange_Struct *zc, uint16 zone_id, uint32 instance_id, float dest_x, float dest_y, float dest_z, float dest_h, int8 ignore_r) {
	//this is called once the client is fully allowed to zone here
	//it takes care of all the activities which occur when a client zones out

	SendLogoutPackets();

	/* QS: PlayerLogZone */
	if (RuleB(QueryServ, PlayerLogZone)){
		std::string event_desc = StringFormat("Zoning :: zoneid:%u instid:%u x:%4.2f y:%4.2f z:%4.2f h:%4.2f zonemode:%d from zoneid:%u instid:%i", zone_id, instance_id, dest_x, dest_y, dest_z, dest_h, zone_mode, GetZoneID(), GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_Zoning, CharacterID(), event_desc);
	}

	/* Dont clear aggro until the zone is successful */
	entity_list.RemoveFromHateLists(this);

	if(GetPet())
		entity_list.RemoveFromHateLists(GetPet());

	if (GetPendingExpeditionInviteID() != 0)
	{
		// live re-invites if client zoned with a pending invite, save pending invite info in world
		auto expedition = Expedition::FindCachedExpeditionByID(GetPendingExpeditionInviteID());
		if (expedition)
		{
			expedition->SendWorldPendingInvite(m_pending_expedition_invite, GetName());
		}
	}

	LogInfo("Zoning [{}] to: [{}] ([{}]) - ([{}]) x [{}] y [{}] z [{}]", m_pp.name, ZoneName(zone_id), zone_id, instance_id, dest_x, dest_y, dest_z);

	//set the player's coordinates in the new zone so they have them
	//when they zone into it
	m_Position.x = dest_x; //these coordinates will now be saved when ~client is called
	m_Position.y = dest_y;
	m_Position.z = dest_z;
	m_Position.w = dest_h; // Cripp: fix for zone heading
	m_pp.heading = dest_h;
	m_pp.zone_id = zone_id;
	m_pp.zoneInstance = instance_id;

	//Force a save so its waiting for them when they zone
	Save(2);

	if (zone_id == zone->GetZoneID() && instance_id == zone->GetInstanceID()) {
		// No need to ask worldserver if we're zoning to ourselves (most
		// likely to a bind point), also fixes a bug since the default response was failure
		auto outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
		ZoneChange_Struct* zc2 = (ZoneChange_Struct*) outapp->pBuffer;
		strcpy(zc2->char_name, GetName());
		zc2->zoneID = zone_id;
		zc2->instanceID = instance_id;
		zc2->success = 1;
		outapp->priority = 6;
		FastQueuePacket(&outapp);

		zone->StartShutdownTimer(AUTHENTICATION_TIMEOUT * 1000);
	} else {
	// vesuvias - zoneing to another zone so we need to the let the world server
	//handle things with the client for a while
	auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
	ZoneToZone_Struct *ztz = (ZoneToZone_Struct *)pack->pBuffer;
	ztz->response = 0;
	ztz->current_zone_id = zone->GetZoneID();
	ztz->current_instance_id = zone->GetInstanceID();
	ztz->requested_zone_id = zone_id;
	ztz->requested_instance_id = instance_id;
	ztz->admin = admin;
	ztz->ignorerestrictions = ignore_r;
	strcpy(ztz->name, GetName());
	ztz->guild_id = GuildID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
	}

	//reset to unsolicited.
	zone_mode = ZoneUnsolicited;
	m_ZoneSummonLocation = glm::vec4();
	zonesummon_id = 0;
	zonesummon_ignorerestrictions = 0;
}

void Client::MovePC(const char* zonename, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	ProcessMovePC(ZoneID(zonename), 0, x, y, z, heading, ignorerestrictions, zm);
}

//designed for in zone moving
void Client::MovePC(float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	ProcessMovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading, ignorerestrictions, zm);
}

void Client::MovePC(uint32 zoneID, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	ProcessMovePC(zoneID, 0, x, y, z, heading, ignorerestrictions, zm);
}

void Client::MovePC(uint32 zoneID, uint32 instanceID, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm){
	ProcessMovePC(zoneID, instanceID, x, y, z, heading, ignorerestrictions, zm);
}

void Client::MoveZone(const char *zone_short_name) {
	ProcessMovePC(ZoneID(zone_short_name), 0, 0.0f, 0.0f, 0.0f, 0.0f, 3, ZoneToSafeCoords);
}

void Client::MoveZoneGroup(const char *zone_short_name) {
	if (!GetGroup()) {
		MoveZone(zone_short_name);
	} else {
		auto client_group = GetGroup();
		for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
			if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
				auto group_member = client_group->members[member_index]->CastToClient();
				group_member->MoveZone(zone_short_name);
			}
		}
	}
}

void Client::MoveZoneRaid(const char *zone_short_name) {
	if (!GetRaid()) {
		MoveZone(zone_short_name);
	} else {
		auto client_raid = GetRaid();
		for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
			if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
				auto raid_member = client_raid->members[member_index].member->CastToClient();
				raid_member->MoveZone(zone_short_name);
			}
		}
	}
}

void Client::MoveZoneInstance(uint16 instance_id) {
	if (!database.CharacterInInstanceGroup(instance_id, CharacterID())) {
		database.AddClientToInstance(instance_id, CharacterID());
	}
	ProcessMovePC(database.ZoneIDFromInstanceID(instance_id), instance_id, 0.0f, 0.0f, 0.0f, 0.0f, 3, ZoneToSafeCoords);
}

void Client::MoveZoneInstanceGroup(uint16 instance_id) {
	if (!GetGroup()) {
		MoveZoneInstance(instance_id);
	} else {
		auto client_group = GetGroup();
		for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
			if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
				auto group_member = client_group->members[member_index]->CastToClient();
				group_member->MoveZoneInstance(instance_id);
			}
		}
	}
}

void Client::MoveZoneInstanceRaid(uint16 instance_id) {
	if (!GetRaid()) {
		MoveZoneInstance(instance_id);
	} else {
		auto client_raid = GetRaid();
		for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
			if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
				auto raid_member = client_raid->members[member_index].member->CastToClient();
				raid_member->MoveZoneInstance(instance_id);
			}
		}
	}
}

void Client::ProcessMovePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm)
{
	// From what I have read, dragged corpses should stay with the player for Intra-zone summons etc, but we can implement that later.
	ClearDraggedCorpses();

	if(zoneID == 0)
		zoneID = zone->GetZoneID();

	if(zoneID == zone->GetZoneID() && instance_id == zone->GetInstanceID()) {
		// TODO: Determine if this condition is necessary.
		if(IsAIControlled()) {
			GMMove(x, y, z);
			return;
		}

		if(zm != SummonPC && GetPetID() != 0) {
			//if they have a pet and they are staying in zone, move with them
			Mob *p = GetPet();
			if(p != nullptr){
				p->SetPetOrder(SPO_Follow);
				p->GMMove(x+15, y, z);	//so it dosent have to run across the map.
			}
		}
	}

	switch(zm) {
		case GateToBindPoint:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case EvacToSafeCoords:
		case ZoneToSafeCoords:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case GMSummon:
			Message(Chat::Yellow, "You have been summoned by a GM!");
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case ZoneToBindPoint:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case ZoneSolicited:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case SummonPC:
			Message(Chat::Yellow, "You have been summoned!");
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case Rewind:
			Message(Chat::Yellow, "Rewinding to previous location.");
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		default:
			LogError("Client::ProcessMovePC received a reguest to perform an unsupported client zone operation");
			break;
	}
}

void Client::ZonePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	bool ReadyToZone = true;
	int iZoneNameLength = 0;
	const char*	pShortZoneName = nullptr;
	char* pZoneName = nullptr;

	pShortZoneName = ZoneName(zoneID);
	content_db.GetZoneLongName(pShortZoneName, &pZoneName);

	cheat_manager.SetExemptStatus(Port, true);

	if(!pZoneName) {
		Message(Chat::Red, "Invalid zone number specified");
		safe_delete_array(pZoneName);
		return;
	}
	iZoneNameLength = strlen(pZoneName);
	glm::vec4 zone_safe_point;
	switch(zm) {
		case EvacToSafeCoords:
		case ZoneToSafeCoords:
			zone_safe_point = zone->GetSafePoint();
			x = zone_safe_point.x;
			y = zone_safe_point.y;
			z = zone_safe_point.z;
			heading = zone_safe_point.w;
			break;
		case GMSummon:
			m_Position = glm::vec4(x, y, z, heading);
			m_ZoneSummonLocation = m_Position;
			zonesummon_id = zoneID;
			zonesummon_ignorerestrictions = 1;
			break;
		case ZoneSolicited:
			m_ZoneSummonLocation = glm::vec4(x, y, z, heading);
			zonesummon_id = zoneID;
			zonesummon_ignorerestrictions = ignorerestrictions;
			break;
		case GateToBindPoint:
			x = m_Position.x = m_pp.binds[0].x;
			y = m_Position.y = m_pp.binds[0].y;
			z = m_Position.z = m_pp.binds[0].z;
			heading = m_pp.binds[0].heading;
			break;
		case ZoneToBindPoint:
			x = m_Position.x = m_pp.binds[0].x;
			y = m_Position.y = m_pp.binds[0].y;
			z = m_Position.z = m_pp.binds[0].z;
			heading = m_pp.binds[0].heading;
			zonesummon_ignorerestrictions = 1;
			LogDebug("Player [{}] has died and will be zoned to bind point in zone: [{}] at LOC x=[{}], y=[{}], z=[{}], heading=[{}]",
					GetName(), pZoneName, m_pp.binds[0].x, m_pp.binds[0].y, m_pp.binds[0].z, m_pp.binds[0].heading);
			break;
		case SummonPC:
			m_ZoneSummonLocation = glm::vec4(x, y, z, heading);
			m_Position = m_ZoneSummonLocation;
			break;
		case Rewind:
			LogDebug("[{}] has requested a /rewind from [{}], [{}], [{}], to [{}], [{}], [{}] in [{}]", GetName(),
					m_Position.x, m_Position.y, m_Position.z,
					m_RewindLocation.x, m_RewindLocation.y, m_RewindLocation.z, zone->GetShortName());
			m_ZoneSummonLocation = glm::vec4(x, y, z, heading);
			m_Position = m_ZoneSummonLocation;
			break;
		default:
			LogError("Client::ZonePC() received a reguest to perform an unsupported client zone operation");
			ReadyToZone = false;
			break;
	}

	if (ReadyToZone)
	{
		//if client is looting, we need to send an end loot
		if (IsLooting())
		{
			Entity* entity = entity_list.GetID(entity_id_being_looted);
			if (entity == 0)
			{
				Message(Chat::Red, "Error: OP_EndLootRequest: Corpse not found (ent = 0)");
				if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
					Corpse::SendEndLootErrorPacket(this);
				else
					Corpse::SendLootReqErrorPacket(this);
			}
			else if (!entity->IsCorpse())
			{
				Message(Chat::Red, "Error: OP_EndLootRequest: Corpse not found (!entity->IsCorpse())");
				Corpse::SendLootReqErrorPacket(this);
			}
			else
			{
				Corpse::SendEndLootErrorPacket(this);
				entity->CastToCorpse()->EndLoot(this, nullptr);
			}
			SetLooting(0);
		}

		zone_mode = zm;
		if (zm == ZoneToBindPoint) {
			auto outapp = new EQApplicationPacket(OP_ZonePlayerToBind,
							      sizeof(ZonePlayerToBind_Struct) + iZoneNameLength);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			// If we are SoF and later and are respawning from hover, we want the real zone ID, else zero to use the old hack.
			//
			if(zone->GetZoneID() == zoneID) {
				if ((ClientVersionBit() & EQ::versions::maskSoFAndLater) && (!RuleB(Character, RespawnFromHover) || !IsHoveringForRespawn()))
					gmg->bind_zone_id = 0;
				else
					gmg->bind_zone_id = zoneID;
			} else {
				gmg->bind_zone_id = zoneID;
			}

			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			strcpy(gmg->zone_name, pZoneName);

			outapp->priority = 6;
			FastQueuePacket(&outapp);
			safe_delete(outapp);
		}
		else if(zm == ZoneSolicited || zm == ZoneToSafeCoords) {
			auto outapp =
			    new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
			RequestClientZoneChange_Struct* gmg = (RequestClientZoneChange_Struct*) outapp->pBuffer;

			gmg->zone_id = zoneID;
			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			gmg->instance_id = instance_id;
			gmg->type = 0x01;				//an observed value, not sure of meaning

			outapp->priority = 6;
			FastQueuePacket(&outapp);
			safe_delete(outapp);
		}
		else if(zm == EvacToSafeCoords) {
			auto outapp =
			    new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
			RequestClientZoneChange_Struct* gmg = (RequestClientZoneChange_Struct*) outapp->pBuffer;

			// if we are in the same zone we want to evac to, client will not send OP_ZoneChange back to do an actual
			// zoning of the client, so we have to send a viable zoneid that the client *could* zone to to make it believe
			// we are leaving the zone, even though we are not. We have to do this because we are missing the correct op code
			// and struct that should be used for evac/succor.
			// 213 is Plane of War
			// 76 is orignial Plane of Hate
			// WildcardX 27 January 2008. Tested this for 6.2 and Titanium clients.

			if(GetZoneID() == 1)
				gmg->zone_id = 2;
			else if(GetZoneID() == 2)
				gmg->zone_id = 1;
			else
				gmg->zone_id = 1;

			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			gmg->instance_id = instance_id;
			gmg->type = 0x01;				// '0x01' was an observed value for the type field, not sure of meaning

			// we hide the real zoneid we want to evac/succor to here
			zonesummon_id = zoneID;

			outapp->priority = 6;
			FastQueuePacket(&outapp);
			safe_delete(outapp);
		}
		else {
			if(zoneID == GetZoneID()) {
				//properly handle proximities
				entity_list.ProcessMove(this, glm::vec3(m_Position));
				m_Proximity = glm::vec3(m_Position);

				//send out updates to people in zone.
				SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
			}

			auto outapp =
			    new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
			RequestClientZoneChange_Struct* gmg = (RequestClientZoneChange_Struct*) outapp->pBuffer;

			gmg->zone_id = zoneID;
			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			gmg->instance_id = instance_id;
			gmg->type = 0x01;	//an observed value, not sure of meaning
			outapp->priority = 6;
			FastQueuePacket(&outapp);
			safe_delete(outapp);
		}

		LogDebug("Player [{}] has requested a zoning to LOC x=[{}], y=[{}], z=[{}], heading=[{}] in zoneid=[{}]", GetName(), x, y, z, heading, zoneID);
		//Clear zonesummon variables if we're zoning to our own zone
		//Client wont generate a zone change packet to the server in this case so
		//They aren't needed and it keeps behavior on next zone attempt from being undefined.
		if(zoneID == zone->GetZoneID() && instance_id == zone->GetInstanceID())
		{
			if(zm != EvacToSafeCoords && zm != ZoneToSafeCoords && zm != ZoneToBindPoint)
			{
				m_ZoneSummonLocation = glm::vec4();
				zonesummon_id = 0;
				zonesummon_ignorerestrictions = 0;
				zone_mode = ZoneUnsolicited;
			}
		}
	}

	safe_delete_array(pZoneName);
}

void Client::GoToSafeCoords(uint16 zone_id, uint16 instance_id) {
	if(zone_id == 0)
		zone_id = zone->GetZoneID();

	MovePC(zone_id, instance_id, 0.0f, 0.0f, 0.0f, 0.0f, 0, ZoneToSafeCoords);
}


void Mob::Gate(uint8 bind_number) {
	GoToBind(bind_number);
	if (RuleB(NPC, NPCHealOnGate) && IsNPC() && GetHPRatio() <= RuleR(NPC, NPCHealOnGateAmount)) {
		auto HealAmount = (RuleR(NPC, NPCHealOnGateAmount) / 100);
		SetHP(int64(GetMaxHP() * HealAmount));
	}
}

void Client::Gate(uint8 bind_number) {
	Mob::Gate(bind_number);
}

void NPC::Gate(uint8 bind_number) {
	entity_list.MessageCloseString(this, true, RuleI(Range, SpellMessages), Chat::Spells, GATES, GetCleanName());

	Mob::Gate(bind_number);
}

void Client::SetBindPoint(int bind_number, int to_zone, int to_instance, const glm::vec3 &location)
{
	if (bind_number < 0 || bind_number >= 4)
		bind_number = 0;

	if (to_zone == -1) {
		m_pp.binds[bind_number].zone_id = zone->GetZoneID();
		m_pp.binds[bind_number].instance_id = (zone->GetInstanceID() != 0 && zone->IsInstancePersistent()) ? zone->GetInstanceID() : 0;
		m_pp.binds[bind_number].x = m_Position.x;
		m_pp.binds[bind_number].y = m_Position.y;
		m_pp.binds[bind_number].z = m_Position.z;
	} else {
		m_pp.binds[bind_number].zone_id = to_zone;
		m_pp.binds[bind_number].instance_id = to_instance;
		m_pp.binds[bind_number].x = location.x;
		m_pp.binds[bind_number].y = location.y;
		m_pp.binds[bind_number].z = location.z;
	}
	database.SaveCharacterBindPoint(CharacterID(), m_pp.binds[bind_number], bind_number);
}

void Client::SetBindPoint2(int bind_number, int to_zone, int to_instance, const glm::vec4 &location)
{
	if (bind_number < 0 || bind_number >= 4)
		bind_number = 0;

	if (to_zone == -1) {
		m_pp.binds[bind_number].zone_id = zone->GetZoneID();
		m_pp.binds[bind_number].instance_id = (zone->GetInstanceID() != 0 && zone->IsInstancePersistent()) ? zone->GetInstanceID() : 0;
		m_pp.binds[bind_number].x = m_Position.x;
		m_pp.binds[bind_number].y = m_Position.y;
		m_pp.binds[bind_number].z = m_Position.z;
		m_pp.binds[bind_number].heading = m_Position.w;
	} else {
		m_pp.binds[bind_number].zone_id = to_zone;
		m_pp.binds[bind_number].instance_id = to_instance;
		m_pp.binds[bind_number].x = location.x;
		m_pp.binds[bind_number].y = location.y;
		m_pp.binds[bind_number].z = location.z;
		m_pp.binds[bind_number].heading = location.w;
	}
	database.SaveCharacterBindPoint(CharacterID(), m_pp.binds[bind_number], bind_number);
}

void Client::GoToBind(uint8 bind_number) {
	// if the bind number is invalid, use the primary bind
	if(bind_number > 4)
		bind_number = 0;

	// move the client, which will zone them if needed.
	// ignore restrictions on the zone request..?
	if(bind_number == 0)
		MovePC(
			m_pp.binds[0].zone_id,
			m_pp.binds[0].instance_id,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1,
			GateToBindPoint
		);
	else
		MovePC(
			m_pp.binds[bind_number].zone_id,
			m_pp.binds[bind_number].instance_id,
			m_pp.binds[bind_number].x,
			m_pp.binds[bind_number].y,
			m_pp.binds[bind_number].z,
			m_pp.binds[bind_number].heading,
			1
		);
}

void Client::GoToDeath() {
	MovePC(m_pp.binds[0].zone_id, m_pp.binds[0].instance_id, 0.0f, 0.0f, 0.0f, 0.0f, 1, ZoneToBindPoint);
}

void Client::ClearZoneFlag(uint32 zone_id) {
	if (!HasZoneFlag(zone_id)) {
		return;
	}

	zone_flags.erase(zone_id);

	std::string query = fmt::format(
		"DELETE FROM zone_flags WHERE charID = {} AND zoneID = {}",
		CharacterID(),
		zone_id
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		LogError("MySQL Error while trying to clear zone flag for [{}]: [{}]", GetName(), results.ErrorMessage().c_str());
	}
}

bool Client::HasZoneFlag(uint32 zone_id) const {
	return zone_flags.find(zone_id) != zone_flags.end();
}

void Client::LoadZoneFlags() {
	std::string query = fmt::format(
		"SELECT zoneID from zone_flags WHERE charID = {}",
		CharacterID()
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		LogError("MySQL Error while trying to load zone flags for [{}]: [{}]", GetName(), results.ErrorMessage().c_str());
		return;
	}

	zone_flags.clear();

	for (auto row : results) {
		zone_flags.insert(std::stoul(row[0]));
	}
}

void Client::SendZoneFlagInfo(Client *to) const {
	if (zone_flags.empty()) {
		to->Message(
			Chat::White,
			fmt::format(
				"{} {} no Zone Flags.",
				to == this ? "You" : GetName(),
				to == this ? "have" : "has"
			).c_str()
		);
		return;
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} the following Zone Flags:",
			to == this ? "You" : GetName(),
			to == this ? "have" : "has"
		).c_str()
	);

	int flag_count = 0;
	for (const auto& zone_id : zone_flags) {
		int flag_number = (flag_count + 1);
		const char* zone_short_name = ZoneName(zone_id, true);
		if (zone_short_name != "UNKNOWN") {
			std::string zone_long_name = ZoneLongName(zone_id);
			float safe_x, safe_y, safe_z, safe_heading;
			int16 min_status = AccountStatus::Player;
			uint8 min_level = 0;
			char flag_name[128];
			if (!content_db.GetSafePoints(
				zone_short_name,
				0,
				&safe_x,
				&safe_y,
				&safe_z,
				&safe_heading,
				&min_status,
				&min_level,
				flag_name
			)) {
				strcpy(flag_name, "ERROR");
			}

			to->Message(
				Chat::White,
				fmt::format(
					"Flag {} | Zone ID: {} Zone Name: {} ({}){}",
					flag_number,
					zone_id,
					zone_long_name,
					zone_short_name,
					(
						flag_name != "" ?
						fmt::format(
							" Flag Required: {}",
							flag_name
						) :
						""
					)
				).c_str()
			);
			flag_count++;
		}
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} {} Zone Flags.",
			to == this ? "You" : GetName(),
			to == this ? "have" : "has",
			flag_count
		).c_str()
	);
}

void Client::SetZoneFlag(uint32 zone_id) {
	if (HasZoneFlag(zone_id)) {
		return;
	}

	zone_flags.insert(zone_id);

	std::string query = fmt::format(
		"INSERT INTO zone_flags (charID, zoneID) VALUES ({}, {})",
		CharacterID(),
		zone_id
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		LogError("MySQL Error while trying to set zone flag for [{}]: [{}]", GetName(), results.ErrorMessage().c_str());
	}
}

void Client::ClearPEQZoneFlag(uint32 zone_id) {
	if (!HasPEQZoneFlag(zone_id)) {
		return;
	}

	peqzone_flags.erase(zone_id);

	auto query = fmt::format(
		"DELETE FROM character_peqzone_flags WHERE id = {} AND zone_id = {}",
		CharacterID(),
		zone_id
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		LogError("MySQL Error while trying to clear zone flag for [{}]: [{}]", GetName(), results.ErrorMessage().c_str());
	}
}

bool Client::HasPEQZoneFlag(uint32 zone_id) const {
	return peqzone_flags.find(zone_id) != peqzone_flags.end();
}

void Client::LoadPEQZoneFlags() {
	std::string query = fmt::format(
		"SELECT zone_id from character_peqzone_flags WHERE id = {}",
		CharacterID()
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		LogError("MySQL Error while trying to load zone flags for [{}]: [{}]", GetName(), results.ErrorMessage().c_str());
		return;
	}

	peqzone_flags.clear();

	for (auto row : results) {
		peqzone_flags.insert(std::stoul(row[0]));
	}
}

void Client::SendPEQZoneFlagInfo(Client *to) const {
	if (peqzone_flags.empty()) {
		to->Message(
			Chat::White,
			fmt::format(
				"{} {} no PEQZone Flags.",
				to == this ? "You" : GetName(),
				to == this ? "have" : "has"
			).c_str()
		);
		return;
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} the following PEQZone Flags:",
			to == this ? "You" : GetName(),
			to == this ? "have" : "has"
		).c_str()
	);

	int flag_count = 0;
	for (const auto& zone_id : peqzone_flags) {
		int flag_number = (flag_count + 1);
		std::string zone_short_name = ZoneName(zone_id, true);
		if (zone_short_name != "UNKNOWN") {
			std::string zone_long_name = ZoneLongName(zone_id);
			to->Message(
				Chat::White,
				fmt::format(
					"Flag {} | Zone ID: {} Zone Name: {} ({})",
					flag_number,
					zone_id,
					zone_long_name,
					zone_short_name
				).c_str()
			);
			flag_count++;
		}
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} {} PEQZone Flags.",
			to == this ? "You" : GetName(),
			to == this ? "have" : "has",
			flag_count
		).c_str()
	);
}

void Client::SetPEQZoneFlag(uint32 zone_id) {
	if (HasPEQZoneFlag(zone_id)) {
		return;
	}

	peqzone_flags.insert(zone_id);

	auto query = fmt::format(
		"INSERT INTO character_peqzone_flags (id, zone_id) VALUES ({}, {})",
		CharacterID(),
		zone_id
	);
	auto results = database.QueryDatabase(query);

	if (!results.Success()) {
		LogError("MySQL Error while trying to set zone flag for [{}]: [{}]", GetName(), results.ErrorMessage().c_str());
	}
}

bool Client::CanBeInZone() {
	//check some critial rules to see if this char needs to be booted from the zone
	//only enforce rules here which are serious enough to warrant being kicked from
	//the zone

	if(Admin() >= RuleI(GM, MinStatusToZoneAnywhere))
		return(true);

	float safe_x, safe_y, safe_z, safe_heading;
	int16 min_status = AccountStatus::Player;
	uint8 min_level = 0;
	char flag_needed[128];
	if(!content_db.GetSafePoints(
		zone->GetShortName(),
		zone->GetInstanceVersion(),
		&safe_x,
		&safe_y,
		&safe_z,
		&safe_heading,
		&min_status,
		&min_level,
		flag_needed
	)) {
		//this should not happen...
		LogDebug("[CLIENT] Unable to query zone info for ourself [{}]", zone->GetShortName());
		return(false);
	}

	if(GetLevel() < min_level) {
		LogDebug("[CLIENT] Character does not meet min level requirement ([{}] < [{}])!", GetLevel(), min_level);
		return(false);
	}
	if(Admin() < min_status) {
		LogDebug("[CLIENT] Character does not meet min status requirement ([{}] < [{}])!", Admin(), min_status);
		return(false);
	}

	if(flag_needed[0] != '\0') {
		//the flag needed string is not empty, meaning a flag is required.
		if(Admin() < minStatusToIgnoreZoneFlags && !HasZoneFlag(zone->GetZoneID())) {
			LogDebug("[CLIENT] Character does not have the flag to be in this zone ([{}])!", flag_needed);
			return(false);
		}
	}

	return(true);
}
