#include "../client.h"

void command_appearanceeffects(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help")) {
		c->Message(Chat::White, "Syntax: #appearanceeffects [subcommand].");
		c->Message(Chat::White, "[view] Display all appearance effects saved to your target. #appearanceffects view");
		c->Message(Chat::White, "[set] Set an appearance effects saved to your target. #appearanceffects set [app_effectid] [slotid]");
		c->Message(Chat::White, "[remove] Remove all appearance effects saved to your target. #appearanceffects remove");
	}

	if (!strcasecmp(sep->arg[1], "view")) {
		Mob* m_target = c->GetTarget();
		if (m_target) {
			m_target->GetAppearenceEffects();
		}
		return;
	}


	if (!strcasecmp(sep->arg[1], "set")) {
		int32 app_effectid = atof(sep->arg[2]);
		int32 slot = atoi(sep->arg[3]);

		Mob* m_target = c->GetTarget();
		if (m_target) {
			m_target->SendAppearanceEffect(app_effectid, 0, 0, 0, 0, nullptr, slot, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			c->Message(Chat::White, "Appearance Effect ID %i for slot %i has been set.", app_effectid, slot);
		}
	}

	if (!strcasecmp(sep->arg[1], "remove")) {
		Mob* m_target = c->GetTarget();
		if (m_target) {
			m_target->SendIllusionPacket(m_target->GetRace(), m_target->GetGender(), m_target->GetTexture(), m_target->GetHelmTexture(),
				m_target->GetHairColor(), m_target->GetBeardColor(), m_target->GetEyeColor1(), m_target->GetEyeColor2(),
				m_target->GetHairStyle(), m_target->GetLuclinFace(), m_target->GetBeard(), 0xFF,
				m_target->GetDrakkinHeritage(), m_target->GetDrakkinTattoo(), m_target->GetDrakkinDetails(), m_target->GetSize(), false);
			m_target->ClearAppearenceEffects();
			c->Message(Chat::White, "All Appearance Effects have been removed.");
		}
		return;
	}
}
