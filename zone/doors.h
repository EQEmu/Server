#ifndef DOORS_H
#define DOORS_H

#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/linked_list.h"

#include "mob.h"
#include "zonedump.h"

class Client;
class Mob;
class NPC;
struct Door;

class Doors : public Entity
{
public:
	~Doors();

	Doors(const char *model, const glm::vec4& position, uint8 open_type = 58, uint16 size = 100);
	Doors(const Door* door);

	bool GetDisableTimer() { return disable_timer; }
	bool IsDoor() const { return true; }
	bool IsDoorOpen() { return is_open; }
	bool Process();
	bool triggered;
	char *GetDoorName() { return door_name; }
	const glm::vec4 GetDestination() const { return m_Destination; }
	const glm::vec4 &GetPosition() const { return m_Position; }
	int GetIncline() { return incline; }
	int GetInvertState() { return invert_state; }
	uint8 GetDoorID() { return door_id; }
	uint8 GetNoKeyring() { return no_key_ring; }
	uint8 GetOpenType() { return open_type; }
	uint8 GetTriggerDoorID() { return trigger_door; }
	uint8 GetTriggerType() { return trigger_type; }
	uint8 IsLDoNDoor() { return is_ldon_door; }
	uint16 GetLockpick() { return lockpick; }
	uint16 GetSize() { return size; }
	uint32 GetClientVersionMask() { return client_version_mask; }
	uint32 GetDoorDBID() { return database_id; }
	uint32 GetDoorParam() { return door_param; }
	uint32 GetEntityID() { return entity_id; }
	uint32 GetGuildID() { return guild_id; }
	uint32 GetKeyItem() { return key_item_id; }
	void CreateDatabaseEntry();
	void ForceClose(Mob *sender, bool alt_mode = false);
	void ForceOpen(Mob *sender, bool alt_mode = false);
	void HandleClick(Client *sender, uint8 trigger);
	void NPCOpen(NPC *sender, bool alt_mode = false);
	void SetDisableTimer(bool flag);
	void SetDoorName(const char *name);
	void SetEntityID(uint32 entity) { entity_id = entity; }
	void SetIncline(int in);
	void SetKeyItem(uint32 in) { key_item_id = in; }
	void SetLocation(float x, float y, float z);
	void SetLockpick(uint16 in) { lockpick = in; }
	void SetNoKeyring(uint8 in) { no_key_ring = in; }
	void SetOpenState(bool st) { is_open = st; }
	void SetOpenType(uint8 in);
	void SetPosition(const glm::vec4 &position);
	void SetSize(uint16 size);
	void ToggleState(Mob *sender);

	float GetX();
	float GetY();
	float GetZ();

private:

	uint32    database_id;
	uint8     door_id;
	char      zone_name[32];
	char      door_name[32];
	glm::vec4 m_Position;
	int       incline;
	uint8     open_type;
	uint32    guild_id;
	uint16    lockpick;
	uint32    key_item_id;
	uint8     no_key_ring;
	uint8     trigger_door;
	uint8     trigger_type;
	uint32    door_param;
	uint16    size;
	int       invert_state;
	uint32    entity_id;
	bool      disable_timer;
	bool      is_open;
	Timer     close_timer;
	char      destination_zone_name[16];
	int       destination_instance_id;
	glm::vec4 m_Destination;
	uint8     is_ldon_door;
	uint32    client_version_mask;
};
#endif
