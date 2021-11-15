#include "../client.h"

void command_findspell(Client *c, const Seperator *sep)
{
	if (SPDAT_RECORDS <= 0) {
		c->Message(Chat::White, "Spells not loaded");
		return;
	}

	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #findspell [search criteria]");
		return;
	}

	if (sep->IsNumber(1)) {
		int spell_id = std::stoi(sep->arg[1]);
		if (!IsValidSpell(spell_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spell ID {} was not found.",
					spell_id
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Spell {}: {}",
					spell_id,
					spells[spell_id].name
				).c_str()
			);
		}
	}
	else {
		std::string search_criteria = str_tolower(sep->argplus[1]);
		int         found_count     = 0;
		for (int    spell_id        = 0; spell_id < SPDAT_RECORDS; spell_id++) {
			auto current_spell = spells[spell_id];
			if (current_spell.name[0] != 0) {
				std::string spell_name       = current_spell.name;
				std::string spell_name_lower = str_tolower(spell_name);
				if (search_criteria.length() > 0 && spell_name_lower.find(search_criteria) == std::string::npos) {
					continue;
				}

				c->Message(
					Chat::White,
					fmt::format(
						"Spell {}: {}",
						spell_id,
						spell_name
					).c_str()
				);
				found_count++;

				if (found_count == 20) {
					break;
				}
			}
		}

		if (found_count == 20) {
			c->Message(Chat::White, "20 Spells found... max reached.");
		}
		else {
			auto spell_message = (
				found_count > 0 ?
					(
						found_count == 1 ?
							"A Spell was" :
							fmt::format("{} Spells were", found_count)
					) :
					"No Spells were"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} found.",
					spell_message
				).c_str()
			);
		}
	}
}

inline bool CastRestrictedSpell(int spellid)
{
	switch (spellid) {
		case SPELL_TOUCH_OF_VINITRAS:
		case SPELL_DESPERATE_HOPE:
		case SPELL_CHARM:
		case SPELL_METAMORPHOSIS65:
		case SPELL_JT_BUFF:
		case SPELL_CAN_O_WHOOP_ASS:
		case SPELL_PHOENIX_CHARM:
		case SPELL_CAZIC_TOUCH:
		case SPELL_AVATAR_KNOCKBACK:
		case SPELL_SHAPECHANGE65:
		case SPELL_SUNSET_HOME1218:
		case SPELL_SUNSET_HOME819:
		case SPELL_SHAPECHANGE75:
		case SPELL_SHAPECHANGE80:
		case SPELL_SHAPECHANGE85:
		case SPELL_SHAPECHANGE90:
		case SPELL_SHAPECHANGE95:
		case SPELL_SHAPECHANGE100:
		case SPELL_SHAPECHANGE25:
		case SPELL_SHAPECHANGE30:
		case SPELL_SHAPECHANGE35:
		case SPELL_SHAPECHANGE40:
		case SPELL_SHAPECHANGE45:
		case SPELL_SHAPECHANGE50:
		case SPELL_NPC_AEGOLISM:
		case SPELL_SHAPECHANGE55:
		case SPELL_SHAPECHANGE60:
		case SPELL_COMMAND_OF_DRUZZIL:
		case SPELL_SHAPECHANGE70:
			return true;
		default:
			return false;
	}
}

