#ifndef DOORS_H
#define DOORS_H
#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "entity.h"
#include "mob.h"
#include "zonedump.h"

class Doors : public Entity
{
public:
	Doors(const Door* door);
	Doors(const char *dmodel, float dx, float dy, float dz, float dheading, uint8 dopentype = 58, uint16 dsize = 100);
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
	float	GetX() { return pos_x; }
	float	GetY() { return pos_y; }
	float	GetZ() { return pos_z; }
	float	GetHeading() { return heading; }
	int		GetIncline() { return incline; }
	bool	triggered;
	void	SetOpenState(bool st) { isopen = st; }
	bool	IsDoorOpen() { return isopen; }

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
	float	GetDestX() { return dest_x; }
	float	GetDestY() { return dest_y; }
	float	GetDestZ() { return dest_z; }
	float	GetDestHeading() { return dest_heading; }

	uint8	IsLDoNDoor() { return is_ldon_door; }
	uint32	GetClientVersionMask() { return client_version_mask; }

	void	NPCOpen(NPC* sender, bool alt_mode=false);
	void	ForceOpen(Mob *sender, bool alt_mode=false);
	void	ForceClose(Mob *sender, bool alt_mode=false);
	void	ToggleState(Mob *sender);

	void	SetX(float in);
	void	SetY(float in);
	void	SetZ(float in);
	void	SetHeading(float in);
	void	SetIncline(int in);
	void	SetDoorName(char* name);
	void	SetOpenType(uint8 in);
	void	SetLocation(float x, float y, float z);
	void	SetSize(uint16 size);
	void	CreateDatabaseEntry();

private:

	uint32	db_id;
	uint8	door_id;
	char	zone_name[32];
	char	door_name[32];
	float	pos_x;
	float	pos_y;
	float	pos_z;
	float	heading;
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
	bool	isopen;
	Timer	close_timer;
	//Timer	trigger_timer;

	char	dest_zone[16];
	int		dest_instance_id;
	float	dest_x;
	float	dest_y;
	float	dest_z;
	float	dest_heading;

	uint8	is_ldon_door;
	uint32	client_version_mask;
};
#endif
