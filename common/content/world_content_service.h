/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_WORLD_CONTENT_SERVICE_H
#define EQEMU_WORLD_CONTENT_SERVICE_H

#include <string>
#include <vector>

namespace Expansion {
	static const int EXPANSION_ALL        = -1;
	static const int EXPANSION_FILTER_MAX = 99;

	enum ExpansionNumber {
		Classic = 0,
		TheRuinsOfKunark,
		TheScarsOfVelious,
		TheShadowsOfLuclin,
		ThePlanesOfPower,
		TheLegacyOfYkesha,
		LostDungeonsOfNorrath,
		GatesOfDiscord,
		OmensOfWar,
		DragonsOfNorrath,
		DepthsOfDarkhollow,
		ProphecyOfRo,
		TheSerpentsSpine,
		TheBuriedSea,
		SecretsOfFaydwer,
		SeedsOfDestruction,
		Underfoot,
		HouseOfThule,
		VeilOfAlaris,
		RainOfFear,
		CallOfTheForsaken,
		TheDarkendSea,
		TheBrokenMirror,
		EmpiresOfKunark,
		RingOfScale,
		TheBurningLands,
		TormentOfVelious,
		MaxId
	};

	/**
	 * If you add to this, make sure you update LogCategory
	 */
	static const char *ExpansionName[ExpansionNumber::MaxId] = {
		"Classic",
		"The Ruins of Kunark",
		"The Scars of Velious",
		"The Shadows of Luclin",
		"The Planes of Power",
		"The Legacy of Ykesha",
		"Lost Dungeons of Norrath",
		"Gates of Discord",
		"Omens of War",
		"Dragons of Norrath",
		"Depths of Darkhollow",
		"Prophecy of Ro",
		"The Serpent's Spine",
		"The Buried Sea",
		"Secrets of Faydwer",
		"Seeds of Destruction",
		"Underfoot",
		"House of Thule",
		"Veil of Alaris",
		"Rain of Fear",
		"Call of the Forsaken",
		"The Darkened Sea",
		"The Broken Mirror",
		"Empires of Kunark",
		"Ring of Scale",
		"The Burning Lands",
		"Torment of Velious",
	};
}

class WorldContentService {
public:

	WorldContentService();

	std::string GetCurrentExpansionName();
	int GetCurrentExpansion() const;
	void SetCurrentExpansion(int current_expansion);

	bool IsClassicEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::Classic; }
	bool IsTheRuinsOfKunarkEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheRuinsOfKunark; }
	bool IsTheScarsOfVeliousEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheScarsOfVelious; }
	bool IsTheShadowsOfLuclinEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheShadowsOfLuclin; }
	bool IsThePlanesOfPowerEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::ThePlanesOfPower; }
	bool IsTheLegacyOfYkeshaEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheLegacyOfYkesha; }
	bool IsLostDungeonsOfNorrathEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::LostDungeonsOfNorrath; }
	bool IsGatesOfDiscordEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::GatesOfDiscord; }
	bool IsOmensOfWarEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::OmensOfWar; }
	bool IsDragonsOfNorrathEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::DragonsOfNorrath; }
	bool IsDepthsOfDarkhollowEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::DepthsOfDarkhollow; }
	bool IsProphecyOfRoEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::ProphecyOfRo; }
	bool IsTheSerpentsSpineEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheSerpentsSpine; }
	bool IsTheBuriedSeaEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheBuriedSea; }
	bool IsSecretsOfFaydwerEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::SecretsOfFaydwer; }
	bool IsSeedsOfDestructionEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::SeedsOfDestruction; }
	bool IsUnderfootEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::Underfoot; }
	bool IsHouseOfThuleEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::HouseOfThule; }
	bool IsVeilOfAlarisEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::VeilOfAlaris; }
	bool IsRainOfFearEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::RainOfFear; }
	bool IsCallOfTheForsakenEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::CallOfTheForsaken; }
	bool IsTheDarkendSeaEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheDarkendSea; }
	bool IsTheBrokenMirrorEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheBrokenMirror; }
	bool IsEmpiresOfKunarkEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::EmpiresOfKunark; }
	bool IsRingOfScaleEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::RingOfScale; }
	bool IsTheBurningLandsEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheBurningLands; }
	bool IsTormentOfVeliousEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TormentOfVelious; }

	bool IsCurrentExpansionClassic() { return current_expansion == Expansion::ExpansionNumber::Classic; }
	bool IsCurrentExpansionTheRuinsOfKunark() { return current_expansion == Expansion::ExpansionNumber::TheRuinsOfKunark; }
	bool IsCurrentExpansionTheScarsOfVelious() { return current_expansion == Expansion::ExpansionNumber::TheScarsOfVelious; }
	bool IsCurrentExpansionTheShadowsOfLuclin() { return current_expansion == Expansion::ExpansionNumber::TheShadowsOfLuclin; }
	bool IsCurrentExpansionThePlanesOfPower() { return current_expansion == Expansion::ExpansionNumber::ThePlanesOfPower; }
	bool IsCurrentExpansionTheLegacyOfYkesha() { return current_expansion == Expansion::ExpansionNumber::TheLegacyOfYkesha; }
	bool IsCurrentExpansionLostDungeonsOfNorrath() { return current_expansion == Expansion::ExpansionNumber::LostDungeonsOfNorrath; }
	bool IsCurrentExpansionGatesOfDiscord() { return current_expansion == Expansion::ExpansionNumber::GatesOfDiscord; }
	bool IsCurrentExpansionOmensOfWar() { return current_expansion == Expansion::ExpansionNumber::OmensOfWar; }
	bool IsCurrentExpansionDragonsOfNorrath() { return current_expansion == Expansion::ExpansionNumber::DragonsOfNorrath; }
	bool IsCurrentExpansionDepthsOfDarkhollow() { return current_expansion == Expansion::ExpansionNumber::DepthsOfDarkhollow; }
	bool IsCurrentExpansionProphecyOfRo() { return current_expansion == Expansion::ExpansionNumber::ProphecyOfRo; }
	bool IsCurrentExpansionTheSerpentsSpine() { return current_expansion == Expansion::ExpansionNumber::TheSerpentsSpine; }
	bool IsCurrentExpansionTheBuriedSea() { return current_expansion == Expansion::ExpansionNumber::TheBuriedSea; }
	bool IsCurrentExpansionSecretsOfFaydwer() { return current_expansion == Expansion::ExpansionNumber::SecretsOfFaydwer; }
	bool IsCurrentExpansionSeedsOfDestruction() { return current_expansion == Expansion::ExpansionNumber::SeedsOfDestruction; }
	bool IsCurrentExpansionUnderfoot() { return current_expansion == Expansion::ExpansionNumber::Underfoot; }
	bool IsCurrentExpansionHouseOfThule() { return current_expansion == Expansion::ExpansionNumber::HouseOfThule; }
	bool IsCurrentExpansionVeilOfAlaris() { return current_expansion == Expansion::ExpansionNumber::VeilOfAlaris; }
	bool IsCurrentExpansionRainOfFear() { return current_expansion == Expansion::ExpansionNumber::RainOfFear; }
	bool IsCurrentExpansionCallOfTheForsaken() { return current_expansion == Expansion::ExpansionNumber::CallOfTheForsaken; }
	bool IsCurrentExpansionTheDarkendSea() { return current_expansion == Expansion::ExpansionNumber::TheDarkendSea; }
	bool IsCurrentExpansionTheBrokenMirror() { return current_expansion == Expansion::ExpansionNumber::TheBrokenMirror; }
	bool IsCurrentExpansionEmpiresOfKunark() { return current_expansion == Expansion::ExpansionNumber::EmpiresOfKunark; }
	bool IsCurrentExpansionRingOfScale() { return current_expansion == Expansion::ExpansionNumber::RingOfScale; }
	bool IsCurrentExpansionTheBurningLands() { return current_expansion == Expansion::ExpansionNumber::TheBurningLands; }
	bool IsCurrentExpansionTormentOfVelious() { return current_expansion == Expansion::ExpansionNumber::TormentOfVelious; }

private:
	int current_expansion{};
	std::vector<std::string> content_flags;
public:
	const std::vector<std::string> &GetContentFlags() const;
	bool IsContentFlagEnabled(const std::string& content_flag);
	void SetContentFlags(std::vector<std::string> content_flags);
	void SetExpansionContext();
};

extern WorldContentService content_service;

#endif //EQEMU_WORLD_CONTENT_SERVICE_H
