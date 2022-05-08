#include "../client.h"
#include "../groups.h"
#include "../mob_movement_manager.h"
#include "../raids.h"
#include "../raids.h"

void command_npcedit(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "Error: Must have NPC targeted");
		return;
	}

	if (strcasecmp(sep->arg[1], "help") == 0) {

		c->Message(Chat::White, "Help File for #npcedit. Syntax for commands are:");
		c->Message(Chat::White, "#npcedit name - Sets an NPC's Name");
		c->Message(Chat::White, "#npcedit lastname - Sets an NPC's Lastname");
		c->Message(Chat::White, "#npcedit level - Sets an NPC's Level");
		c->Message(Chat::White, "#npcedit race - Sets an NPC's  Race");
		c->Message(Chat::White, "#npcedit class - Sets an NPC's Class");
		c->Message(Chat::White, "#npcedit bodytype - Sets an NPC's Bodytype");
		c->Message(Chat::White, "#npcedit hp - Sets an NPC's Hitpoints");
		c->Message(Chat::White, "#npcedit mana - Sets an NPC's Mana");
		c->Message(Chat::White, "#npcedit gender - Sets an NPC's Gender");
		c->Message(Chat::White, "#npcedit texture - Sets an NPC's Texture");
		c->Message(Chat::White, "#npcedit helmtexture - Sets an NPC's Helmet Texture");
		c->Message(Chat::White, "#npcedit herosforgemodel - Sets an NPC's Hero's Forge Model");
		c->Message(Chat::White, "#npcedit size - Sets an NPC's Size");
		c->Message(Chat::White, "#npcedit hpregen - Sets an NPC's Hitpoints Regeneration Rate Per Tick");
		c->Message(Chat::White, "#npcedit hp_regen_per_second - Sets an NPC's HP regeneration per second");
		c->Message(Chat::White, "#npcedit manaregen - Sets an NPC's Mana Regeneration Rate Per Tick");
		c->Message(Chat::White, "#npcedit loottable - Sets an NPC's Loottable ID");
		c->Message(Chat::White, "#npcedit merchantid - Sets an NPC's Merchant ID");
		c->Message(Chat::White, "#npcedit alt_currency_id - Sets an NPC's Alternate Currency ID");
		c->Message(Chat::White, "#npcedit spell - Sets an NPC's Spells List ID");
		c->Message(Chat::White, "#npcedit npc_spells_effects_id - Sets an NPC's Spell Effects ID");
		c->Message(Chat::White, "#npcedit faction - Sets an NPC's Faction ID");
		c->Message(Chat::White, "#npcedit adventure_template_id - Sets an NPC's Adventure Template ID");
		c->Message(Chat::White, "#npcedit trap_template - Sets an NPC's Trap Template ID");
		c->Message(Chat::White, "#npcedit damage [minimum] [maximum] - Sets an NPC's Damage");
		c->Message(Chat::White, "#npcedit attackcount - Sets an NPC's Attack Count");
		c->Message(Chat::White, "#npcedit special_attacks - Sets an NPC's Special Attacks");
		c->Message(Chat::White, "#npcedit special_abilities - Sets an NPC's Special Abilities");
		c->Message(Chat::White, "#npcedit aggroradius - Sets an NPC's Aggro Radius");
		c->Message(Chat::White, "#npcedit assistradius - Sets an NPC's Assist Radius");
		c->Message(Chat::White, "#npcedit featuresave - Saves an NPC's current facial features to the database");
		c->Message(Chat::White, "#npcedit armortint_id - Sets an NPC's Armor Tint ID");
		c->Message(Chat::White, "#npcedit color [red] [green] [blue] - Sets an NPC's Red, Green, and Blue armor tint");
		c->Message(Chat::White, "#npcedit ammoidfile - Sets an NPC's Ammo ID File");
		c->Message(
			Chat::White,
			"#npcedit weapon [primary_model] [secondary_model] - Sets an NPC's Primary and Secondary Weapon Model"
		);
		c->Message(Chat::White, "#npcedit meleetype [primary_type] [secondary_type] - Sets an NPC's Melee Types");
		c->Message(Chat::White, "#npcedit rangedtype - Sets an NPC's Ranged Type");
		c->Message(Chat::White, "#npcedit runspeed - Sets an NPC's Run Speed");
		c->Message(Chat::White, "#npcedit mr - Sets an NPC's Magic Resistance");
		c->Message(Chat::White, "#npcedit pr - Sets an NPC's Poison Resistance");
		c->Message(Chat::White, "#npcedit dr - Sets an NPC's Disease Resistance");
		c->Message(Chat::White, "#npcedit fr - Sets an NPC's Fire Resistance");
		c->Message(Chat::White, "#npcedit cr - Sets an NPC's Cold Resistance");
		c->Message(Chat::White, "#npcedit corrup - Sets an NPC's Corruption Resistance");
		c->Message(Chat::White, "#npcedit phr - Sets and NPC's Physical Resistance");
		c->Message(
			Chat::White,
			"#npcedit seeinvis - Sets an NPC's See Invisible Flag [0 = Cannot See Invisible, 1 = Can See Invisible]"
		);
		c->Message(
			Chat::White,
			"#npcedit seeinvisundead - Sets an NPC's See Invisible vs. Undead Flag  [0 = Cannot See Invisible vs. Undead, 1 = Can See Invisible vs. Undead]"
		);
		c->Message(
			Chat::White,
			"#npcedit qglobal - Sets an NPC's Quest Global Flag [0 = Quest Globals Off, 1 = Quest Globals On]"
		);
		c->Message(Chat::White, "#npcedit ac - Sets an NPC's Armor Class");
		c->Message(
			Chat::White,
			"#npcedit npcaggro - Sets an NPC's NPC Aggro Flag [0 = Aggro NPCs Off, 1 = Aggro NPCs On]"
		);
		c->Message(Chat::White, "#npcedit spawn_limit - Sets an NPC's Spawn Limit Counter");
		c->Message(Chat::White, "#npcedit attackspeed - Sets an NPC's Attack Speed Modifier");
		c->Message(Chat::White, "#npcedit attackdelay - Sets an NPC's Attack Delay");
		c->Message(Chat::White, "#npcedit findable - Sets an NPC's Findable Flag [0 = Not Findable, 1 = Findable]");
		c->Message(Chat::White, "#npcedit str - Sets an NPC's Strength");
		c->Message(Chat::White, "#npcedit sta - Sets an NPC's Stamina");
		c->Message(Chat::White, "#npcedit dex - Sets an NPC's Dexterity");
		c->Message(Chat::White, "#npcedit agi - Sets an NPC's Agility");
		c->Message(Chat::White, "#npcedit int - Sets an NPC's Intelligence");
		c->Message(Chat::White, "#npcedit wis - Sets an NPC's Wisdom");
		c->Message(Chat::White, "#npcedit cha - Sets an NPC's Charisma");
		c->Message(
			Chat::White,
			"#npcedit seehide - Sets an NPC's See Hide Flag [0 = Cannot See Hide, 1 = Can See Hide]"
		);
		c->Message(
			Chat::White,
			"#npcedit seeimprovedhide - Sets an NPC's See Improved Hide Flag [0 = Cannot See Improved Hide, 1 = Can See Improved Hide]"
		);
		c->Message(Chat::White, "#npcedit trackable - Sets an NPC's Trackable Flag [0 = Not Trackable, 1 = Trackable]");
		c->Message(Chat::White, "#npcedit atk - Sets an NPC's Attack");
		c->Message(Chat::White, "#npcedit accuracy - Sets an NPC's Accuracy");
		c->Message(Chat::White, "#npcedit avoidance - Sets an NPC's Avoidance");
		c->Message(Chat::White, "#npcedit slow_mitigation - Sets an NPC's Slow Mitigation");
		c->Message(Chat::White, "#npcedit version - Sets an NPC's Version");
		c->Message(Chat::White, "#npcedit maxlevel - Sets an NPC's Maximum Level");
		c->Message(Chat::White, "#npcedit scalerate - Sets an NPC's Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
		c->Message(
			Chat::White,
			"#npcedit spellscale - Sets an NPC's Spell Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(
			Chat::White,
			"#npcedit healscale - Sets an NPC's Heal Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(
			Chat::White,
			"#npcedit no_target - Sets an NPC's No Target Hotkey Flag [0 = Not Targetable with Target Hotkey, 1 = Targetable with Target Hotkey]"
		);
		c->Message(
			Chat::White,
			"#npcedit raidtarget - Sets an NPC's Raid Target Flag [0 = Not a Raid Target, 1 = Raid Target]"
		);
		c->Message(Chat::White, "#npcedit armtexture - Sets an NPC's Arm Texture");
		c->Message(Chat::White, "#npcedit bracertexture - Sets an NPC's Bracer Texture");
		c->Message(Chat::White, "#npcedit handtexture - Sets an NPC's Hand Texture");
		c->Message(Chat::White, "#npcedit legtexture - Sets an NPC's Leg Texture");
		c->Message(Chat::White, "#npcedit feettexture - Sets an NPC's Feet Texture");
		c->Message(Chat::White, "#npcedit walkspeed - Sets an NPC's Walk Speed");
		c->Message(Chat::White, "#npcedit show_name - Sets an NPC's Show Name Flag [0 = Hidden, 1 = Shown]");
		c->Message(
			Chat::White,
			"#npcedit untargetable - Sets an NPC's Untargetable Flag [0 = Targetable, 1 = Untargetable]"
		);
		c->Message(Chat::White, "#npcedit charm_ac - Sets an NPC's Armor Class while Charmed");
		c->Message(Chat::White, "#npcedit charm_min_dmg - Sets an NPC's Minimum Damage while Charmed");
		c->Message(Chat::White, "#npcedit charm_max_dmg - Sets an NPC's Max Damage while Charmed");
		c->Message(Chat::White, "#npcedit charm_attack_delay - Sets an NPC's Attack Delay while Charmed");
		c->Message(Chat::White, "#npcedit charm_accuracy_rating - Sets an NPC's Accuracy Rating while Charmed");
		c->Message(Chat::White, "#npcedit charm_avoidance_rating - Sets an NPC's Avoidance Rating while Charmed");
		c->Message(Chat::White, "#npcedit charm_atk - Sets an NPC's Attack while Charmed");
		c->Message(
			Chat::White,
			"#npcedit skip_global_loot - Sets an NPC's Skip Global Loot Flag [0 = Don't Skip, 1 = Skip"
		);
		c->Message(
			Chat::White,
			"#npcedit rarespawn - Sets an NPC's Rare Spawn Flag [0 = Not a Rare Spawn, 1 = Rare Spawn]"
		);
		c->Message(
			Chat::White,
			"#npcedit stuck_behavior - Sets an NPC's Stuck Behavior [0 = Run to Target, 1 = Warp to Target, 2 = Take No Action, 3 = Evade Combat]"
		);
		c->Message(
			Chat::White,
			"#npcedit flymode - Sets an NPC's Fly Mode [0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running]"
		);
		c->Message(
			Chat::White,
			"#npcedit always_aggro - Sets an NPC's Always Aggro Flag [0 = Does not Always Aggro, 1 = Always Aggro]"
		);
		c->Message(
			Chat::White,
			"#npcedit exp_mod - Sets an NPC's Experience Modifier [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(Chat::White, "#npcedit setanimation - Sets an NPC's Animation on Spawn (Stored in spawn2 table)");
		c->Message(
			Chat::White,
			"#npcedit respawntime - Sets an NPC's Respawn Timer in Seconds (Stored in spawn2 table)"
		);
	}

	uint32 npc_id = c->GetTarget()->CastToNPC()->GetNPCTypeID();
	if (strcasecmp(sep->arg[1], "name") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has the name '{}'.", npc_id, sep->arg[2]).c_str());
		std::string query = fmt::format("UPDATE npc_types SET name = '{}' WHERE id = {}", sep->arg[2], npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "lastname") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has the lastname '{}'.", npc_id, sep->arg[2]).c_str());
		std::string query = fmt::format("UPDATE npc_types SET lastname = '{}' WHERE id = {}", sep->arg[2], npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "level") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} is now level {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET level = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "race") == 0) {
		auto race_id = atoi(sep->arg[2]);
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now a {} ({}).", npc_id, GetRaceIDName(race_id), race_id).c_str());
		std::string query = fmt::format("UPDATE npc_types SET race = {} WHERE id = {}", race_id, npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "class") == 0) {
		auto class_id = atoi(sep->arg[2]);
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now a {} ({}).", npc_id, GetClassIDName(class_id), class_id).c_str());
		std::string query = fmt::format("UPDATE npc_types SET class = {} WHERE id = {}", class_id, npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "bodytype") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is now using Bodytype {} ({}).",
				npc_id,
				EQ::constants::GetBodyTypeName(static_cast<bodyType>(std::stoul(sep->arg[2]))),
				std::stoul(sep->arg[2])
			).c_str()
		);
		std::string query = fmt::format(
			"UPDATE npc_types SET bodytype = {} WHERE id = {}",
			std::stoul(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hp") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now has {} Health.",
				npc_id,
				std::strtoll(sep->arg[2], nullptr, 10)
			).c_str()
		);
		std::string query = fmt::format("UPDATE npc_types SET hp = {} WHERE id = {}", strtoull(sep->arg[2], nullptr, 10), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "mana") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Mana.", npc_id, std::strtoll(sep->arg[2], nullptr, 10)).c_str());
		std::string query = fmt::format("UPDATE npc_types SET mana = {} WHERE id = {}", std::strtoll(sep->arg[2], nullptr, 10), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "gender") == 0) {
		auto gender_id = atoi(sep->arg[2]);
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now a {} ({}).", npc_id, gender_id, GetGenderName(gender_id)).c_str());
		std::string query = fmt::format("UPDATE npc_types SET gender = {} WHERE id = {}", gender_id, npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "texture") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} is now using Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET texture = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "helmtexture") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Helmet Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET helmtexture = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "herosforgemodel") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Hero's Forge Model {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET herosforgemodel = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "size") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} is now Size {:.2f}.", npc_id, atof(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET size = {:.2f} WHERE id = {}", atof(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hpregen") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now regenerates {} Health per Tick.", npc_id, std::strtoll(sep->arg[2], nullptr, 10)).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET hp_regen_rate = {} WHERE id = {}",
			std::strtoll(sep->arg[2], nullptr, 10),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "hp_regen_per_second") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now regenerates {} HP per second.",
				npc_id,
				std::strtoll(sep->arg[2], nullptr, 10)).c_str()
		);
		std::string query = fmt::format(
			"UPDATE npc_types SET hp_regen_per_second = {} WHERE id = {}",
			std::strtoll(sep->arg[2], nullptr, 10),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "manaregen") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now regenerates {} Mana per Tick.", npc_id, std::strtoll(sep->arg[2], nullptr, 10)).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET mana_regen_rate = {} WHERE id = {}",
			std::strtoll(sep->arg[2], nullptr, 10),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "loottable") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using loottable ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET loottable_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "merchantid") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using merchant ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET merchant_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "alt_currency_id") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Alternate Currency ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET alt_currency_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spell") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Spell List ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET npc_spells_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "npc_spells_effects_id") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using NPC Spells Effects ID {}.", npc_id, sep->arg[2]).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET npc_spells_effects_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "faction") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Faction ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET npc_faction_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "adventure_template_id") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Adventure Template ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET adventure_template_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "trap_template") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Trap Template ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET trap_template = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "damage") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now hits from {} to {} damage.",
				npc_id,
				atoi(sep->arg[2]),
				atoi(sep->arg[3])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET mindmg = {}, maxdmg = {} WHERE id = {}",
			atoi(sep->arg[2]),
			atoi(sep->arg[3]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "attackcount") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has an Attack Count of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET attack_count = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "special_attacks") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is now using the following Special Attacks '{}'.",
				npc_id,
				sep->arg[2]
			).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET npcspecialattks = '{}' WHERE id = {}",
			sep->arg[2],
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "special_abilities") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is now using the following Special Abilities '{}'.",
				npc_id,
				sep->arg[2]
			).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET special_abilities = '{}' WHERE id = {}",
			sep->arg[2],
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "aggroradius") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has an Aggro Radius of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET aggroradius = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "assistradius") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has an Assist Radius of {}", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET assistradius = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "featuresave") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} saved with all current body and facial feature settings.", npc_id).c_str());
		Mob         *target = c->GetTarget();
		std::string query   = fmt::format(
			"UPDATE npc_types "
			"SET luclin_haircolor = {}, luclin_beardcolor = {}, "
			"luclin_hairstyle = {}, luclin_beard = {}, "
			"face = {}, drakkin_heritage = {}, "
			"drakkin_tattoo = {}, drakkin_details = {}, "
			"texture = {}, helmtexture = {}, "
			"gender = {}, size = {:.2f}"
			"WHERE id = {}",
			target->GetHairColor(),
			target->GetBeardColor(),
			target->GetHairStyle(),
			target->GetBeard(),
			target->GetLuclinFace(),
			target->GetDrakkinHeritage(),
			target->GetDrakkinTattoo(),
			target->GetDrakkinDetails(),
			target->GetTexture(),
			target->GetHelmTexture(),
			target->GetGender(),
			target->GetSize(),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "armortint_id") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Armor Tint ID {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET armortint_id = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "color") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now has {} Red, {} Green, and {} Blue tinting on their armor.",
				npc_id,
				atoi(sep->arg[2]),
				atoi(sep->arg[3]),
				atoi(sep->arg[4])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET armortint_red = {}, armortint_green = {}, armortint_blue = {} WHERE id = {}",
			atoi(sep->arg[2]),
			atoi(sep->arg[3]),
			atoi(sep->arg[4]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "ammoidfile") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Ammo ID File {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET ammo_idfile = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "weapon") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} will have Model {} set to their Primary and Model {} set to their Secondary on repop.",
				npc_id,
				atoi(sep->arg[2]),
				atoi(sep->arg[3])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET d_melee_texture1 = {}, d_melee_texture2 = {} WHERE id = {}",
			atoi(sep->arg[2]),
			atoi(sep->arg[3]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "meleetype") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now has a Primary Melee Type of {} and a Secondary Melee Type of {}.",
				npc_id,
				atoi(sep->arg[2]),
				atoi(sep->arg[3])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET prim_melee_type = {}, sec_melee_type = {} WHERE id = {}",
			atoi(sep->arg[2]),
			atoi(sep->arg[3]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "rangedtype") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Ranged Type of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET ranged_type = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "runspeed") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now runs at {:.2f}.", npc_id, atof(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET runspeed = {:.2f} WHERE id = {}",
			atof(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "mr") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Magic Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET MR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "pr") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Poison Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET PR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "dr") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Disease Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET DR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "fr") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Fire Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET FR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "cr") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Cold Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET CR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "corrup") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Corruption Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET corrup = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "phr") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Physical Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET PhR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeinvis") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} can {} See Invisible.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET see_invis = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeinvisundead") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} can {} See Invisible vs. Undead.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET see_invis_undead = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "qglobal") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} can {} use Quest Globals.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET qglobal = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "ac") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Armor Class.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET ac = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "npcaggro") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} will {} aggro other NPCs that have a hostile faction.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET npc_aggro = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spawn_limit") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Spawn Limit of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET spawn_limit = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "attackspeed") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has an Attack Speed of {:.2f}.", npc_id, atof(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET attack_speed = {:.2f} WHERE id = {}",
			atof(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "attackdelay") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has an Attack Delay of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET attack_delay = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "findable") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is {} Findable.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET findable = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "str") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Strength.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET STR = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "sta") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Stamina.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET STA = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "agi") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Agility.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET AGI = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "dex") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Dexterity.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET DEX = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "int") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Intelligence.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET _INT = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "wis") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Magic Resistance of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET WIS = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "cha") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Charisma.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET CHA = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seehide") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} can {} See Hide.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET see_hide = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "seeimprovedhide") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} can {} See Improved Hide.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET see_improved_hide = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "trackable") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is {} Trackable.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET trackable = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "atk") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Attack.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET atk = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "accuracy") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Accuracy.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET accuracy = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "avoidance") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now has {} Avoidance.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET avoidance = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "slow_mitigation") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has {} Slow Mitigation.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET slow_mitigation = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "version") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} is now using Version {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET version = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "maxlevel") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Maximum Level of {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET maxlevel = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "scalerate") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Scaling Rate of {}%%.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET scalerate = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "spellscale") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Spell Scaling Rate of {}%%.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET spellscale = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "healscale") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has a Heal Scaling Rate of {}%%.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET healscale = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "no_target") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is {} Targetable with Target Hotkey.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET no_target_hotkey = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "raidtarget") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is {} designated as a Raid Target.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET raid_target = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "armtexture") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Arm Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET armtexture = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "bracertexture") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Bracer Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET bracertexture = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "handtexture") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Hand Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET handtexture = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "legtexture") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Leg Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET legtexture = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "feettexture") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} is now using Feet Texture {}.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET feettexture = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "walkspeed") == 0) {
		c->Message(Chat::Yellow, fmt::format("NPC ID {} now walks at {:.2f}.", npc_id, atof(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET walkspeed = {:.2f} WHERE id = {}",
			atof(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "show_name") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} will {} show their name.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format("UPDATE npc_types SET show_name = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "untargetable") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} will {} be untargetable.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET untargetable = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_ac") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has {} Armor Class while Charmed.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET charm_ac = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_min_dmg") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now does {} Minimum Damage while Charmed.",
				npc_id,
				atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET charm_min_dmg = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_max_dmg") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now does {} Maximum Damage while Charmed.",
				npc_id,
				atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET charm_max_dmg = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_attack_delay") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has {} Attack Delay while Charmed.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET charm_attack_delay = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_accuracy_rating") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now has {} Accuracy Rating while Charmed.",
				npc_id,
				atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET charm_accuracy_rating = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_avoidance_rating") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now has {} Avoidance Rating while Charmed.",
				npc_id,
				atoi(sep->arg[2])).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET charm_avoidance_rating = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "charm_atk") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has {} Attack while Charmed.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET charm_atk = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "skip_global_loot") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} will {} skip Global Loot.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET skip_global_loot = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "rarespawn") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is {} designated as a Rare Spawn.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET rare_spawn = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "stuck_behavior") == 0) {
		auto        behavior_id   = atoi(sep->arg[2]);
		std::string behavior_name = "Unknown";
		if (behavior_id == MobStuckBehavior::RunToTarget) {
			behavior_name = "Run To Target";
		}
		else if (behavior_id == MobStuckBehavior::WarpToTarget) {
			behavior_name = "Warp To Target";
		}
		else if (behavior_id == MobStuckBehavior::TakeNoAction) {
			behavior_name = "Take No Action";
		}
		else if (behavior_id == MobStuckBehavior::EvadeCombat) {
			behavior_name = "Evade Combat";
		}
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is now using Stuck Behavior {} ({}).",
				npc_id,
				behavior_name,
				behavior_id
			).c_str());
		std::string query = fmt::format("UPDATE npc_types SET stuck_behavior = {} WHERE id = {}", behavior_id, npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "flymode") == 0) {
		auto flymode_id = static_cast<uint8>(std::stoul(sep->arg[2]));
		std::string flymode_name = EQ::constants::GetFlyModeName(flymode_id);
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} is now using Fly Mode {} ({}).",
				npc_id,
				flymode_name,
				flymode_id
			).c_str()
		);
		std::string query = fmt::format("UPDATE npc_types SET flymode = {} WHERE id = {}", flymode_id, npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "always_aggro") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} will {} Always Aggro.",
				npc_id,
				(atoi(sep->arg[2]) == 1 ? "now" : "no longer")).c_str());
		std::string query = fmt::format(
			"UPDATE npc_types SET always_aggro = {} WHERE id = {}",
			atoi(sep->arg[2]),
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "exp_mod") == 0) {
		c->Message(
			Chat::Yellow,
			fmt::format("NPC ID {} now has an Experience Modifier of {}%%.", npc_id, atoi(sep->arg[2])).c_str());
		std::string query = fmt::format("UPDATE npc_types SET exp_mod = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
		content_db.QueryDatabase(query);
		return;
	}

	if (strcasecmp(sep->arg[1], "setanimation") == 0) {
		int         animation      = 0;
		std::string animation_name = "Unknown";
		if (sep->arg[2] && atoi(sep->arg[2]) <= 4) {
			if (strcasecmp(sep->arg[2], "stand") == 0 || atoi(sep->arg[2]) == 0) { // Stand
				animation      = 0;
				animation_name = "Standing";
			}
			else if (strcasecmp(sep->arg[2], "sit") == 0 || atoi(sep->arg[2]) == 1) { // Sit
				animation      = 1;
				animation_name = "Sitting";
			}
			else if (strcasecmp(sep->arg[2], "crouch") == 0 || atoi(sep->arg[2]) == 2) { // Crouch
				animation      = 2;
				animation_name = "Crouching";
			}
			else if (strcasecmp(sep->arg[2], "dead") == 0 || atoi(sep->arg[2]) == 3) { // Dead
				animation      = 3;
				animation_name = "Dead";
			}
			else if (strcasecmp(sep->arg[2], "loot") == 0 || atoi(sep->arg[2]) == 4) { // Looting Animation
				animation      = 4;
				animation_name = "Looting";
			}
		}
		else {
			c->Message(
				Chat::White,
				"You must specify an Animation (0 = Stand, 1 = Sit, 2 = Crouch, 3 = Dead, 4 = Loot)"
			);
			c->Message(Chat::White, "Example: #npcedit setanimation sit");
			c->Message(Chat::White, "Example: #npcedit setanimation 0");
			return;
		}

		c->Message(
			Chat::Yellow,
			fmt::format(
				"NPC ID {} now has their Spawn Animation set to {} ({}) on Spawn Group ID {}.",
				npc_id,
				animation_name,
				animation,
				c->GetTarget()->CastToNPC()->GetSpawnGroupId()
			).c_str()
		);
		std::string query = fmt::format(
			"UPDATE spawn2 SET animation = {} WHERE spawngroupID = {}",
			animation,
			c->GetTarget()->CastToNPC()->GetSpawnGroupId()
		);
		content_db.QueryDatabase(query);

		c->GetTarget()->SetAppearance(EmuAppearance(animation));
		return;
	}

	if (strcasecmp(sep->arg[1], "respawntime") == 0) {
		if (sep->arg[2][0] && sep->IsNumber(sep->arg[2]) && atoi(sep->arg[2]) > 0) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"NPC ID {} now has a Respawn Timer of {} Seconds on Spawn Group ID {}.",
					npc_id,
					atoi(sep->arg[2]),
					c->GetTarget()->CastToNPC()->GetSpawnGroupId()).c_str());
			std::string query = fmt::format(
				"UPDATE spawn2 SET respawntime = {} WHERE spawngroupID = {} AND version = {}",
				atoi(sep->arg[2]),
				c->GetTarget()->CastToNPC()->GetSpawnGroupId(),
				zone->GetInstanceVersion());
			content_db.QueryDatabase(query);
			return;
		}
	}

	if ((sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "*") == 0) ||
		((c->GetTarget() == 0) || (c->GetTarget()->IsClient()))) {
		c->Message(Chat::White, "Type #npcedit help for more info");
	}

}

