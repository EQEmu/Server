/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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
#ifndef _TRAP_H
#define _TRAP_H

#include "../common/debug.h"

#include "entity.h"

//ID of the NPC type to spawn when a trap is set off, to do the damage
#define TRAP_NPC_TYPE 1586

enum TrapTypes
{
	trapTypeDebuff = 0,
	trapTypeAlarm = 1,
	trapTypeMysticSpawn = 2,
	trapTypeBanditSpawn = 3,
	trapTypeDamage = 4,
};

class Trap: public Entity
{
public:
	Trap();
	virtual ~Trap();
	virtual bool Process();
	virtual bool IsTrap() const { return true; }
	void	Trigger(Mob* trigger);

	void	SpellOnTarget(Mob* trigger, uint32 spell_id);

	NPC * GetHiddenTrigger() { return hiddenTrigger; }
	void SetHiddenTrigger(NPC* n) { hiddenTrigger = n; }
	void CreateHiddenTrigger();

	//Trap data, leave this unprotected
	Timer	respawn_timer; //Respawn Time when Trap's been disarmed
	Timer	chkarea_timer;
	uint32	trap_id; //Database ID of trap
	float	x; //X position
	float	y; //Y position
	float	z; //Z position
	float	maxzdiff;	//maximum z diff to be triggerable
	float	radius;		//radius around trap to be triggerable
	uint8	chance;		//%chance that the trap is triggered each 'tick'
	uint8	effect; //Effect ID
	int32	effectvalue; //Value of Effect
	int32	effectvalue2; //Value of Effect
	uint8	skill; //Skill to detect/disarm with rogue.
	uint8	level;
	bool	detected;
	bool	disarmed;
	uint32	respawn_time;
	uint32	respawn_var;

	std::string message;
protected:
	NPC *hiddenTrigger;
	bool ownHiddenTrigger;
};

#endif

