/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

/*

Beacon class, extends Mob. Used for AE rain spells to have a mob
target to center around.

*/

class Zone;

#ifdef _WINDOWS
	#if (!defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER < 1900))
		#define snprintf	_snprintf
		#define vsnprintf	_vsnprintf
	#endif
    #define strncasecmp	_strnicmp
    #define strcasecmp	_stricmp
#endif

#include "../common/races.h"
#include "beacon.h"
#include "entity.h"
#include "mob.h"


#ifdef BOTS
#include "bot.h"
#endif


#include "../common/spdat.h"

extern EntityList entity_list;
extern Zone* zone;

// if lifetime is 0 this is a permanent beacon.. not sure if that'll be
// useful for anything
Beacon::Beacon(Mob *at_mob, int lifetime)
:Mob
(
	nullptr, nullptr, 0, 0, 0, INVISIBLE_MAN, 0, BT_NoTarget, 0, 0, 0, 0, 0, at_mob->GetPosition(), 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, EQEmu::TintProfile(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false
),
		remove_timer(lifetime),
		spell_timer(0)
{
	remove_timer.Disable();
	spell_timer.Disable();
	remove_me = false;
	spell_id = 0xFFFF;
	resist_adjust = 0;
	spell_iterations = 0;
	caster_id = 0;
	max_targets = 4; // default

	if(lifetime)
		remove_timer.Start();
}

Beacon::~Beacon()
{

}

bool Beacon::Process()
{
	if(remove_me)
	{
		return false;
	}

	if
	(
		spell_timer.Enabled() &&
		spell_timer.Check() &&
		IsValidSpell(spell_id)
	)
	{
		Mob *caster = entity_list.GetMob(caster_id);
		if(caster && spell_iterations-- && max_targets)
		{
			// NPCs should never be affected by an AE they cast. PB AEs shouldn't affect caster either
			// I don't think any other cases that get here matter
			bool affect_caster = (!caster->IsNPC() && !caster->IsAIControlled()) && spells[spell_id].targettype != ST_AECaster;
			entity_list.AESpell(caster, this, spell_id, affect_caster, resist_adjust, &max_targets);
		}
		else
		{
			// spell is done casting, or caster disappeared
			spell_id = 0xFFFF;
			spell_iterations = 0;
			spell_timer.Disable();
			caster_id = 0;
		}
	}

	if(remove_timer.Enabled() && remove_timer.Check())
	{
		return false;
	}

	return true;
}

void Beacon::AELocationSpell(Mob *caster, uint16 cast_spell_id, int16 resist_adjust)
{
	if(!IsValidSpell(cast_spell_id) || !caster)
		return;

	caster_id = caster->GetID();
	spell_id = cast_spell_id;
	this->resist_adjust = resist_adjust;
	spell_iterations = spells[spell_id].AEDuration / 2500;
	spell_iterations = spell_iterations < 1 ? 1 : spell_iterations;	// at least 1
	if (spells[spell_id].aemaxtargets)
		max_targets = spells[spell_id].aemaxtargets;
	spell_timer.Start(2500);
	spell_timer.Trigger();
}


