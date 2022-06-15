#include "../client.h"
#include "../groups.h"
#include "../raids.h"
#include "../raids.h"

void command_npcedit(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	int arguments = sep->argnum;

	bool is_help = (arguments == 0 || !strcasecmp(sep->arg[1], "help"));

	if (is_help) {
		c->Message(Chat::White, "Usage: #npcedit name [Name] - Sets an NPC's Name");
		c->Message(Chat::White, "Usage: #npcedit lastname [Last Name] - Sets an NPC's Last Name");
		c->Message(Chat::White, "Usage: #npcedit level [Level] - Sets an NPC's Level");
		c->Message(Chat::White, "Usage: #npcedit race [Race ID] - Sets an NPC's Race");
		c->Message(Chat::White, "Usage: #npcedit class [Class ID] - Sets an NPC's Class");
		c->Message(Chat::White, "Usage: #npcedit bodytype [Body Type ID] - Sets an NPC's Bodytype");
		c->Message(Chat::White, "Usage: #npcedit hp [HP] - Sets an NPC's HP");
		c->Message(Chat::White, "Usage: #npcedit mana [Mana] - Sets an NPC's Mana");
		c->Message(Chat::White, "Usage: #npcedit gender [Gender ID] - Sets an NPC's Gender");
		c->Message(Chat::White, "Usage: #npcedit texture [Texture] - Sets an NPC's Texture");
		c->Message(Chat::White, "Usage: #npcedit helmtexture [Helmet Texture] - Sets an NPC's Helmet Texture");
		c->Message(Chat::White, "Usage: #npcedit herosforgemodel [Model Number] - Sets an NPC's Hero's Forge Model");
		c->Message(Chat::White, "Usage: #npcedit size [Size] - Sets an NPC's Size");
		c->Message(Chat::White, "Usage: #npcedit hpregen [HP Regen] - Sets an NPC's HP Regen Rate Per Tick");
		c->Message(Chat::White, "Usage: #npcedit hp_regen_per_second [HP Regen] - Sets an NPC's HP Regen Rate Per Second");
		c->Message(Chat::White, "Usage: #npcedit manaregen [Mana Regen] - Sets an NPC's Mana Regen Rate Per Tick");
		c->Message(Chat::White, "Usage: #npcedit loottable [Loottable ID] - Sets an NPC's Loottable ID");
		c->Message(Chat::White, "Usage: #npcedit merchantid [Merchant ID] - Sets an NPC's Merchant ID");
		c->Message(Chat::White, "Usage: #npcedit alt_currency_id [Alternate Currency ID] - Sets an NPC's Alternate Currency ID");
		c->Message(Chat::White, "Usage: #npcedit spell [Spell List ID] - Sets an NPC's Spells List ID");
		c->Message(Chat::White, "Usage: #npcedit npc_spells_effects_id [Spell Effects ID] - Sets an NPC's Spell Effects ID");
		c->Message(Chat::White, "Usage: #npcedit faction [Faction ID] - Sets an NPC's Faction ID");
		c->Message(Chat::White, "Usage: #npcedit adventure_template_id [Template ID] - Sets an NPC's Adventure Template ID");
		c->Message(Chat::White, "Usage: #npcedit trap_template [Template ID] - Sets an NPC's Trap Template ID");
		c->Message(Chat::White, "Usage: #npcedit damage [Minimum] [Maximum] - Sets an NPC's Damage");
		c->Message(Chat::White, "Usage: #npcedit attackcount [Attack Count] - Sets an NPC's Attack Count");
		c->Message(Chat::White, "Usage: #npcedit special_attacks [Special Attacks] - Sets an NPC's Special Attacks");
		c->Message(Chat::White, "Usage: #npcedit special_abilities [Special Abilities] - Sets an NPC's Special Abilities");
		c->Message(Chat::White, "Usage: #npcedit aggroradius [Radius] - Sets an NPC's Aggro Radius");
		c->Message(Chat::White, "Usage: #npcedit assistradius [Radius] - Sets an NPC's Assist Radius");
		c->Message(Chat::White, "Usage: #npcedit featuresave - Saves an NPC's current facial features to the database");
		c->Message(Chat::White, "Usage: #npcedit armortint_id [Armor Tint ID] - Sets an NPC's Armor Tint ID");
		c->Message(Chat::White, "Usage: #npcedit color [Red] [Green] [Blue] - Sets an NPC's Red, Green, and Blue armor tint");
		c->Message(Chat::White, "Usage: #npcedit ammoidfile [ID File] - Sets an NPC's Ammo ID File");
		c->Message(
			Chat::White,
			"Usage: #npcedit weapon [Primary Model] [Secondary Model] - Sets an NPC's Primary and Secondary Weapon Model"
		);
		c->Message(Chat::White, "Usage: #npcedit meleetype [Primary Type] [Secondary Type] - Sets an NPC's Melee Skill Types");
		c->Message(Chat::White, "Usage: #npcedit rangedtype [Type] - Sets an NPC's Ranged Skill Type");
		c->Message(Chat::White, "Usage: #npcedit runspeed [Run Speed] - Sets an NPC's Run Speed");
		c->Message(Chat::White, "Usage: #npcedit mr [Resistance] - Sets an NPC's Magic Resistance");
		c->Message(Chat::White, "Usage: #npcedit pr [Resistance] - Sets an NPC's Poison Resistance");
		c->Message(Chat::White, "Usage: #npcedit dr [Resistance] - Sets an NPC's Disease Resistance");
		c->Message(Chat::White, "Usage: #npcedit fr [Resistance] - Sets an NPC's Fire Resistance");
		c->Message(Chat::White, "Usage: #npcedit cr [Resistance] - Sets an NPC's Cold Resistance");
		c->Message(Chat::White, "Usage: #npcedit corrup [Resistance] - Sets an NPC's Corruption Resistance");
		c->Message(Chat::White, "Usage: #npcedit phr [Resistance] - Sets and NPC's Physical Resistance");
		c->Message(
			Chat::White,
			"Usage: #npcedit seeinvis [Flag] - Sets an NPC's See Invisible Flag [0 = Cannot See Invisible, 1 = Can See Invisible]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit seeinvisundead [Flag] - Sets an NPC's See Invisible vs. Undead Flag  [0 = Cannot See Invisible vs. Undead, 1 = Can See Invisible vs. Undead]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit qglobal [Flag] - Sets an NPC's Quest Global Flag [0 = Quest Globals Off, 1 = Quest Globals On]"
		);
		c->Message(Chat::White, "Usage: #npcedit ac [Armor Class] - Sets an NPC's Armor Class");
		c->Message(
			Chat::White,
			"Usage: #npcedit npcaggro [Flag] - Sets an NPC's NPC Aggro Flag [0 = Aggro NPCs Off, 1 = Aggro NPCs On]"
		);
		c->Message(Chat::White, "Usage: #npcedit spawn_limit [Limit] - Sets an NPC's Spawn Limit Counter");
		c->Message(Chat::White, "Usage: #npcedit attackspeed [Attack Speed] - Sets an NPC's Attack Speed Modifier");
		c->Message(Chat::White, "Usage: #npcedit attackdelay [Attack Delay] - Sets an NPC's Attack Delay");
		c->Message(Chat::White, "Usage: #npcedit findable [Flag] - Sets an NPC's Findable Flag [0 = Not Findable, 1 = Findable]");
		c->Message(Chat::White, "Usage: #npcedit str [Strength] - Sets an NPC's Strength");
		c->Message(Chat::White, "Usage: #npcedit sta [Stamina] - Sets an NPC's Stamina");
		c->Message(Chat::White, "Usage: #npcedit agi [Agility] - Sets an NPC's Agility");
		c->Message(Chat::White, "Usage: #npcedit dex [Dexterity] - Sets an NPC's Dexterity");
		c->Message(Chat::White, "Usage: #npcedit int [Intelligence] - Sets an NPC's Intelligence");
		c->Message(Chat::White, "Usage: #npcedit wis [Wisdom] - Sets an NPC's Wisdom");
		c->Message(Chat::White, "Usage: #npcedit cha [Charisma] - Sets an NPC's Charisma");
		c->Message(
			Chat::White,
			"Usage: #npcedit seehide [Flag] - Sets an NPC's See Hide Flag [0 = Cannot See Hide, 1 = Can See Hide]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit seeimprovedhide [Flag] - Sets an NPC's See Improved Hide Flag [0 = Cannot See Improved Hide, 1 = Can See Improved Hide]"
		);
		c->Message(Chat::White, "Usage: #npcedit trackable [Flag] - Sets an NPC's Trackable Flag [0 = Not Trackable, 1 = Trackable]");
		c->Message(Chat::White, "Usage: #npcedit atk [Attack] - Sets an NPC's Attack");
		c->Message(Chat::White, "Usage: #npcedit accuracy [Accuracy] - Sets an NPC's Accuracy");
		c->Message(Chat::White, "Usage: #npcedit avoidance [Avoidance] - Sets an NPC's Avoidance");
		c->Message(Chat::White, "Usage: #npcedit slow_mitigation [Slow Mitigation] - Sets an NPC's Slow Mitigation");
		c->Message(Chat::White, "Usage: #npcedit version [Version] - Sets an NPC's Version");
		c->Message(Chat::White, "Usage: #npcedit maxlevel [Max Level] - Sets an NPC's Maximum Level");
		c->Message(Chat::White, "Usage: #npcedit scalerate [Scale Rate] - Sets an NPC's Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
		c->Message(
			Chat::White,
			"Usage: #npcedit spellscale [Scale Rate] - Sets an NPC's Spell Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit healscale [Scale Rate] - Sets an NPC's Heal Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit no_target [Flag] - Sets an NPC's No Target Hotkey Flag [0 = Not Targetable with Target Hotkey, 1 = Targetable with Target Hotkey]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit raidtarget [Flag] - Sets an NPC's Raid Target Flag [0 = Not a Raid Target, 1 = Raid Target]"
		);
		c->Message(Chat::White, "Usage: #npcedit armtexture [Texture] - Sets an NPC's Arm Texture");
		c->Message(Chat::White, "Usage: #npcedit bracertexture [Texture] - Sets an NPC's Bracer Texture");
		c->Message(Chat::White, "Usage: #npcedit handtexture [Texture] - Sets an NPC's Hand Texture");
		c->Message(Chat::White, "Usage: #npcedit legtexture [Texture] - Sets an NPC's Leg Texture");
		c->Message(Chat::White, "Usage: #npcedit feettexture [Texture] - Sets an NPC's Feet Texture");
		c->Message(Chat::White, "Usage: #npcedit walkspeed [Walk Speed] - Sets an NPC's Walk Speed");
		c->Message(Chat::White, "Usage: #npcedit show_name [Flag] - Sets an NPC's Show Name Flag [0 = Hidden, 1 = Shown]");
		c->Message(
			Chat::White,
			"Usage: #npcedit untargetable [Flag] - Sets an NPC's Untargetable Flag [0 = Targetable, 1 = Untargetable]"
		);
		c->Message(Chat::White, "Usage: #npcedit charm_ac [Armor Class] - Sets an NPC's Armor Class while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_min_dmg [Damage] - Sets an NPC's Minimum Damage while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_max_dmg [Damage] - Sets an NPC's Maximum Damage while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_attack_delay [Attack Delay] - Sets an NPC's Attack Delay while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_accuracy_rating [Accuracy] - Sets an NPC's Accuracy Rating while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_avoidance_rating [Avoidance] - Sets an NPC's Avoidance Rating while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_atk [Attack] - Sets an NPC's Attack while Charmed");
		c->Message(
			Chat::White,
			"Usage: #npcedit skip_global_loot [Flag] - Sets an NPC's Skip Global Loot Flag [0 = Don't Skip, 1 = Skip"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit rarespawn [Flag] - Sets an NPC's Rare Spawn Flag [0 = Not a Rare Spawn, 1 = Rare Spawn]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit stuck_behavior [Stuck Behavior] - Sets an NPC's Stuck Behavior [0 = Run to Target, 1 = Warp to Target, 2 = Take No Action, 3 = Evade Combat]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit flymode [Fly Mode] - Sets an NPC's Fly Mode [0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit always_aggro [Flag] - Sets an NPC's Always Aggro Flag [0 = Does not Always Aggro, 1 = Always Aggro]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit exp_mod [Modifier] - Sets an NPC's Experience Modifier [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(Chat::White, "Usage: #npcedit setanimation [Animation ID] - Sets an NPC's Animation on Spawn (Stored in spawn2 table)");
		c->Message(
			Chat::White,
			"Usage: #npcedit respawntime [Respawn Time] - Sets an NPC's Respawn Timer in Seconds (Stored in spawn2 table)"
		);
		return;
	}

	std::string sub_command = sep->arg[1];

	uint32 npc_id = c->GetTarget()->CastToNPC()->GetNPCTypeID();
	auto npc_id_string = fmt::format(
		"NPC ID {}",
		commify(std::to_string(npc_id))
	);

	if (!strcasecmp(sep->arg[1], "name")) {
		std::string name = sep->argplus[2];
		if (!name.empty()) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now named '{}'.",
					npc_id_string,
					sep->argplus[2]
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET name = '{}' WHERE id = {}",
				sep->argplus[2],
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit name [Name] - Sets an NPC's Name");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "lastname")) {
		std::string last_name = sep->argplus[2];
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} now has the lastname '{}'.",
				npc_id_string,
				last_name
			).c_str()
		);
		auto query = fmt::format(
			"UPDATE npc_types SET lastname = '{}' WHERE id = {}",
			last_name,
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	} else if (!strcasecmp(sep->arg[1], "level")) {
		if (sep->IsNumber(2)) {
			auto level = static_cast<uint8>(std::stoul(sep->arg[2]));
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now level {}.",
					npc_id_string,
					level
				).c_str()
			);
			auto query = fmt::format("UPDATE npc_types SET level = {} WHERE id = {}", atoi(sep->arg[2]), npc_id);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit level [Level] - Sets an NPC's Level");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "race")) {
		if (sep->IsNumber(2)) {
			auto race_id = static_cast<uint16>(std::stoul(sep->arg[2]));
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now a {} ({}).",
					npc_id_string,
					GetRaceIDName(race_id),
					race_id
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET race = {} WHERE id = {}",
				race_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit race [Race ID] - Sets an NPC's Race");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "class")) {
		if (sep->IsNumber(2)) {
			auto class_id = static_cast<uint8>(std::stoul(sep->arg[2]));
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now a {} ({}).",
					npc_id_string,
					GetClassIDName(class_id),
					class_id
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET class = {} WHERE id = {}",
				class_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit class [Class ID] - Sets an NPC's Class");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "bodytype")) {
		if (sep->IsNumber(2)) {
			auto body_type_id = static_cast<uint8>(std::stoul(sep->arg[2]));
			auto body_type_name = EQ::constants::GetBodyTypeName(static_cast<bodyType>(body_type_id));
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Body Type {}.",
					npc_id_string,
					(
						!body_type_name.empty() ?
						fmt::format(
							"{} ({})",
							body_type_name,
							body_type_id
						) :
						std::to_string(body_type_id)
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET bodytype = {} WHERE id = {}",
				body_type_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit bodytype [Body Type ID] - Sets an NPC's Bodytype");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "hp")) {
		if (sep->IsNumber(2)) {
			auto hp = std::stoll(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Health.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET hp = {} WHERE id = {}",
				hp,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit hp [HP] - Sets an NPC's HP");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "mana")) {
		if (sep->IsNumber(2)) {
			auto mana = std::stoll(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Mana.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET mana = {} WHERE id = {}",
				mana,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit mana [Mana] - Sets an NPC's Mana");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "gender")) {
		if (sep->IsNumber(2)) {
			auto gender_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now a {} ({}).",
					npc_id_string,
					gender_id,
					GetGenderName(gender_id)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET gender = {} WHERE id = {}",
				gender_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit gender [Gender ID] - Sets an NPC's Gender");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "texture")) {
		if (sep->IsNumber(2)) {
			auto texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Texture {}.",
					npc_id_string,
					texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET texture = {} WHERE id = {}",
				texture,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit texture [Texture] - Sets an NPC's Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "helmtexture")) {
		if (sep->IsNumber(2)) {
			auto helmet_texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Helmet Texture {}.",
					npc_id_string,
					helmet_texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET helmtexture = {} WHERE id = {}",
				atoi(sep->arg[2]),
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit helmtexture [Helmet Texture] - Sets an NPC's Helmet Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "herosforgemodel")) {
		if (sep->IsNumber(2)) {
			auto heros_forge_model = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Hero's Forge Model {}.",
					npc_id_string,
					heros_forge_model
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET herosforgemodel = {} WHERE id = {}",
				heros_forge_model,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit herosforgemodel [Model Number] - Sets an NPC's Hero's Forge Model");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "size")) {
		if (sep->IsNumber(2)) {
			auto size = std::stof(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now Size {:.2f}.",
					npc_id_string,
					size
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET size = {:.2f} WHERE id = {}",
				size,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit size [Size] - Sets an NPC's Size");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "hpregen")) {
		if (sep->IsNumber(2)) {
			auto hp_regen = std::stoll(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now regenerates {} Health per Tick.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET hp_regen_rate = {} WHERE id = {}",
				hp_regen,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit hpregen [HP Regen] - Sets an NPC's HP Regen Rate Per Tick");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "hp_regen_per_second")) {
		if (sep->IsNumber(2)) {
			auto hp_regen_per_second = std::stoll(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now regenerates {} HP per Second.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET hp_regen_per_second = {} WHERE id = {}",
				hp_regen_per_second,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit hp_regen_per_second [HP Regen] - Sets an NPC's HP Regen Rate Per Second");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "manaregen")) {
		if (sep->IsNumber(2)) {
			auto mana_regen = std::stoll(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now regenerates {} Mana per Tick.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET mana_regen_rate = {} WHERE id = {}",
				mana_regen,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit manaregen [Mana Regen] - Sets an NPC's Mana Regen Rate Per Tick");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "loottable")) {
		if (sep->IsNumber(2)) {
			auto loottable_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Loottable ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET loottable_id = {} WHERE id = {}",
				loottable_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit loottable [Loottable ID] - Sets an NPC's Loottable ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "merchantid")) {
		if (sep->IsNumber(2)) {
			auto merchant_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Merchant ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET merchant_id = {} WHERE id = {}",
				merchant_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit merchantid [Merchant ID] - Sets an NPC's Merchant ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "alt_currency_id")) {
		if (sep->IsNumber(2)) {
			auto alternate_currency_id = std::stoul(sep->arg[2]);
			auto alternate_currency_item_id  = zone->GetCurrencyItemID(alternate_currency_id);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Alternate Currency {}.",
					npc_id_string,
					(
						alternate_currency_item_id ?
						fmt::format(
							"{} ({})",
							database.CreateItemLink(alternate_currency_item_id),
							alternate_currency_id
						) :
						std::to_string(alternate_currency_id)
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET alt_currency_id = {} WHERE id = {}",
				alternate_currency_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit alt_currency_id [Alternate Currency ID] - Sets an NPC's Alternate Currency ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "spell")) {
		if (sep->IsNumber(2)) {
			auto spell_list_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Spell List ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET npc_spells_id = {} WHERE id = {}",
				spell_list_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit spell [Spell List ID] - Sets an NPC's Spells List ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "npc_spells_effects_id")) {
		if (sep->IsNumber(2)) {
			auto spell_effects_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Spells Effects ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET npc_spells_effects_id = {} WHERE id = {}",
				spell_effects_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit npc_spells_effects_id [Spell Effects ID] - Sets an NPC's Spell Effects ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "faction")) {
		if (sep->IsNumber(2)) {
			auto faction_id = std::stoi(sep->arg[2]);
			auto faction_name = content_db.GetFactionName(faction_id);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Faction {}.",
					npc_id_string,
					(
						!faction_name.empty() ?
						fmt::format(
							"{} ({})",
							faction_name,
							faction_id
						) :
						commify(sep->arg[2])
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET npc_faction_id = {} WHERE id = {}",
				faction_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit faction [Faction ID] - Sets an NPC's Faction ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "adventure_template_id")) {
		if (sep->IsNumber(2)) {
			auto adventure_template_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Adventure Template ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET adventure_template_id = {} WHERE id = {}",
				adventure_template_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit adventure_template_id [Template ID] - Sets an NPC's Adventure Template ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "trap_template")) {
		if (sep->IsNumber(2)) {
			auto trap_template = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Trap Template ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET trap_template = {} WHERE id = {}",
				trap_template,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit trap_template [Template ID] - Sets an NPC's Trap Template ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "damage")) {
		if (sep->IsNumber(2) && sep->IsNumber(3)) {
			auto minimum_damage = std::stoul(sep->arg[2]);
			auto maximum_damage = std::stoul(sep->arg[3]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now hits from {} to {} damage.",
					npc_id_string,
					commify(sep->arg[2]),
					commify(sep->arg[3])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET mindmg = {}, maxdmg = {} WHERE id = {}",
				minimum_damage,
				maximum_damage,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit damage [Minimum] [Maximum] - Sets an NPC's Damage");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "attackcount")) {
		if (sep->IsNumber(2)) {
			auto attack_count = std::stoi(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has an Attack Count of {}.",
					npc_id_string,
					attack_count
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET attack_count = {} WHERE id = {}",
				attack_count,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit attackcount [Attack Count] - Sets an NPC's Attack Count");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "special_attacks")) {
		std::string special_attacks = sep->argplus[2];
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} is now using the following Special Attacks '{}'.",
				npc_id_string,
				special_attacks
			).c_str()
		);
		auto query = fmt::format(
			"UPDATE npc_types SET npcspecialattks = '{}' WHERE id = {}",
			special_attacks,
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	} else if (!strcasecmp(sep->arg[1], "special_abilities")) {
		std::string special_abilities = sep->argplus[2];
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} is now using the following Special Abilities '{}'.",
				npc_id_string,
				special_abilities
			).c_str()
		);
		auto query = fmt::format(
			"UPDATE npc_types SET special_abilities = '{}' WHERE id = {}",
			special_abilities,
			npc_id
		);
		content_db.QueryDatabase(query);
		return;
	} else if (!strcasecmp(sep->arg[1], "aggroradius")) {
		if (sep->IsNumber(2)) {
			auto aggro_radius = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has an Aggro Radius of {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET aggroradius = {} WHERE id = {}",
				aggro_radius,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit aggroradius [Radius] - Sets an NPC's Aggro Radius");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "assistradius")) {
		if (sep->IsNumber(2)) {
			auto assist_radius = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has an Assist Radius of {}",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET assistradius = {} WHERE id = {}",
				assist_radius,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit assistradius [Radius] - Sets an NPC's Assist Radius");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "featuresave")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} saved with all current body and facial feature settings.",
				npc_id
			).c_str()
		);
		auto target = c->GetTarget();
		auto query = fmt::format(
			"UPDATE npc_types "
			"SET luclin_haircolor = {}, luclin_beardcolor = {}, "
			"luclin_eyecolor = {}, luclin_eyecolor2 = {}, "
			"luclin_hairstyle = {}, luclin_beard = {}, "
			"face = {}, drakkin_heritage = {}, "
			"drakkin_tattoo = {}, drakkin_details = {}, "
			"texture = {}, helmtexture = {}, "
			"gender = {}, size = {:.2f} "
			"WHERE id = {}",
			target->GetHairColor(),
			target->GetBeardColor(),
			target->GetEyeColor1(),
			target->GetEyeColor2(),
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
	} else if (!strcasecmp(sep->arg[1], "armortint_id")) {
		if (sep->IsNumber(2)) {
			auto armor_tint_id = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Armor Tint ID {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET armortint_id = {} WHERE id = {}",
				armor_tint_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit armortint_id [Armor Tint ID] - Sets an NPC's Armor Tint ID");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "color")) {
		if (sep->IsNumber(2)) {
			auto red = static_cast<uint8>(std::stoul(sep->arg[2]));
			uint8 green = sep->IsNumber(3) ? std::stoul(sep->arg[3]) : 0;
			uint8 blue = sep->IsNumber(4) ? std::stoul(sep->arg[4]) : 0;
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Red, {} Green, and {} Blue tinting on their armor.",
					npc_id_string,
					red,
					green,
					blue
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET armortint_red = {}, armortint_green = {}, armortint_blue = {} WHERE id = {}",
				red,
				green,
				blue,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit color [Red] [Green] [Blue] - Sets an NPC's Red, Green, and Blue armor tint");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "ammoidfile")) {
		if (sep->IsNumber(2)) {
			auto ammo_id_file = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Ammo ID File {}.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET ammo_idfile = {} WHERE id = {}",
				ammo_id_file,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit ammoidfile [ID File] - Sets an NPC's Ammo ID File");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "weapon")) {
		if (sep->IsNumber(2)) {
			auto primary_model = std::stoul(sep->arg[2]);
			uint32 secondary_model = sep->IsNumber(3) ? std::stoul(sep->arg[3]) : 0;
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} will have Model {} set to their Primary and Model {} set to their Secondary on repop.",
					npc_id_string,
					commify(sep->arg[2]),
					sep->IsNumber(3) ? commify(sep->arg[3]) : 0
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET d_melee_texture1 = {}, d_melee_texture2 = {} WHERE id = {}",
				primary_model,
				secondary_model,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit weapon [Primary Model] [Secondary Model] - Sets an NPC's Primary and Secondary Weapon Model"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "meleetype")) {
		if (sep->IsNumber(2)) {
			auto primary_type = std::stoul(sep->arg[2]);
			uint32 secondary_type = sep->IsNumber(3) ? std::stoul(sep->arg[3]) : 0;

			auto primary_skill = EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(primary_type));
			auto secondary_skill = EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(secondary_type));

			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Primary Melee Type of {} and a Secondary Melee Type of {}.",
					npc_id_string,
					(
						!primary_skill.empty() ?
						fmt::format(
							"{} ({})",
							primary_skill,
							primary_type
						) :
						std::to_string(primary_type)
					),
					(
						!secondary_skill.empty() ?
						fmt::format(
							"{} ({})",
							secondary_skill,
							secondary_type
						) :
						std::to_string(secondary_type)
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET prim_melee_type = {}, sec_melee_type = {} WHERE id = {}",
				primary_type,
				secondary_type,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit meleetype [Primary Type] [Secondary Type] - Sets an NPC's Melee Skill Types");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "rangedtype")) {
		if (sep->IsNumber(2)) {
			auto ranged_type = std::stoul(sep->arg[2]);

			auto ranged_skill = EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(ranged_type));

			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Ranged Type of {}.",
					npc_id_string,
					(
						!ranged_skill.empty() ?
						fmt::format(
							"{} ({})",
							ranged_skill,
							ranged_type
						) :
						std::to_string(ranged_type)
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET ranged_type = {} WHERE id = {}",
				ranged_type,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit rangedtype [Type] - Sets an NPC's Ranged Skill Type");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "runspeed")) {
		if (sep->IsNumber(2)) {
			auto run_speed = std::stof(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now runs at a Run Speed of {:.2f}.",
					npc_id_string,
					run_speed
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET runspeed = {:.2f} WHERE id = {}",
				run_speed,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit runspeed [Run Speed] - Sets an NPC's Run Speed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "mr")) {
		if (sep->IsNumber(2)) {
			auto magic_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Magic Resistance of {}.",
					npc_id_string,
					magic_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET MR = {} WHERE id = {}",
				magic_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit mr [Resistance] - Sets an NPC's Magic Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "pr")) {
		if (sep->IsNumber(2)) {
			auto poison_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Poison Resistance of {}.",
					npc_id_string,
					poison_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET PR = {} WHERE id = {}",
				poison_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit pr [Resistance] - Sets an NPC's Poison Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "dr")) {
		if (sep->IsNumber(2)) {
			auto disease_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Disease Resistance of {}.",
					npc_id_string,
					disease_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET DR = {} WHERE id = {}",
				disease_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit dr [Resistance] - Sets an NPC's Disease Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "fr")) {
		if (sep->IsNumber(2)) {
			auto fire_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Fire Resistance of {}.",
					npc_id_string,
					fire_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET FR = {} WHERE id = {}",
				fire_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit fr [Resistance] - Sets an NPC's Fire Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "cr")) {
		if (sep->IsNumber(2)) {
			auto cold_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Cold Resistance of {}.",
					npc_id_string,
					cold_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET CR = {} WHERE id = {}",
				cold_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit cr [Resistance] - Sets an NPC's Cold Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "corrup")) {
		if (sep->IsNumber(2)) {
			auto corruption_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Corruption Resistance of {}.",
					npc_id_string,
					corruption_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET corrup = {} WHERE id = {}",
				corruption_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit corrup [Resistance] - Sets an NPC's Corruption Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "phr")) {
		if (sep->IsNumber(2)) {
			auto physical_resist = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Physical Resistance of {}.",
					npc_id_string,
					physical_resist
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET PhR = {} WHERE id = {}",
				physical_resist,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit phr [Resistance] - Sets and NPC's Physical Resistance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "seeinvis")) {
		if (sep->IsNumber(2)) {
			auto see_invisible = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} can {} See Invisible.",
					npc_id_string,
					see_invisible ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET see_invis = {} WHERE id = {}",
				see_invisible,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seeinvis [Flag] - Sets an NPC's See Invisible Flag [0 = Cannot See Invisible, 1 = Can See Invisible]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "seeinvisundead")) {
		if (sep->IsNumber(2)) {
			auto see_invisible_undead = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} can {} See Invisible vs. Undead.",
					npc_id_string,
					see_invisible_undead ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET see_invis_undead = {} WHERE id = {}",
				see_invisible_undead,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seeinvisundead [Flag] - Sets an NPC's See Invisible vs. Undead Flag  [0 = Cannot See Invisible vs. Undead, 1 = Can See Invisible vs. Undead]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "qglobal")) {
		if (sep->IsNumber(2)) {
			auto use_qglobals = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} can {} use Quest Globals.",
					npc_id_string,
					use_qglobals ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET qglobal = {} WHERE id = {}",
				use_qglobals,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit qglobal [Flag] - Sets an NPC's Quest Global Flag [0 = Quest Globals Off, 1 = Quest Globals On]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "ac")) {
		if (sep->IsNumber(2)) {
			auto armor_class = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Armor Class.",
					npc_id_string,
					armor_class
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET ac = {} WHERE id = {}",
				armor_class,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit ac [Armor Class] - Sets an NPC's Armor Class");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "npcaggro")) {
		if (sep->IsNumber(2)) {
			auto aggro_npcs = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} will {} aggro other NPCs that have a hostile faction.",
					npc_id_string,
					aggro_npcs ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET npc_aggro = {} WHERE id = {}",
				aggro_npcs,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit npcaggro [Flag] - Sets an NPC's NPC Aggro Flag [0 = Aggro NPCs Off, 1 = Aggro NPCs On]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "spawn_limit")) {
		if (sep->IsNumber(2)) {
			auto spawn_limit = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Spawn Limit of {}.",
					npc_id_string,
					spawn_limit
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET spawn_limit = {} WHERE id = {}",
				spawn_limit,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit spawn_limit [Limit] - Sets an NPC's Spawn Limit Counter");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "attackspeed")) {
		if (sep->IsNumber(2)) {
			auto attack_speed = std::stof(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has an Attack Speed of {:.2f}.",
					npc_id_string,
					attack_speed
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET attack_speed = {:.2f} WHERE id = {}",
				attack_speed,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit attackspeed [Attack Speed] - Sets an NPC's Attack Speed Modifier");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "attackdelay")) {
		if (sep->IsNumber(2)) {
			auto attack_delay = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has an Attack Delay of {}.",
					npc_id_string,
					attack_delay
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET attack_delay = {} WHERE id = {}",
				attack_delay,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit attackdelay [Attack Delay] - Sets an NPC's Attack Delay");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "findable")) {
		if (sep->IsNumber(2)) {
			auto is_findable = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is {} Findable.",
					npc_id_string,
					is_findable ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET findable = {} WHERE id = {}",
				is_findable,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit findable [Flag] - Sets an NPC's Findable Flag [0 = Not Findable, 1 = Findable]");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "str")) {
		if (sep->IsNumber(2)) {
			auto strength = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Strength.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET STR = {} WHERE id = {}",
				strength,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit str [Strength] - Sets an NPC's Strength");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "sta")) {
		if (sep->IsNumber(2)) {
			auto stamina = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Stamina.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET STA = {} WHERE id = {}",
				stamina,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit sta [Stamina] - Sets an NPC's Stamina");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "agi")) {
		if (sep->IsNumber(2)) {
			auto agility = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Agility.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET AGI = {} WHERE id = {}",
				agility,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit agi [Agility] - Sets an NPC's Agility");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "dex")) {
		if (sep->IsNumber(2)) {
			auto dexterity = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Dexterity.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET DEX = {} WHERE id = {}",
				dexterity,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit dex [Dexterity] - Sets an NPC's Dexterity");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "int")) {
		if (sep->IsNumber(2)) {
			auto intelligence = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Intelligence.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET _INT = {} WHERE id = {}",
				intelligence,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit int [Intelligence] - Sets an NPC's Intelligence");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "wis")) {
		if (sep->IsNumber(2)) {
			auto wisdom = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Wisdom.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET WIS = {} WHERE id = {}",
				wisdom,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit wis [Wisdom] - Sets an NPC's Wisdom");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "cha")) {
		if (sep->IsNumber(2)) {
			auto charisma = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Charisma.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET CHA = {} WHERE id = {}",
				charisma,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit cha [Charisma] - Sets an NPC's Charisma");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "seehide")) {
		if (sep->IsNumber(2)) {
			auto see_hide = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} can {} See Hide.",
					npc_id_string,
					see_hide ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET see_hide = {} WHERE id = {}",
				see_hide,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seehide [Flag] - Sets an NPC's See Hide Flag [0 = Cannot See Hide, 1 = Can See Hide]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "seeimprovedhide")) {
		if (sep->IsNumber(2)) {
			auto see_improved_hide = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} can {} See Improved Hide.",
					npc_id_string,
					see_improved_hide ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET see_improved_hide = {} WHERE id = {}",
				see_improved_hide,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seeimprovedhide [Flag] - Sets an NPC's See Improved Hide Flag [0 = Cannot See Improved Hide, 1 = Can See Improved Hide]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "trackable")) {
		if (sep->IsNumber(2)) {
			auto is_trackable = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is {} Trackable.",
					npc_id_string,
					is_trackable ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET trackable = {} WHERE id = {}",
				is_trackable,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit trackable [Flag] - Sets an NPC's Trackable Flag [0 = Not Trackable, 1 = Trackable]");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "atk")) {
		if (sep->IsNumber(2)) {
			auto attack = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Attack.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET atk = {} WHERE id = {}",
				attack,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit atk [Attack] - Sets an NPC's Attack");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "accuracy")) {
		if (sep->IsNumber(2)) {
			auto accuracy = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Accuracy.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET accuracy = {} WHERE id = {}",
				accuracy,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit accuracy [Accuracy] - Sets an NPC's Accuracy");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "avoidance")) {
		if (sep->IsNumber(2)) {
			auto avoidance = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Avoidance.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET avoidance = {} WHERE id = {}",
				avoidance,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit avoidance [Avoidance] - Sets an NPC's Avoidance");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "slow_mitigation")) {
		if (sep->IsNumber(2)) {
			auto slow_mitigation = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Slow Mitigation.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET slow_mitigation = {} WHERE id = {}",
				slow_mitigation,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit slow_mitigation [Slow Mitigation] - Sets an NPC's Slow Mitigation");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "version")) {
		if (sep->IsNumber(2)) {
			auto version = std::stoi(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Version {}.",
					npc_id_string,
					version
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET version = {} WHERE id = {}",
				version,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit version [Version] - Sets an NPC's Version");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "maxlevel")) {
		if (sep->IsNumber(2)) {
			auto max_level = static_cast<uint8>(std::stoul(sep->arg[2]));
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Maximum Level of {}.",
					npc_id_string,
					max_level
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET maxlevel = {} WHERE id = {}",
				max_level,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit maxlevel [Max Level] - Sets an NPC's Maximum Level");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "scalerate")) {
		if (sep->IsNumber(2)) {
			auto scale_rate = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Scaling Rate of {}%%.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET scalerate = {} WHERE id = {}",
				scale_rate,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit scalerate [Scale Rate] - Sets an NPC's Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "spellscale")) {
		if (sep->IsNumber(2)) {
			auto spell_scale = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Spell Scaling Rate of {}%%.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET spellscale = {} WHERE id = {}",
				spell_scale,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit spellscale [Scale Rate] - Sets an NPC's Spell Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "healscale")) {
		if (sep->IsNumber(2)) {
			auto heal_scale = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has a Heal Scaling Rate of {}%%.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET healscale = {} WHERE id = {}",
				heal_scale,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit healscale [Scale Rate] - Sets an NPC's Heal Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "no_target")) {
		if (sep->IsNumber(2)) {
			auto is_no_target = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is {} Targetable with Target Hotkey.",
					npc_id_string,
					is_no_target ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET no_target_hotkey = {} WHERE id = {}",
				is_no_target,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit no_target [Flag] - Sets an NPC's No Target Hotkey Flag [0 = Not Targetable with Target Hotkey, 1 = Targetable with Target Hotkey]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "raidtarget")) {
		if (sep->IsNumber(2)) {
			auto is_raid_target = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is {} designated as a Raid Target.",
					npc_id_string,
					is_raid_target ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET raid_target = {} WHERE id = {}",
				is_raid_target,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit raidtarget [Flag] - Sets an NPC's Raid Target Flag [0 = Not a Raid Target, 1 = Raid Target]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "armtexture")) {
		if (sep->IsNumber(2)) {
			auto arm_texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Arm Texture {}.",
					npc_id_string,
					arm_texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET armtexture = {} WHERE id = {}",
				arm_texture,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit armtexture [Texture] - Sets an NPC's Arm Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "bracertexture")) {
		if (sep->IsNumber(2)) {
			auto bracer_texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Bracer Texture {}.",
					npc_id_string,
					bracer_texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET bracertexture = {} WHERE id = {}",
				bracer_texture,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit bracertexture [Texture] - Sets an NPC's Bracer Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "handtexture")) {
		if (sep->IsNumber(2)) {
			auto hand_texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Hand Texture {}.",
					npc_id_string,
					hand_texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET handtexture = {} WHERE id = {}",
				hand_texture,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit handtexture [Texture] - Sets an NPC's Hand Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "legtexture")) {
		if (sep->IsNumber(2)) {
			auto leg_texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Leg Texture {}.",
					npc_id_string,
					leg_texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET legtexture = {} WHERE id = {}",
				leg_texture,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit legtexture [Texture] - Sets an NPC's Leg Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "feettexture")) {
		if (sep->IsNumber(2)) {
			auto feet_texture = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Feet Texture {}.",
					npc_id_string,
					feet_texture
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET feettexture = {} WHERE id = {}",
				feet_texture,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit feettexture [Texture] - Sets an NPC's Feet Texture");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "walkspeed")) {
		if (sep->IsNumber(2)) {
			auto walk_speed = std::stof(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now walks at a Walk Speed of {:.2f}.",
					npc_id_string,
					walk_speed
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET walkspeed = {:.2f} WHERE id = {}",
				walk_speed,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit walkspeed [Walk Speed] - Sets an NPC's Walk Speed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "show_name")) {
		if (sep->IsNumber(2)) {
			auto show_name = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} will {} show their name.",
					npc_id_string,
					show_name ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET show_name = {} WHERE id = {}",
				show_name,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit show_name [Flag] - Sets an NPC's Show Name Flag [0 = Hidden, 1 = Shown]");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "untargetable")) {
		if (sep->IsNumber(2)) {
			auto is_untargetable = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} will {} be untargetable.",
					npc_id_string,
					is_untargetable ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET untargetable = {} WHERE id = {}",
				is_untargetable,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit untargetable [Flag] - Sets an NPC's Untargetable Flag [0 = Targetable, 1 = Untargetable]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_ac")) {
		if (sep->IsNumber(2)) {
			auto charm_armor_class = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Armor Class while Charmed.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_ac = {} WHERE id = {}",
				charm_armor_class,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_ac [Armor Class] - Sets an NPC's Armor Class while Charmed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_min_dmg")) {
		if (sep->IsNumber(2)) {
			auto charm_minimum_damage = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now does {} Minimum Damage while Charmed.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_min_dmg = {} WHERE id = {}",
				charm_minimum_damage,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_min_dmg [Damage] - Sets an NPC's Minimum Damage while Charmed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_max_dmg")) {
		if (sep->IsNumber(2)) {
			auto charm_maximum_damage = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now does {} Maximum Damage while Charmed.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_max_dmg = {} WHERE id = {}",
				charm_maximum_damage,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_max_dmg [Damage] - Sets an NPC's Maximum Damage while Charmed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_attack_delay")) {
		if (sep->IsNumber(2)) {
			auto charm_attack_delay = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Attack Delay while Charmed.",
					npc_id_string,
					charm_attack_delay
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_attack_delay = {} WHERE id = {}",
				charm_attack_delay,
				npc_id
			);
			content_db.QueryDatabase(query);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_accuracy_rating")) {
		if (sep->IsNumber(2)) {
			auto charm_accuracy_rating = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Accuracy Rating while Charmed.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_accuracy_rating = {} WHERE id = {}",
				charm_accuracy_rating,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_accuracy_rating [Accuracy] - Sets an NPC's Accuracy Rating while Charmed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_avoidance_rating")) {
		if (sep->IsNumber(2)) {
			auto charm_avoidance_rating = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Avoidance Rating while Charmed.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_avoidance_rating = {} WHERE id = {}",
				charm_avoidance_rating,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_avoidance_rating [Avoidance] - Sets an NPC's Avoidance Rating while Charmed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "charm_atk")) {
		if (sep->IsNumber(2)) {
			auto charm_attack = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has {} Attack while Charmed.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET charm_atk = {} WHERE id = {}",
				charm_attack,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_atk [Attack] - Sets an NPC's Attack while Charmed");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "skip_global_loot")) {
		if (sep->IsNumber(2)) {
			auto skip_global_loot = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} will {} skip Global Loot.",
					npc_id_string,
					skip_global_loot ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET skip_global_loot = {} WHERE id = {}",
				skip_global_loot,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit skip_global_loot [Flag] - Sets an NPC's Skip Global Loot Flag [0 = Don't Skip, 1 = Skip"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "rarespawn")) {
		if (sep->IsNumber(2)) {
			auto is_rare_spawn = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is {} designated as a Rare Spawn.",
					npc_id_string,
					is_rare_spawn ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET rare_spawn = {} WHERE id = {}",
				is_rare_spawn,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit rarespawn [Flag] - Sets an NPC's Rare Spawn Flag [0 = Not a Rare Spawn, 1 = Rare Spawn]"
			);	
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "stuck_behavior")) {
		if (sep->IsNumber(2)) {
			auto behavior_id = (
				static_cast<uint8>(std::stoul(sep->arg[2])) > EQ::constants::StuckBehavior::EvadeCombat ?
				EQ::constants::StuckBehavior::EvadeCombat :
				static_cast<uint8>(std::stoul(sep->arg[2]))
			);
			auto behavior_name = EQ::constants::GetStuckBehaviorName(behavior_id);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Stuck Behavior {}.",
					npc_id_string,
					(
						!behavior_name.empty() ?
						fmt::format(
							"{} ({})",
							behavior_name,
							behavior_id
						) :
						std::to_string(behavior_id)
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET stuck_behavior = {} WHERE id = {}",
				behavior_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit stuck_behavior [Stuck Behavior] - Sets an NPC's Stuck Behavior [0 = Run to Target, 1 = Warp to Target, 2 = Take No Action, 3 = Evade Combat]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "flymode")) {
		if (sep->IsNumber(2)) {
			auto flymode_id = (
				static_cast<int8>(std::stoul(sep->arg[2])) > GravityBehavior::LevitateWhileRunning ?
				GravityBehavior::LevitateWhileRunning :
				static_cast<int8>(std::stoul(sep->arg[2]))
			);
			auto flymode_name = EQ::constants::GetFlyModeName(flymode_id);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Fly Mode {}.",
					npc_id_string,
					(
						!flymode_name.empty() ?
						fmt::format(
							"{} ({})",
							flymode_name,
							flymode_id
						) :
						std::to_string(flymode_id)
					)
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET flymode = {} WHERE id = {}",
				flymode_id,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit flymode [Fly Mode] - Sets an NPC's Fly Mode [0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "always_aggro")) {
		if (sep->IsNumber(2)) {
			auto always_aggro = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} will {} Always Aggro.",
					npc_id_string,
					always_aggro ? "now" : "no longer"
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET always_aggro = {} WHERE id = {}",
				always_aggro,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit always_aggro [Flag] - Sets an NPC's Always Aggro Flag [0 = Does not Always Aggro, 1 = Always Aggro]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "exp_mod")) {
		if (sep->IsNumber(2)) {
			auto experience_modifier = std::stoul(sep->arg[2]);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} now has an Experience Modifier of {}%%.",
					npc_id_string,
					commify(sep->arg[2])
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE npc_types SET exp_mod = {} WHERE id = {}",
				experience_modifier,
				npc_id
			);
			content_db.QueryDatabase(query);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit exp_mod [Modifier] - Sets an NPC's Experience Modifier [50 = 50%, 100 = 100%, 200 = 200%]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "setanimation")) {
		if (sep->IsNumber(2)) {
			auto animation_id = (
				std::stoul(sep->arg[2]) > EQ::constants::SpawnAnimations::Looting ?
				EQ::constants::SpawnAnimations::Looting :
				std::stoul(sep->arg[2])
			);
			auto animation_name = EQ::constants::GetSpawnAnimationName(animation_id);
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} is now using Spawn Animation {} on Spawn Group ID {}.",
					npc_id_string,
					(
						!animation_name.empty() ?
						fmt::format(
							"{} ({})",
							animation_name,
							animation_id
						) :
						std::to_string(animation_id)
					),
					commify(std::to_string(c->GetTarget()->CastToNPC()->GetSpawnGroupId()))
				).c_str()
			);
			auto query = fmt::format(
				"UPDATE spawn2 SET animation = {} WHERE spawngroupID = {}",
				animation_id,
				c->GetTarget()->CastToNPC()->GetSpawnGroupId()
			);
			content_db.QueryDatabase(query);

			c->GetTarget()->SetAppearance(EmuAppearance(animation_id));
		} else {
			c->Message(Chat::White, "Usage: #npcedit setanimation [Animation ID] - Sets an NPC's Animation on Spawn (Stored in spawn2 table)");
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "respawntime")) {
		if (sep->IsNumber(2)) {
			auto respawn_time = std::stoul(sep->arg[2]);
			if (respawn_time) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"{} now has a Respawn Timer of {} ({}) on Spawn Group ID {}.",
						npc_id_string,
						ConvertSecondsToTime(respawn_time),
						respawn_time,
						commify(std::to_string(c->GetTarget()->CastToNPC()->GetSpawnGroupId()))
					).c_str()
				);
				auto query = fmt::format(
					"UPDATE spawn2 SET respawntime = {} WHERE spawngroupID = {} AND version = {}",
					respawn_time,
					c->GetTarget()->CastToNPC()->GetSpawnGroupId(),
					zone->GetInstanceVersion()
				);
				content_db.QueryDatabase(query);
			} else {
				c->Message(Chat::White, "Respawn Timer must be greater than 0 seconds.");
				return;
			}
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit respawntime [Respawn Time] - Sets an NPC's Respawn Timer in Seconds (Stored in spawn2 table)"
			);
		}
		return;
	} else {
		c->Message(Chat::White, "Usage: #npcedit name [Name] - Sets an NPC's Name");
		c->Message(Chat::White, "Usage: #npcedit lastname [Last Name] - Sets an NPC's Last Name");
		c->Message(Chat::White, "Usage: #npcedit level [Level] - Sets an NPC's Level");
		c->Message(Chat::White, "Usage: #npcedit race [Race ID] - Sets an NPC's Race");
		c->Message(Chat::White, "Usage: #npcedit class [Class ID] - Sets an NPC's Class");
		c->Message(Chat::White, "Usage: #npcedit bodytype [Body Type ID] - Sets an NPC's Bodytype");
		c->Message(Chat::White, "Usage: #npcedit hp [HP] - Sets an NPC's HP");
		c->Message(Chat::White, "Usage: #npcedit mana [Mana] - Sets an NPC's Mana");
		c->Message(Chat::White, "Usage: #npcedit gender [Gender ID] - Sets an NPC's Gender");
		c->Message(Chat::White, "Usage: #npcedit texture [Texture] - Sets an NPC's Texture");
		c->Message(Chat::White, "Usage: #npcedit helmtexture [Helmet Texture] - Sets an NPC's Helmet Texture");
		c->Message(Chat::White, "Usage: #npcedit herosforgemodel [Model Number] - Sets an NPC's Hero's Forge Model");
		c->Message(Chat::White, "Usage: #npcedit size [Size] - Sets an NPC's Size");
		c->Message(Chat::White, "Usage: #npcedit hpregen [HP Regen] - Sets an NPC's HP Regen Rate Per Tick");
		c->Message(Chat::White, "Usage: #npcedit hp_regen_per_second [HP Regen] - Sets an NPC's HP Regen Rate Per Second");
		c->Message(Chat::White, "Usage: #npcedit manaregen [Mana Regen] - Sets an NPC's Mana Regen Rate Per Tick");
		c->Message(Chat::White, "Usage: #npcedit loottable [Loottable ID] - Sets an NPC's Loottable ID");
		c->Message(Chat::White, "Usage: #npcedit merchantid [Merchant ID] - Sets an NPC's Merchant ID");
		c->Message(Chat::White, "Usage: #npcedit alt_currency_id [Alternate Currency ID] - Sets an NPC's Alternate Currency ID");
		c->Message(Chat::White, "Usage: #npcedit spell [Spell List ID] - Sets an NPC's Spells List ID");
		c->Message(Chat::White, "Usage: #npcedit npc_spells_effects_id [Spell Effects ID] - Sets an NPC's Spell Effects ID");
		c->Message(Chat::White, "Usage: #npcedit faction [Faction ID] - Sets an NPC's Faction ID");
		c->Message(Chat::White, "Usage: #npcedit adventure_template_id [Template ID] - Sets an NPC's Adventure Template ID");
		c->Message(Chat::White, "Usage: #npcedit trap_template [Template ID] - Sets an NPC's Trap Template ID");
		c->Message(Chat::White, "Usage: #npcedit damage [Minimum] [Maximum] - Sets an NPC's Damage");
		c->Message(Chat::White, "Usage: #npcedit attackcount [Attack Count] - Sets an NPC's Attack Count");
		c->Message(Chat::White, "Usage: #npcedit special_attacks [Special Attacks] - Sets an NPC's Special Attacks");
		c->Message(Chat::White, "Usage: #npcedit special_abilities [Special Abilities] - Sets an NPC's Special Abilities");
		c->Message(Chat::White, "Usage: #npcedit aggroradius [Radius] - Sets an NPC's Aggro Radius");
		c->Message(Chat::White, "Usage: #npcedit assistradius [Radius] - Sets an NPC's Assist Radius");
		c->Message(Chat::White, "Usage: #npcedit featuresave - Saves an NPC's current facial features to the database");
		c->Message(Chat::White, "Usage: #npcedit armortint_id [Armor Tint ID] - Sets an NPC's Armor Tint ID");
		c->Message(Chat::White, "Usage: #npcedit color [Red] [Green] [Blue] - Sets an NPC's Red, Green, and Blue armor tint");
		c->Message(Chat::White, "Usage: #npcedit ammoidfile [ID File] - Sets an NPC's Ammo ID File");
		c->Message(
			Chat::White,
			"Usage: #npcedit weapon [Primary Model] [Secondary Model] - Sets an NPC's Primary and Secondary Weapon Model"
		);
		c->Message(Chat::White, "Usage: #npcedit meleetype [Primary Type] [Secondary Type] - Sets an NPC's Melee Skill Types");
		c->Message(Chat::White, "Usage: #npcedit rangedtype [Type] - Sets an NPC's Ranged Skill Type");
		c->Message(Chat::White, "Usage: #npcedit runspeed [Run Speed] - Sets an NPC's Run Speed");
		c->Message(Chat::White, "Usage: #npcedit mr [Resistance] - Sets an NPC's Magic Resistance");
		c->Message(Chat::White, "Usage: #npcedit pr [Resistance] - Sets an NPC's Poison Resistance");
		c->Message(Chat::White, "Usage: #npcedit dr [Resistance] - Sets an NPC's Disease Resistance");
		c->Message(Chat::White, "Usage: #npcedit fr [Resistance] - Sets an NPC's Fire Resistance");
		c->Message(Chat::White, "Usage: #npcedit cr [Resistance] - Sets an NPC's Cold Resistance");
		c->Message(Chat::White, "Usage: #npcedit corrup [Resistance] - Sets an NPC's Corruption Resistance");
		c->Message(Chat::White, "Usage: #npcedit phr [Resistance] - Sets and NPC's Physical Resistance");
		c->Message(
			Chat::White,
			"Usage: #npcedit seeinvis [Flag] - Sets an NPC's See Invisible Flag [0 = Cannot See Invisible, 1 = Can See Invisible]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit seeinvisundead [Flag] - Sets an NPC's See Invisible vs. Undead Flag  [0 = Cannot See Invisible vs. Undead, 1 = Can See Invisible vs. Undead]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit qglobal [Flag] - Sets an NPC's Quest Global Flag [0 = Quest Globals Off, 1 = Quest Globals On]"
		);
		c->Message(Chat::White, "Usage: #npcedit ac [Armor Class] - Sets an NPC's Armor Class");
		c->Message(
			Chat::White,
			"Usage: #npcedit npcaggro [Flag] - Sets an NPC's NPC Aggro Flag [0 = Aggro NPCs Off, 1 = Aggro NPCs On]"
		);
		c->Message(Chat::White, "Usage: #npcedit spawn_limit [Limit] - Sets an NPC's Spawn Limit Counter");
		c->Message(Chat::White, "Usage: #npcedit attackspeed [Attack Speed] - Sets an NPC's Attack Speed Modifier");
		c->Message(Chat::White, "Usage: #npcedit attackdelay [Attack Delay] - Sets an NPC's Attack Delay");
		c->Message(Chat::White, "Usage: #npcedit findable [Flag] - Sets an NPC's Findable Flag [0 = Not Findable, 1 = Findable]");
		c->Message(Chat::White, "Usage: #npcedit str [Strength] - Sets an NPC's Strength");
		c->Message(Chat::White, "Usage: #npcedit sta [Stamina] - Sets an NPC's Stamina");
		c->Message(Chat::White, "Usage: #npcedit agi [Agility] - Sets an NPC's Agility");
		c->Message(Chat::White, "Usage: #npcedit dex [Dexterity] - Sets an NPC's Dexterity");
		c->Message(Chat::White, "Usage: #npcedit int [Intelligence] - Sets an NPC's Intelligence");
		c->Message(Chat::White, "Usage: #npcedit wis [Wisdom] - Sets an NPC's Wisdom");
		c->Message(Chat::White, "Usage: #npcedit cha [Charisma] - Sets an NPC's Charisma");
		c->Message(
			Chat::White,
			"Usage: #npcedit seehide [Flag] - Sets an NPC's See Hide Flag [0 = Cannot See Hide, 1 = Can See Hide]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit seeimprovedhide [Flag] - Sets an NPC's See Improved Hide Flag [0 = Cannot See Improved Hide, 1 = Can See Improved Hide]"
		);
		c->Message(Chat::White, "Usage: #npcedit trackable [Flag] - Sets an NPC's Trackable Flag [0 = Not Trackable, 1 = Trackable]");
		c->Message(Chat::White, "Usage: #npcedit atk [Attack] - Sets an NPC's Attack");
		c->Message(Chat::White, "Usage: #npcedit accuracy [Accuracy] - Sets an NPC's Accuracy");
		c->Message(Chat::White, "Usage: #npcedit avoidance [Avoidance] - Sets an NPC's Avoidance");
		c->Message(Chat::White, "Usage: #npcedit slow_mitigation [Slow Mitigation] - Sets an NPC's Slow Mitigation");
		c->Message(Chat::White, "Usage: #npcedit version [Version] - Sets an NPC's Version");
		c->Message(Chat::White, "Usage: #npcedit maxlevel [Max Level] - Sets an NPC's Maximum Level");
		c->Message(Chat::White, "Usage: #npcedit scalerate [Scale Rate] - Sets an NPC's Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
		c->Message(
			Chat::White,
			"Usage: #npcedit spellscale [Scale Rate] - Sets an NPC's Spell Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit healscale [Scale Rate] - Sets an NPC's Heal Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit no_target [Flag] - Sets an NPC's No Target Hotkey Flag [0 = Not Targetable with Target Hotkey, 1 = Targetable with Target Hotkey]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit raidtarget [Flag] - Sets an NPC's Raid Target Flag [0 = Not a Raid Target, 1 = Raid Target]"
		);
		c->Message(Chat::White, "Usage: #npcedit armtexture [Texture] - Sets an NPC's Arm Texture");
		c->Message(Chat::White, "Usage: #npcedit bracertexture [Texture] - Sets an NPC's Bracer Texture");
		c->Message(Chat::White, "Usage: #npcedit handtexture [Texture] - Sets an NPC's Hand Texture");
		c->Message(Chat::White, "Usage: #npcedit legtexture [Texture] - Sets an NPC's Leg Texture");
		c->Message(Chat::White, "Usage: #npcedit feettexture [Texture] - Sets an NPC's Feet Texture");
		c->Message(Chat::White, "Usage: #npcedit walkspeed [Walk Speed] - Sets an NPC's Walk Speed");
		c->Message(Chat::White, "Usage: #npcedit show_name [Flag] - Sets an NPC's Show Name Flag [0 = Hidden, 1 = Shown]");
		c->Message(
			Chat::White,
			"Usage: #npcedit untargetable [Flag] - Sets an NPC's Untargetable Flag [0 = Targetable, 1 = Untargetable]"
		);
		c->Message(Chat::White, "Usage: #npcedit charm_ac [Armor Class] - Sets an NPC's Armor Class while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_min_dmg [Damage] - Sets an NPC's Minimum Damage while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_max_dmg [Damage] - Sets an NPC's Maximum Damage while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_attack_delay [Attack Delay] - Sets an NPC's Attack Delay while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_accuracy_rating [Accuracy] - Sets an NPC's Accuracy Rating while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_avoidance_rating [Avoidance] - Sets an NPC's Avoidance Rating while Charmed");
		c->Message(Chat::White, "Usage: #npcedit charm_atk [Attack] - Sets an NPC's Attack while Charmed");
		c->Message(
			Chat::White,
			"Usage: #npcedit skip_global_loot [Flag] - Sets an NPC's Skip Global Loot Flag [0 = Don't Skip, 1 = Skip"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit rarespawn [Flag] - Sets an NPC's Rare Spawn Flag [0 = Not a Rare Spawn, 1 = Rare Spawn]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit stuck_behavior [Stuck Behavior] - Sets an NPC's Stuck Behavior [0 = Run to Target, 1 = Warp to Target, 2 = Take No Action, 3 = Evade Combat]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit flymode [Fly Mode] - Sets an NPC's Fly Mode [0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit always_aggro [Flag] - Sets an NPC's Always Aggro Flag [0 = Does not Always Aggro, 1 = Always Aggro]"
		);
		c->Message(
			Chat::White,
			"Usage: #npcedit exp_mod [Modifier] - Sets an NPC's Experience Modifier [50 = 50%, 100 = 100%, 200 = 200%]"
		);
		c->Message(Chat::White, "Usage: #npcedit setanimation [Animation ID] - Sets an NPC's Animation on Spawn (Stored in spawn2 table)");
		c->Message(
			Chat::White,
			"Usage: #npcedit respawntime [Respawn Time] - Sets an NPC's Respawn Timer in Seconds (Stored in spawn2 table)"
		);
		return;
	}
}

