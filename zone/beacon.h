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

#ifndef BEACON_H
#define BEACON_H

#include "mob.h"
#include "../common/types.h"
#include "../common/timer.h"

class Group;
class Raid;
struct ExtraAttackOptions;

class Beacon : public Mob
{
public:
	Beacon(const glm::vec4 &in_pos, int lifetime);
	~Beacon();

	//abstract virtual function implementations requird by base abstract class
	virtual bool Death(Mob* killerMob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill) { return true; }
	virtual void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None) { return; }
	virtual bool Attack(Mob* other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false,
		ExtraAttackOptions *opts = nullptr) { return false; }
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return false; }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return 0; }

	bool	IsBeacon()			const { return true; }
	bool	Process();
	virtual void	Depop(bool not_used = true)	{ remove_me = true; }
	void AELocationSpell(Mob *caster, uint16 cast_spell_id, int16 resist_adjust);

protected:
	Timer remove_timer;
	bool remove_me;

	uint16 spell_id;
	int16 resist_adjust;
	int spell_iterations;
	Timer spell_timer;
	int max_targets;

	uint16 caster_id;
private:
};

#endif
