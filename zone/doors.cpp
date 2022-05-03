/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include "../common/string_util.h"

#include "client.h"
#include "doors.h"
#include "entity.h"
#include "guild_mgr.h"
#include "mob.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zonedb.h"
#include "zone_store.h"
#include "../common/repositories/criteria/content_filter_criteria.h"

#include <iostream>
#include <string.h>

#define OPEN_DOOR 0x02
#define CLOSE_DOOR 0x03
#define OPEN_INVDOOR 0x03
#define CLOSE_INVDOOR 0x02

extern EntityList entity_list;
extern WorldServer worldserver;

Doors::Doors(const DoorsRepository::Doors& door) :
		close_timer(5000),
		m_Position(door.pos_x, door.pos_y, door.pos_z, door.heading),
		m_Destination(door.dest_x, door.dest_y, door.dest_z, door.dest_heading)
{
	strn0cpy(zone_name, door.zone.c_str(), sizeof(zone_name));
	strn0cpy(door_name, door.name.c_str(), sizeof(door_name));
	strn0cpy(destination_zone_name, door.dest_zone.c_str(), sizeof(destination_zone_name));

	database_id             = door.id;
	door_id                 = door.doorid;
	incline                 = door.incline;
	open_type               = door.opentype;
	guild_id                = door.guild;
	lockpick                = door.lockpick;
	key_item_id             = door.keyitem;
	no_key_ring             = door.nokeyring;
	trigger_door            = door.triggerdoor;
	trigger_type            = door.triggertype;
	triggered               = false;
	door_param              = door.door_param;
	size                    = door.size;
	invert_state            = door.invert_state;
	destination_instance_id = door.dest_instance;
	is_ldon_door            = door.is_ldon_door;
	client_version_mask     = door.client_version_mask;

	SetOpenState(false);

	close_timer.Disable();

	disable_timer = (door.disable_timer == 1 ? true : false);
}

Doors::Doors(const char *model, const glm::vec4 &position, uint8 open_type, uint16 size) :
		close_timer(5000),
		m_Position(position),
		m_Destination(glm::vec4()){

	strn0cpy(zone_name, zone->GetShortName(), 32);
	strn0cpy(door_name, model, 32);
	strn0cpy(destination_zone_name, "NONE", 32);

	database_id = (uint32) content_db.GetDoorsCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion());
	door_id     = (uint8) content_db.GetDoorsDBCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion());

	open_type               = open_type;
	size                    = size;
	incline                 = 0;
	guild_id                = 0;
	lockpick                = 0;
	key_item_id             = 0;
	no_key_ring             = 0;
	trigger_door            = 0;
	trigger_type            = 0;
	triggered               = false;
	door_param              = 0;
	invert_state            = 0;
	is_ldon_door            = 0;
	client_version_mask     = 4294967295u;
	disable_timer           = 0;
	destination_instance_id = 0;

	SetOpenState(false);
	close_timer.Disable();
}


Doors::~Doors()
{
}

bool Doors::Process()
{
	if (close_timer.Enabled() && close_timer.Check() && IsDoorOpen()) {
		if (open_type == 40 || GetTriggerType() == 1) {
			auto outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
			MoveDoor_Struct *md = (MoveDoor_Struct *) outapp->pBuffer;
			md->doorid = door_id;
			md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
			entity_list.QueueClients(0, outapp);
			safe_delete(outapp);
		}

		triggered = false;
		close_timer.Disable();
		SetOpenState(false);
	}
	return true;
}

void Doors::HandleClick(Client* sender, uint8 trigger) {
	Log(Logs::Detail, Logs::Doors,
	    "%s clicked door %s (dbid %d, eqid %d) at %s",
	    sender->GetName(),
	    door_name,
	    database_id,
	    door_id,
	    to_string(m_Position).c_str()
	);

	Log(Logs::Detail, Logs::Doors,
	    "incline %d, open_type %d, lockpick %d, key %d, nokeyring %d, trigger %d type %d, param %d",
	    incline,
	    open_type,
	    lockpick,
	    key_item_id,
	    no_key_ring,
	    trigger_door,
	    trigger_type,
	    door_param
	);

	Log(Logs::Detail, Logs::Doors,
	    "disable_timer '%s',size %d, invert %d, dest: %s %s",
	    (disable_timer ? "true" : "false"),
	    size,
	    invert_state,
	    destination_zone_name,
	    to_string(m_Destination).c_str()
	);

	auto outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	auto *move_door_packet = (MoveDoor_Struct *) outapp->pBuffer;
	move_door_packet->doorid = door_id;

	if (IsLDoNDoor()) {
		if (sender) {
			if (RuleI(Adventure, ItemIDToEnablePorts) != 0) {
				if (!sender->KeyRingCheck(RuleI(Adventure, ItemIDToEnablePorts))) {
					if (sender->GetInv().HasItem(RuleI(Adventure, ItemIDToEnablePorts)) == INVALID_INDEX) {
						sender->MessageString(Chat::Red, DUNGEON_SEALED);
						safe_delete(outapp);
						return;
					} else {
						sender->KeyRingAdd(RuleI(Adventure, ItemIDToEnablePorts));
					}
				}
			}

			if (!sender->GetPendingAdventureDoorClick()) {
				sender->PendingAdventureDoorClick();
				auto pack = new ServerPacket(
						ServerOP_AdventureClickDoor,
						sizeof(ServerPlayerClickedAdventureDoor_Struct)
				);

				/**
				 * Adventure door
				 */
				ServerPlayerClickedAdventureDoor_Struct *adventure_door_click;
				adventure_door_click = (ServerPlayerClickedAdventureDoor_Struct *) pack->pBuffer;
				strcpy(adventure_door_click->player, sender->GetName());

				adventure_door_click->zone_id = zone->GetZoneID();
				adventure_door_click->id      = GetDoorDBID();

				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
			safe_delete(outapp);
			return;
		}
	}

	// todo: if IsDzDoor() call Client::MovePCDynamicZone(target_zone_id) (for systems that use dzs)

	uint32 required_key_item       = GetKeyItem();
	uint8  disable_add_to_key_ring = GetNoKeyring();
	bool player_has_key            = false;
	uint32 player_key              = 0;

	const EQ::ItemInstance *lock_pick_item = sender->GetInv().GetItem(EQ::invslot::slotCursor);

	// If classic key on cursor rule, check for it, otherwise owning it ok.
	if (RuleB(Doors, RequireKeyOnCursor)) {
		if (lock_pick_item != nullptr &&
			lock_pick_item->GetItem()->ID == required_key_item) {
			player_has_key = true;
		}
	}
	else if (sender->GetInv().HasItem(required_key_item, 1) != INVALID_INDEX) {
		player_has_key = true;
	}

	if (player_has_key) {
		player_key = required_key_item;
	}

	/**
	 * Object is not triggered
	 */
	if (GetTriggerType() == 255) {

		/**
		 * Door is only triggered by an object
		 */
		if (trigger == 1) {
			if (!IsDoorOpen() || (open_type == 58)) {
				move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
			} else {
				move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
			}
		} else {
			safe_delete(outapp);
			return;
		}
	}

	/**
	 * Guild Doors
	 *
	 * Door is not locked
	 */
	bool is_guild_door = (GetGuildID() > 0) && (GetGuildID() == sender->GuildID());
	bool is_door_not_locked = ((required_key_item == 0) && (GetLockpick() == 0) && (GetGuildID() == 0));
	bool is_door_open_and_open_able = (IsDoorOpen() && (open_type == 58));

	if (is_door_not_locked || is_door_open_and_open_able || is_guild_door) {
		if (!IsDoorOpen() || (GetOpenType() == 58)) {
			move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
		} else {
			move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
		}
	} else {

		/**
		 * Guild Doors
		 */
		if ((GetGuildID() > 0) && !sender->GetGM()) {
			std::string guild_name;
			char        door_message[240];

			if (guild_mgr.GetGuildNameByID(guild_id, guild_name)) {
				sprintf(door_message, "Only members of the <%s> guild may enter here", guild_name.c_str());
			} else {
				strcpy(door_message, "Door is locked by an unknown guild");
			}

			sender->Message(Chat::LightBlue, door_message);
			safe_delete(outapp);
			return;
		}

		/**
		 * Key required
		 * If using a lock_pick_item leave messaging to that code below
		 */
		if (lock_pick_item == nullptr && !IsDoorOpen()) {
			sender->Message(Chat::LightBlue, "This is locked...");
		}

		/**
		 * GM can always open locks
		 */
		if (sender->GetGM()) {
			sender->MessageString(Chat::LightBlue, DOORS_GM);

			if (!IsDoorOpen() || (open_type == 58)) {
				move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
			} else {
				move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
			}

		}

		/**
		 * Player has something they are trying to open it with
		 */
		else if (player_key) {

			/**
			 * Key required and client is using the right key
			 */
			if (required_key_item &&
			    (required_key_item == player_key)) {

				if (!disable_add_to_key_ring) {
					sender->KeyRingAdd(player_key);
				}

				sender->Message(Chat::LightBlue, "You got it open!");

				if (!IsDoorOpen() || (open_type == 58)) {
					move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
				} else {
					move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
				}
			}
		}

		/**
		 * Try Lock pick
		 */
		else if (lock_pick_item != nullptr) {
			if (sender->GetSkill(EQ::skills::SkillPickLock)) {
				Timer* pick_lock_timer = sender->GetPickLockTimer();
				if (lock_pick_item->GetItem()->ItemType == EQ::item::ItemTypeLockPick) {
					if (!pick_lock_timer->Check()) {
						// Stop full scale mad spamming
						safe_delete(outapp);
						return;
					}

					float player_pick_lock_skill = sender->GetSkill(EQ::skills::SkillPickLock);

					LogSkills("Client has lockpicks: skill=[{}]", player_pick_lock_skill);

					if (GetLockpick() <= player_pick_lock_skill) {

						// Stop full scale spamming
						pick_lock_timer->Start(1000, true);

						if (!IsDoorOpen()) {
							sender->CheckIncreaseSkill(EQ::skills::SkillPickLock, nullptr, 1);
							move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
							sender->MessageString(Chat::LightBlue, DOORS_SUCCESSFUL_PICK);
						} else {
							move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
						}
					} else {
						sender->MessageString(Chat::LightBlue, DOORS_INSUFFICIENT_SKILL);
						safe_delete(outapp);
						return;
					}
				} else {
					sender->MessageString(Chat::LightBlue, DOORS_NO_PICK);
					safe_delete(outapp);
					return;
				}
			} else {
				sender->MessageString(Chat::LightBlue, DOORS_CANT_PICK);
				safe_delete(outapp);
				return;
			}
		}

		/**
		 * Locked door and nothing to open it with
		 */
		else {

			/**
			 * Search for key on keyring
			 */
			if (sender->KeyRingCheck(required_key_item)) {
				player_key = required_key_item;
				sender->Message(Chat::LightBlue, "You got it open!"); // more debug spam
				if (!IsDoorOpen() || (open_type == 58)) {
					move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
				} else {
					move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
				}
			} else {
				sender->MessageString(Chat::LightBlue, DOORS_LOCKED);
				safe_delete(outapp);
				return;
			}
		}
	}

	entity_list.QueueClients(sender, outapp, false);
	if (!IsDoorOpen() || (open_type == 58)) {
		if (!disable_timer)
			close_timer.Start();

		if(strncmp(destination_zone_name, "NONE", strlen("NONE")) == 0)
			SetOpenState(true);
	} else {
		close_timer.Disable();
		if (!disable_timer)
			SetOpenState(false);
	}

	/*
	 * Everything past this point assumes we opened the door
	 *  and met all the requirements for opening
	 *  everything to do with closed doors has already been taken care of
	 *  we return because we don't want people using teleports on an unlocked door (exploit!)
	 */

	if ((move_door_packet->action == CLOSE_DOOR && invert_state == 0) || (move_door_packet->action == CLOSE_INVDOOR && invert_state == 1)) {
		safe_delete(outapp);
		return;
	}

	safe_delete(outapp);

	if ((GetTriggerDoorID() != 0) && (GetTriggerType() == 1)) {
		Doors *trigger_door_entity = entity_list.FindDoor(GetTriggerDoorID());
		if (trigger_door_entity && !trigger_door_entity->triggered) {
			triggered = true;
			trigger_door_entity->HandleClick(sender, 1);
		} else {
			triggered = false;
		}
	} else if ((GetTriggerDoorID() != 0) && (GetTriggerType() != 1)) {
		Doors *trigger_door_entity = entity_list.FindDoor(GetTriggerDoorID());
		if (trigger_door_entity && !trigger_door_entity->triggered) {
			triggered = true;
			trigger_door_entity->HandleClick(sender, 0);
		} else {
			triggered = false;
		}
	}

	/**
	 * Teleport door
	 */
	if (((open_type == 57) || (open_type == 58)) &&
	    (strncmp(destination_zone_name, "NONE", strlen("NONE")) != 0)) {

		/**
		 * If click destination is same zone and doesn't require a key
		 */
		if ((strncmp(destination_zone_name, zone_name, strlen(zone_name)) == 0) && (!required_key_item)) {
			if (!disable_add_to_key_ring) {
				sender->KeyRingAdd(player_key);
			}
			sender->MovePC(
					zone->GetZoneID(),
					zone->GetInstanceID(),
					m_Destination.x,
					m_Destination.y,
					m_Destination.z,
					m_Destination.w
			);
		}
		/**
		 * If requires a key
		 */
		else if (
				(!IsDoorOpen() || open_type == 58) &&
				(required_key_item && ((required_key_item == player_key) || sender->GetGM()))
		) {

			if (!disable_add_to_key_ring) {
				sender->KeyRingAdd(player_key);
			}
			if (ZoneID(destination_zone_name) == zone->GetZoneID()) {
				sender->MovePC(
						zone->GetZoneID(),
						zone->GetInstanceID(),
						m_Destination.x,
						m_Destination.y,
						m_Destination.z,
						m_Destination.w
				);
			} else {
				sender->MovePC(
						ZoneID(destination_zone_name),
						static_cast<uint32>(destination_instance_id),
						m_Destination.x,
						m_Destination.y,
						m_Destination.z,
						m_Destination.w
				);
			}
		}

		if ((!IsDoorOpen() || open_type == 58) && (!required_key_item)) {
			if (ZoneID(destination_zone_name) == zone->GetZoneID()) {
				sender->MovePC(
						zone->GetZoneID(),
						zone->GetInstanceID(),
						m_Destination.x,
						m_Destination.y,
						m_Destination.z,
						m_Destination.w
				);
			} else {
				sender->MovePC(
					ZoneID(destination_zone_name),
						static_cast<uint32>(destination_instance_id),
						m_Destination.x,
						m_Destination.y,
						m_Destination.z,
						m_Destination.w
				);
			}
		}
	}
}

void Doors::Open(Mob* sender, bool alt_mode)
{
	if (sender) {
		if (GetTriggerType() == 255 || GetTriggerDoorID() > 0 || GetLockpick() != 0 || GetKeyItem() != 0 || open_type == 59 || open_type == 58 || !sender->IsNPC()) { // this object isnt triggered or door is locked - NPCs should not open locked doors!
			return;
		}

		auto outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
		MoveDoor_Struct* md = (MoveDoor_Struct*)outapp->pBuffer;
		md->doorid = door_id;
		md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
		entity_list.QueueCloseClients(sender, outapp, false, 200);
		safe_delete(outapp);

		if (!alt_mode) { // original function
			if (!is_open) {
				if (!disable_timer)
					close_timer.Start();
				is_open = true;
			}
			else {
				close_timer.Disable();
				if (!disable_timer)
					is_open = false;
			}
		}
		else { // alternative function
			if (!disable_timer)
				close_timer.Start();
			is_open = true;
		}
	}
}

void Doors::ForceOpen(Mob *sender, bool alt_mode)
{
	auto outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct* md = (MoveDoor_Struct*)outapp->pBuffer;
	md->doorid = door_id;
	md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
	entity_list.QueueClients(sender, outapp, false);
	safe_delete(outapp);

	if (!alt_mode) { // original function
		if (!is_open) {
			if (!disable_timer)
				close_timer.Start();
			is_open = true;
		}
		else {
			close_timer.Disable();
			if (!disable_timer)
				is_open = false;
		}
	}
	else { // alternative function
		if (!disable_timer)
			close_timer.Start();
		is_open = true;
	}
}

void Doors::ForceClose(Mob *sender, bool alt_mode) {
	auto outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct *move_door_packet;
	move_door_packet = (MoveDoor_Struct *) outapp->pBuffer;
	move_door_packet->doorid = door_id;
	move_door_packet->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR; // change from original (open to close)
	entity_list.QueueClients(sender, outapp, false);
	safe_delete(outapp);

	if (!alt_mode) { // original function
		if (!is_open) {
			if (!disable_timer)
				close_timer.Start();
			is_open = true;
		} else {
			close_timer.Disable();
			is_open = false;
		}
	} else { // alternative function
		if (is_open)
			close_timer.Trigger();
	}
}

void Doors::ToggleState(Mob *sender)
{
	if(GetTriggerDoorID() > 0 || GetLockpick() != 0 || GetKeyItem() != 0 || open_type == 58 || open_type == 40) { // borrowed some NPCOpen criteria
		return;
	}

	auto outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct* move_door_packet;
	move_door_packet = (MoveDoor_Struct*)outapp->pBuffer;
	move_door_packet->doorid = door_id;

	if(!is_open) {
		move_door_packet->action = static_cast<uint8>(invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
		is_open=true;
	}
	else {
		move_door_packet->action = static_cast<uint8>(invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
		is_open=false;
	}

	entity_list.QueueClients(sender,outapp,false);
	safe_delete(outapp);
}

int32 ZoneDatabase::GetDoorsCount(uint32* oMaxID, const char *zone_name, int16 version) {

	std::string query = StringFormat("SELECT MAX(id), count(*) FROM doors "
                                    "WHERE zone = '%s' AND (version = %u OR version = -1)",
                                    zone_name, version);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return -1;
    }

    if (results.RowCount() != 1)
        return -1;

    auto row = results.begin();

    if (!oMaxID)
        return atoi(row[1]);

    if (row[0])
        *oMaxID = atoi(row[0]);
    else
        *oMaxID = 0;

    return atoi(row[1]);

}

int32 ZoneDatabase::GetDoorsCountPlusOne(const char *zone_name, int16 version) {
    std::string query = StringFormat(
    		"SELECT MAX(id) FROM doors WHERE zone = '%s' AND version = %u",
		    zone_name,
		    version
    );
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return -1;
    }

	if (results.RowCount() != 1)
        return -1;

    auto row = results.begin();

    if (!row[0])
        return 0;

    return atoi(row[0]) + 1;
}

int32 ZoneDatabase::GetDoorsDBCountPlusOne(const char *zone_name, int16 version) {

	uint32 oMaxID = 0;

    std::string query = StringFormat("SELECT MAX(doorid) FROM doors "
                                    "WHERE zone = '%s' AND (version = %u OR version = -1)",
                                    zone_name, version);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return -1;
	}

    if (results.RowCount() != 1)
        return -1;

    auto row = results.begin();

    if (!row[0])
        return 0;

    return atoi(row[0]) + 1;
}

std::vector<DoorsRepository::Doors> ZoneDatabase::LoadDoors(const std::string& zone_name, int16 version)
{
	LogInfo("Loading Doors from database");

	auto door_entries = DoorsRepository::GetWhere(*this, fmt::format(
		"zone = '{}' AND (version = {} OR version = -1) {} ORDER BY doorid ASC",
		zone_name, version, ContentFilterCriteria::apply()));

	LogDoors("Loaded [{}] doors for [{}] version [{}]", door_entries.size(), zone_name, version);

	return door_entries;
}

void Doors::SetLocation(float x, float y, float z)
{
	entity_list.DespawnAllDoors();
    m_Position = glm::vec4(x, y, z, m_Position.w);
	entity_list.RespawnAllDoors();
}

void Doors::SetPosition(const glm::vec4& position) {
	entity_list.DespawnAllDoors();
	m_Position = position;
	entity_list.RespawnAllDoors();
}

void Doors::SetIncline(int in) {
	entity_list.DespawnAllDoors();
	incline = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetInvertState(int in) {
	entity_list.DespawnAllDoors();
	invert_state = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetOpenType(uint8 in) {
	entity_list.DespawnAllDoors();
	open_type = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetDoorName(const char* name) {
	entity_list.DespawnAllDoors();
	memset(door_name, 0, sizeof(door_name));
	strncpy(door_name, name, sizeof(door_name));
	entity_list.RespawnAllDoors();
}

void Doors::SetSize(uint16 in) {
	entity_list.DespawnAllDoors();
	size = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetDisableTimer(bool flag) {
	disable_timer = flag;
}

void Doors::CreateDatabaseEntry()
{
	if (content_db.GetDoorsDBCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion()) - 1 >= 255) {
		return;
	}

	/**
	 * Persist
	 */
	content_db.InsertDoor(
		GetDoorDBID(),
		GetDoorID(),
		GetDoorName(),
		m_Position,
		GetOpenType(),
		static_cast<uint16>(GetGuildID()),
		GetLockpick(),
		GetKeyItem(),
		static_cast<uint8>(GetDoorParam()),
		static_cast<uint8>(GetInvertState()),
		GetIncline(),
		GetSize()
	);
}

float Doors::GetX()
{
	return m_Position.x;
}

float Doors::GetY()
{
	return m_Position.y;
}

float Doors::GetZ()
{
	return m_Position.z;
}
