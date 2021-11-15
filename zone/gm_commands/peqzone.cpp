#include "../client.h"

void command_peqzone(Client *c, const Seperator *sep)
{
	uint32 timeleft = c->GetPTimers().GetRemainingTime(pTimerPeqzoneReuse) / 60;

	if (!c->GetPTimers().Expired(&database, pTimerPeqzoneReuse, false)) {
		c->Message(Chat::Red, "You must wait %i minute(s) before using this ability again.", timeleft);
		return;
	}

	if (c->GetHPRatio() < 75) {
		c->Message(Chat::White, "You cannot use this command with less than 75 percent health.");
		return;
	}

	//this isnt perfect, but its better...
	if (
		c->IsInvisible(c)
		|| c->IsRooted()
		|| c->IsStunned()
		|| c->IsMezzed()
		|| c->AutoAttackEnabled()
		|| c->GetInvul()
		) {
		c->Message(Chat::White, "You cannot use this command in your current state. Settle down and wait.");
		return;
	}

	uint16 zoneid   = 0;
	uint8  destzone = 0;
	if (sep->IsNumber(1)) {
		zoneid   = atoi(sep->arg[1]);
		destzone = content_db.GetPEQZone(zoneid, 0);
		if (destzone == 0) {
			c->Message(Chat::Red, "You cannot use this command to enter that zone!");
			return;
		}
		if (zoneid == zone->GetZoneID()) {
			c->Message(Chat::Red, "You cannot use this command on the zone you are in!");
			return;
		}
	}
	else if (sep->arg[1][0] == 0 || sep->IsNumber(2) || sep->IsNumber(3) || sep->IsNumber(4) || sep->IsNumber(5)) {
		c->Message(Chat::White, "Usage: #peqzone [zonename]");
		c->Message(Chat::White, "Optional Usage: #peqzone [zoneid]");
		return;
	}
	else {
		zoneid   = ZoneID(sep->arg[1]);
		destzone = content_db.GetPEQZone(zoneid, 0);
		if (zoneid == 0) {
			c->Message(Chat::White, "Unable to locate zone '%s'", sep->arg[1]);
			return;
		}
		if (destzone == 0) {
			c->Message(Chat::Red, "You cannot use this command to enter that zone!");
			return;
		}
		if (zoneid == zone->GetZoneID()) {
			c->Message(Chat::Red, "You cannot use this command on the zone you are in!");
			return;
		}
	}

	if (RuleB (Zone, UsePEQZoneDebuffs)) {
		c->SpellOnTarget(RuleI(Zone, PEQZoneDebuff1), c);
		c->SpellOnTarget(RuleI(Zone, PEQZoneDebuff2), c);
	}

	//zone to safe coords
	c->GetPTimers().Start(pTimerPeqzoneReuse, RuleI(Zone, PEQZoneReuseTime));
	c->MovePC(zoneid, 0.0f, 0.0f, 0.0f, 0.0f, 0, ZoneToSafeCoords);
}

