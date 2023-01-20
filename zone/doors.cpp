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
#include "../common/strings.h"

#include "../common/repositories/doors_repository.h"

#include "client.h"
#include "doors.h"
#include "entity.h"
#include "guild_mgr.h"
#include "mob.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zonedb.h"
#include "../common/zone_store.h"
#include "../common/repositories/criteria/content_filter_criteria.h"

#include <iostream>
#include <string.h>

#define OPEN_DOOR 0x02
#define CLOSE_DOOR 0x03
#define OPEN_INVDOOR 0x03
#define CLOSE_INVDOOR 0x02

extern EntityList  entity_list;
extern WorldServer worldserver;

Doors::Doors(const DoorsRepository::Doors &door) :
	m_close_timer(5000),
	m_position(door.pos_x, door.pos_y, door.pos_z, door.heading),
	m_destination(door.dest_x, door.dest_y, door.dest_z, door.dest_heading)
{
	strn0cpy(m_zone_name, door.zone.c_str(), sizeof(m_zone_name));
	strn0cpy(m_door_name, door.name.c_str(), sizeof(m_door_name));
	strn0cpy(m_destination_zone_name, door.dest_zone.c_str(), sizeof(m_destination_zone_name));

	m_database_id             = door.id;
	m_door_id                 = door.doorid;
	m_incline                 = door.incline;
	m_open_type               = door.opentype;
	m_guild_id                = door.guild;
	m_lockpick                = door.lockpick;
	m_key_item_id             = door.keyitem;
	m_no_key_ring             = door.nokeyring;
	m_trigger_door            = door.triggerdoor;
	m_trigger_type            = door.triggertype;
	triggered                 = false;
	m_door_param              = door.door_param;
	m_size                    = door.size;
	m_invert_state            = door.invert_state;
	m_destination_instance_id = door.dest_instance;
	m_is_ldon_door            = door.is_ldon_door;
	m_dz_switch_id            = door.dz_switch_id;
	m_client_version_mask     = door.client_version_mask;

	SetOpenState(false);

	m_close_timer.Disable();

	m_disable_timer = (door.disable_timer == 1 ? true : false);
}

Doors::Doors(const char *model, const glm::vec4 &position, uint8 open_type, uint16 size) :
	m_close_timer(5000),
	m_position(position),
	m_destination(glm::vec4())
{

	strn0cpy(m_zone_name, zone->GetShortName(), 32);
	strn0cpy(m_door_name, model, 32);
	strn0cpy(m_destination_zone_name, "NONE", 32);

	m_database_id = content_db.GetDoorsCountPlusOne();
	m_door_id     = content_db.GetDoorsDBCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion());

	m_open_type               = open_type;
	m_size                    = size;
	m_incline                 = 0;
	m_guild_id                = 0;
	m_lockpick                = 0;
	m_key_item_id             = 0;
	m_no_key_ring             = 0;
	m_trigger_door            = 0;
	m_trigger_type            = 0;
	triggered                 = false;
	m_door_param              = 0;
	m_invert_state            = 0;
	m_is_ldon_door            = 0;
	m_client_version_mask     = 4294967295u;
	m_disable_timer           = 0;
	m_destination_instance_id = 0;

	SetOpenState(false);
	m_close_timer.Disable();
}


Doors::~Doors()
{
}

bool Doors::Process()
{
	if (m_close_timer.Enabled() && m_close_timer.Check() && IsDoorOpen()) {
		if (m_open_type == 40 || GetTriggerType() == 1) {
			auto            outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
			MoveDoor_Struct *md    = (MoveDoor_Struct *) outapp->pBuffer;
			md->doorid = m_door_id;
			md->action = m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
			entity_list.QueueClients(0, outapp);
			safe_delete(outapp);
		}

		triggered = false;
		m_close_timer.Disable();
		SetOpenState(false);
	}
	return true;
}

void Doors::HandleClick(Client *sender, uint8 trigger)
{
	Log(Logs::Detail, Logs::Doors,
		"%s clicked door %s (dbid %d, eqid %d) at %s",
		sender->GetName(),
		m_door_name,
		m_database_id,
		m_door_id,
		to_string(m_position).c_str()
	);

	Log(Logs::Detail, Logs::Doors,
		"incline %d, open_type %d, lockpick %d, key %d, nokeyring %d, trigger %d type %d, param %d",
		m_incline,
		m_open_type,
		m_lockpick,
		m_key_item_id,
		m_no_key_ring,
		m_trigger_door,
		m_trigger_type,
		m_door_param
	);

	Log(Logs::Detail, Logs::Doors,
		"disable_timer '%s',size %d, invert %d, dest: %s %s",
		(m_disable_timer ? "true" : "false"),
		m_size,
		m_invert_state,
		m_destination_zone_name,
		to_string(m_destination).c_str()
	);

	auto outapp            = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	auto *move_door_packet = (MoveDoor_Struct *) outapp->pBuffer;
	move_door_packet->doorid = m_door_id;

	if (IsLDoNDoor()) {
		if (sender) {
			if (RuleI(Adventure, ItemIDToEnablePorts) != 0) {
				if (!sender->KeyRingCheck(RuleI(Adventure, ItemIDToEnablePorts))) {
					if (sender->GetInv().HasItem(RuleI(Adventure, ItemIDToEnablePorts)) == INVALID_INDEX) {
						sender->MessageString(Chat::Red, DUNGEON_SEALED);
						safe_delete(outapp);
						return;
					}
					else {
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

	if (m_dz_switch_id != 0) {
		sender->UpdateTasksOnTouchSwitch(m_dz_switch_id);
		if (sender->TryMovePCDynamicZoneSwitch(m_dz_switch_id)) {
			safe_delete(outapp);
			return;
		}
	}

	uint32 required_key_item       = GetKeyItem();
	uint8  disable_add_to_key_ring = GetNoKeyring();
	bool   player_has_key          = false;
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
			if (!IsDoorOpen() || (m_open_type == 58)) {
				move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
			}
			else {
				move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
			}
		}
		else {
			safe_delete(outapp);
			return;
		}
	}

	// enforce flags before they hit zoning process
	auto z = GetZone(m_destination_zone_name, 0);
	if (z && !z->flag_needed.empty() && Strings::IsNumber(z->flag_needed) && std::stoi(z->flag_needed) == 1) {
		if (sender->Admin() < minStatusToIgnoreZoneFlags && !sender->HasZoneFlag(z->zoneidnumber)) {
			LogInfo(
				"Character [{}] does not have the flag to be in this zone [{}]!",
				sender->GetCleanName(),
				z->flag_needed
			);
			sender->MessageString(Chat::LightBlue, DOORS_LOCKED);
		}
	}

	/**
	 * Guild Doors
	 *
	 * Door is not locked
	 */
	bool is_guild_door              = (GetGuildID() > 0) && (GetGuildID() == sender->GuildID());
	bool is_door_not_locked         = ((required_key_item == 0) && (GetLockpick() == 0) && (GetGuildID() == 0));
	bool is_door_open_and_open_able = (IsDoorOpen() && (m_open_type == 58));

	if (is_door_not_locked || is_door_open_and_open_able || is_guild_door) {
		if (!IsDoorOpen() || (GetOpenType() == 58)) {
			move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
		}
		else {
			move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
		}
	}
	else {

		/**
		 * Guild Doors
		 */
		if ((GetGuildID() > 0) && !sender->GetGM()) {
			std::string guild_name;
			char        door_message[240];

			if (guild_mgr.GetGuildNameByID(m_guild_id, guild_name)) {
				sprintf(door_message, "Only members of the <%s> guild may enter here", guild_name.c_str());
			}
			else {
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

			if (!IsDoorOpen() || (m_open_type == 58)) {
				move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
			}
			else {
				move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
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

				if (!IsDoorOpen() || (m_open_type == 58)) {
					move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
				}
				else {
					move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
				}
			}
		}

			/**
			 * Try Lock pick
			 */
		else if (lock_pick_item != nullptr) {
			if (sender->GetSkill(EQ::skills::SkillPickLock)) {
				Timer *pick_lock_timer = sender->GetPickLockTimer();
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
							move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR
								: OPEN_INVDOOR);
							sender->MessageString(Chat::LightBlue, DOORS_SUCCESSFUL_PICK);
						}
						else {
							move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR
								: CLOSE_INVDOOR);
						}
					}
					else {
						sender->MessageString(Chat::LightBlue, DOORS_INSUFFICIENT_SKILL);
						safe_delete(outapp);
						return;
					}
				}
				else {
					sender->MessageString(Chat::LightBlue, DOORS_NO_PICK);
					safe_delete(outapp);
					return;
				}
			}
			else {
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
				if (!IsDoorOpen() || (m_open_type == 58)) {
					move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
				}
				else {
					move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
				}
			}
			else {
				sender->MessageString(Chat::LightBlue, DOORS_LOCKED);
				safe_delete(outapp);
				return;
			}
		}
	}

	entity_list.QueueClients(sender, outapp, false);
	if (!IsDoorOpen() || (m_open_type == 58)) {
		if (!m_disable_timer) {
			m_close_timer.Start();
		}

		if (strncmp(m_destination_zone_name, "NONE", strlen("NONE")) == 0) {
			SetOpenState(true);
		}
	}
	else {
		m_close_timer.Disable();
		if (!m_disable_timer) {
			SetOpenState(false);
		}
	}

	/*
	 * Everything past this point assumes we opened the door
	 *  and met all the requirements for opening
	 *  everything to do with closed doors has already been taken care of
	 *  we return because we don't want people using teleports on an unlocked door (exploit!)
	 */

	if ((move_door_packet->action == CLOSE_DOOR && m_invert_state == 0) ||
		(move_door_packet->action == CLOSE_INVDOOR && m_invert_state == 1)) {
		safe_delete(outapp);
		return;
	}

	safe_delete(outapp);

	if ((GetTriggerDoorID() != 0) && (GetTriggerType() == 1)) {
		Doors *trigger_door_entity = entity_list.FindDoor(GetTriggerDoorID());
		if (trigger_door_entity && !trigger_door_entity->triggered) {
			triggered = true;
			trigger_door_entity->HandleClick(sender, 1);
		}
		else {
			triggered = false;
		}
	}
	else if ((GetTriggerDoorID() != 0) && (GetTriggerType() != 1)) {
		Doors *trigger_door_entity = entity_list.FindDoor(GetTriggerDoorID());
		if (trigger_door_entity && !trigger_door_entity->triggered) {
			triggered = true;
			trigger_door_entity->HandleClick(sender, 0);
		}
		else {
			triggered = false;
		}
	}

	/**
	 * Teleport door
	 */
	if (((m_open_type == 57) || (m_open_type == 58)) &&
		(strncmp(m_destination_zone_name, "NONE", strlen("NONE")) != 0)) {

		/**
		 * If click destination is same zone and doesn't require a key
		 */
		if ((strncmp(m_destination_zone_name, m_zone_name, strlen(m_zone_name)) == 0) && (!required_key_item)) {
			if (!disable_add_to_key_ring) {
				sender->KeyRingAdd(player_key);
			}
			sender->MovePC(
				zone->GetZoneID(),
				zone->GetInstanceID(),
				m_destination.x,
				m_destination.y,
				m_destination.z,
				m_destination.w
			);
		}
			/**
			 * If requires a key
			 */
		else if (
			(!IsDoorOpen() || m_open_type == 58) &&
			(required_key_item && ((required_key_item == player_key) || sender->GetGM()))
			) {

			if (!disable_add_to_key_ring) {
				sender->KeyRingAdd(player_key);
			}
			if (ZoneID(m_destination_zone_name) == zone->GetZoneID()) {
				sender->MovePC(
					zone->GetZoneID(),
					zone->GetInstanceID(),
					m_destination.x,
					m_destination.y,
					m_destination.z,
					m_destination.w
				);
			}
			else {
				sender->MovePC(
					ZoneID(m_destination_zone_name),
					static_cast<uint32>(m_destination_instance_id),
					m_destination.x,
					m_destination.y,
					m_destination.z,
					m_destination.w
				);
			}
		}

		if ((!IsDoorOpen() || m_open_type == 58) && (!required_key_item)) {
			if (ZoneID(m_destination_zone_name) == zone->GetZoneID()) {
				sender->MovePC(
					zone->GetZoneID(),
					zone->GetInstanceID(),
					m_destination.x,
					m_destination.y,
					m_destination.z,
					m_destination.w
				);
			}
			else {
				sender->MovePC(
					ZoneID(m_destination_zone_name),
					static_cast<uint32>(m_destination_instance_id),
					m_destination.x,
					m_destination.y,
					m_destination.z,
					m_destination.w
				);
			}
		}
	}
}

void Doors::Open(Mob *sender, bool alt_mode)
{
	if (sender) {
		if (GetTriggerType() == 255 || GetTriggerDoorID() > 0 || GetLockpick() != 0 || GetKeyItem() != 0 ||
			m_open_type == 59 || m_open_type == 58 ||
			!sender->IsNPC()) { // this object isnt triggered or door is locked - NPCs should not open locked doors!
			return;
		}

		auto            outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
		MoveDoor_Struct *md    = (MoveDoor_Struct *) outapp->pBuffer;
		md->doorid = m_door_id;
		md->action = m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
		entity_list.QueueCloseClients(sender, outapp, false, 200);
		safe_delete(outapp);

		if (!alt_mode) { // original function
			if (!m_is_open) {
				if (!m_disable_timer) {
					m_close_timer.Start();
				}
				m_is_open = true;
			}
			else {
				m_close_timer.Disable();
				if (!m_disable_timer) {
					m_is_open = false;
				}
			}
		}
		else { // alternative function
			if (!m_disable_timer) {
				m_close_timer.Start();
			}
			m_is_open = true;
		}
	}
}

void Doors::ForceOpen(Mob *sender, bool alt_mode)
{
	auto            outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct *md    = (MoveDoor_Struct *) outapp->pBuffer;
	md->doorid = m_door_id;
	md->action = m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
	entity_list.QueueClients(sender, outapp, false);
	safe_delete(outapp);

	if (!alt_mode) { // original function
		if (!m_is_open) {
			if (!m_disable_timer) {
				m_close_timer.Start();
			}
			m_is_open = true;
		}
		else {
			m_close_timer.Disable();
			if (!m_disable_timer) {
				m_is_open = false;
			}
		}
	}
	else { // alternative function
		if (!m_disable_timer) {
			m_close_timer.Start();
		}
		m_is_open = true;
	}
}

void Doors::ForceClose(Mob *sender, bool alt_mode)
{
	auto            outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct *move_door_packet;
	move_door_packet = (MoveDoor_Struct *) outapp->pBuffer;
	move_door_packet->doorid = m_door_id;
	move_door_packet->action = m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR; // change from original (open to close)
	entity_list.QueueClients(sender, outapp, false);
	safe_delete(outapp);

	if (!alt_mode) { // original function
		if (!m_is_open) {
			if (!m_disable_timer) {
				m_close_timer.Start();
			}
			m_is_open = true;
		}
		else {
			m_close_timer.Disable();
			m_is_open = false;
		}
	}
	else { // alternative function
		if (m_is_open) {
			m_close_timer.Trigger();
		}
	}
}

void Doors::ToggleState(Mob *sender)
{
	if (GetTriggerDoorID() > 0 || GetLockpick() != 0 || GetKeyItem() != 0 || m_open_type == 58 ||
		m_open_type == 40) { // borrowed some NPCOpen criteria
		return;
	}

	auto            outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct *move_door_packet;
	move_door_packet = (MoveDoor_Struct *) outapp->pBuffer;
	move_door_packet->doorid = m_door_id;

	if (!m_is_open) {
		move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR);
		m_is_open = true;
	}
	else {
		move_door_packet->action = static_cast<uint8>(m_invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR);
		m_is_open = false;
	}

	entity_list.QueueClients(sender, outapp, false);
	safe_delete(outapp);
}

uint32 ZoneDatabase::GetDoorsCountPlusOne()
{
	const uint32 max_door_id = static_cast<uint32>(DoorsRepository::GetMaxId(*this));

	return (max_door_id + 1);
}

int ZoneDatabase::GetDoorsDBCountPlusOne(std::string zone_short_name, int16 version)
{
	const auto query = fmt::format(
		"SELECT COALESCE(MAX(doorid), 1) FROM doors "
		"WHERE zone = '{}' AND (version = {} OR version = -1)",
		zone_short_name,
		version
	);
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return -1;
	}

	auto row = results.begin();

	if (!row[0]) {
		return 0;
	}

	return std::stoi(row[0]) + 1;
}

std::vector<DoorsRepository::Doors> ZoneDatabase::LoadDoors(const std::string &zone_name, int16 version)
{
	auto door_entries = DoorsRepository::GetWhere(
		*this, fmt::format(
			"zone = '{}' AND (version = {} OR version = -1) {} ORDER BY doorid ASC",
			zone_name, version, ContentFilterCriteria::apply()));

	LogDoors("Loaded [{}] doors for [{}] version [{}]", Strings::Commify(door_entries.size()), zone_name, version);

	return door_entries;
}

void Doors::SetLocation(float x, float y, float z)
{
	entity_list.DespawnAllDoors();
	m_position = glm::vec4(x, y, z, m_position.w);
	entity_list.RespawnAllDoors();
}

void Doors::SetPosition(const glm::vec4 &position)
{
	entity_list.DespawnAllDoors();
	m_position = position;
	entity_list.RespawnAllDoors();
}

void Doors::SetIncline(int in)
{
	entity_list.DespawnAllDoors();
	m_incline = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetInvertState(int in)
{
	entity_list.DespawnAllDoors();
	m_invert_state = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetOpenType(uint8 in)
{
	entity_list.DespawnAllDoors();
	m_open_type = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetDoorName(const char *name)
{
	entity_list.DespawnAllDoors();
	memset(m_door_name, 0, sizeof(m_door_name));
	strncpy(m_door_name, name, sizeof(m_door_name));
	entity_list.RespawnAllDoors();
}

void Doors::SetSize(uint16 in)
{
	entity_list.DespawnAllDoors();
	m_size = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetDisableTimer(bool flag)
{
	m_disable_timer = flag;
}

void Doors::CreateDatabaseEntry()
{
	if (content_db.GetDoorsDBCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion()) - 1 >= 255) {
		return;
	}

	content_db.InsertDoor(
		GetDoorDBID(),
		GetDoorID(),
		GetDoorName(),
		m_position,
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
	return m_position.x;
}

float Doors::GetY()
{
	return m_position.y;
}

float Doors::GetZ()
{
	return m_position.z;
}
