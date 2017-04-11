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
	Doors(const Door* door);
	Doors(const char *dmodel, const glm::vec4& position, uint8 dopentype = 58, uint16 dsize = 100);
	~Doors();
	bool	IsDoor() const { return true; }
	void	HandleClick(Client* sender, uint8 trigger);
	bool	Process();
	uint8	GetDoorID() { return door_id; }
	uint32	GetDoorDBID() { return db_id; }
	uint32	GetGuildID() { return guild_id; }
	uint8	GetOpenType() { return opentype; }
	char*	GetDoorName() { return door_name; }
	uint32	GetDoorParam() { return door_param; }
	int		GetInvertState() { return invert_state; }
	const glm::vec4& GetPosition() const{ return m_Position; }
	int		GetIncline() { return incline; }
	bool	triggered;
	void	SetOpenState(bool st) { is_open = st; }
	bool	IsDoorOpen() { return is_open; }

	uint8	GetTriggerDoorID() { return trigger_door; }
	uint8	GetTriggerType() { return trigger_type; }

	uint32	GetKeyItem() { return keyitem; }
	void	SetKeyItem(uint32 in) { keyitem = in; }
	uint8	GetNoKeyring() { return nokeyring; }
	void	SetNoKeyring(uint8 in) { nokeyring = in; }
	uint16	GetLockpick() { return lockpick; }
	void	SetLockpick(uint16 in) { lockpick = in; }
	uint16	GetSize() { return size; }
	void	SetGuildID(uint32 guild_id) { this->guild_id = guild_id; }

	uint32	GetEntityID() { return entity_id; }
	void	SetEntityID(uint32 entity) { entity_id = entity; }

	void	DumpDoor();
	const glm::vec4 GetDestination() const { return m_Destination; }

	uint8	IsLDoNDoor() { return is_ldon_door; }
	uint32	GetClientVersionMask() { return client_version_mask; }

	void	NPCOpen(NPC* sender, bool alt_mode=false);
	void	ForceOpen(Mob *sender, bool alt_mode=false);
	void	ForceClose(Mob *sender, bool alt_mode=false);
	void	ToggleState(Mob *sender);

	void	SetPosition(const glm::vec4& position);
	void	SetLocation(float x, float y, float z);
	void	SetIncline(int in);
	void	SetDoorName(const char* name);
	void	SetOpenType(uint8 in);
	void	SetSize(uint16 size);

	void	SetDisableTimer(bool flag);
	bool	GetDisableTimer() { return disable_timer; }

	void	CreateDatabaseEntry();

private:

	uint32	db_id;
	uint8	door_id;
	char	zone_name[32];
	char	door_name[32];
	glm::vec4 m_Position;
	int		incline;
	uint8	opentype;
	uint32	guild_id;
	uint16	lockpick;
	uint32	keyitem;
	uint8	nokeyring;
	uint8	trigger_door;
	uint8	trigger_type;
	uint32	door_param;
	uint16	size;
	int		invert_state;
	uint32	entity_id;
	bool	disable_timer;
	bool	is_open;
	Timer	close_timer;
	//Timer	trigger_timer;

	char	dest_zone[16];
	int		dest_instance_id;
	glm::vec4 m_Destination;

	uint8	is_ldon_door;
	uint32	client_version_mask;
};
#endif
