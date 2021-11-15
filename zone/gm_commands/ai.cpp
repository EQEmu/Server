#include "../client.h"

void command_ai(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();

	if (strcasecmp(sep->arg[1], "factionid") == 0) {
		if (target && sep->IsNumber(2)) {
			if (target->IsNPC()) {
				target->CastToNPC()->SetNPCFactionID(atoi(sep->arg[2]));
			}
			else {
				c->Message(Chat::White, "%s is not an NPC.", target->GetName());
			}
		}
		else {
			c->Message(Chat::White, "Usage: (targeted) #ai factionid [factionid]");
		}
	}
	else if (strcasecmp(sep->arg[1], "spellslist") == 0) {
		if (target && sep->IsNumber(2) && atoi(sep->arg[2]) >= 0) {
			if (target->IsNPC()) {
				target->CastToNPC()->AI_AddNPCSpells(atoi(sep->arg[2]));
			}
			else {
				c->Message(Chat::White, "%s is not an NPC.", target->GetName());
			}
		}
		else {
			c->Message(Chat::White, "Usage: (targeted) #ai spellslist [npc_spells_id]");
		}
	}
	else if (strcasecmp(sep->arg[1], "con") == 0) {
		if (target && sep->arg[2][0] != 0) {
			Mob *tar2 = entity_list.GetMob(sep->arg[2]);
			if (tar2) {
				c->Message(
					Chat::White,
					"%s considering %s: %i",
					target->GetName(),
					tar2->GetName(),
					tar2->GetReverseFactionCon(target));
			}
			else {
				c->Message(Chat::White, "Error: %s not found.", sep->arg[2]);
			}
		}
		else {
			c->Message(Chat::White, "Usage: (targeted) #ai con [mob name]");
		}
	}
	else if (strcasecmp(sep->arg[1], "guard") == 0) {
		if (target && target->IsNPC()) {
			target->CastToNPC()->SaveGuardSpot(target->GetPosition());
		}
		else {
			c->Message(
				Chat::White,
				"Usage: (targeted) #ai guard - sets npc to guard the current location (use #summon to move)"
			);
		}
	}
	else if (strcasecmp(sep->arg[1], "roambox") == 0) {
		if (target && target->IsAIControlled() && target->IsNPC()) {
			if ((sep->argnum == 6 || sep->argnum == 7 || sep->argnum == 8) && sep->IsNumber(2) && sep->IsNumber(3) &&
				sep->IsNumber(4) && sep->IsNumber(5) && sep->IsNumber(6)) {
				uint32 tmp  = 2500;
				uint32 tmp2 = 2500;
				if (sep->IsNumber(7)) {
					tmp = atoi(sep->arg[7]);
				}
				if (sep->IsNumber(8)) {
					tmp2 = atoi(sep->arg[8]);
				}
				target->CastToNPC()->AI_SetRoambox(
					atof(sep->arg[2]),
					atof(sep->arg[3]),
					atof(sep->arg[4]),
					atof(sep->arg[5]),
					atof(sep->arg[6]),
					tmp,
					tmp2
				);
			}
			else if ((sep->argnum == 3 || sep->argnum == 4) && sep->IsNumber(2) && sep->IsNumber(3)) {
				uint32 tmp  = 2500;
				uint32 tmp2 = 2500;
				if (sep->IsNumber(4)) {
					tmp = atoi(sep->arg[4]);
				}
				if (sep->IsNumber(5)) {
					tmp2 = atoi(sep->arg[5]);
				}
				target->CastToNPC()->AI_SetRoambox(atof(sep->arg[2]), atof(sep->arg[3]), tmp, tmp2);
			}
			else {
				c->Message(Chat::White, "Usage: #ai roambox dist max_x min_x max_y min_y [delay] [mindelay]");
				c->Message(Chat::White, "Usage: #ai roambox dist roamdist [delay] [mindelay]");
			}
		}
		else {
			c->Message(Chat::White, "You need a AI NPC targeted");
		}
	}
	else if (strcasecmp(sep->arg[1], "stop") == 0 && c->Admin() >= commandToggleAI) {
		if (target) {
			if (target->IsAIControlled()) {
				target->AI_Stop();
			}
			else {
				c->Message(Chat::White, "Error: Target is not AI controlled");
			}
		}
		else {
			c->Message(Chat::White, "Usage: Target a Mob with AI enabled and use this to turn off their AI.");
		}
	}
	else if (strcasecmp(sep->arg[1], "start") == 0 && c->Admin() >= commandToggleAI) {
		if (target) {
			if (!target->IsAIControlled()) {
				target->AI_Start();
			}
			else {
				c->Message(Chat::White, "Error: Target is already AI controlled");
			}
		}
		else {
			c->Message(Chat::White, "Usage: Target a Mob with AI disabled and use this to turn on their AI.");
		}
	}
	else {
		c->Message(Chat::White, "#AI Sub-commands");
		c->Message(Chat::White, "  factionid");
		c->Message(Chat::White, "  spellslist");
		c->Message(Chat::White, "  con");
		c->Message(Chat::White, "  guard");
	}
}

