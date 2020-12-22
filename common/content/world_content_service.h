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

	bool IsClassicEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::Classic || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheRuinsOfKunarkEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheRuinsOfKunark || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheScarsOfVeliousEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheScarsOfVelious || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheShadowsOfLuclinEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheShadowsOfLuclin || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsThePlanesOfPowerEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::ThePlanesOfPower || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheLegacyOfYkeshaEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheLegacyOfYkesha || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsLostDungeonsOfNorrathEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::LostDungeonsOfNorrath || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsGatesOfDiscordEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::GatesOfDiscord || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsOmensOfWarEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::OmensOfWar || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsDragonsOfNorrathEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::DragonsOfNorrath || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsDepthsOfDarkhollowEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::DepthsOfDarkhollow || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsProphecyOfRoEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::ProphecyOfRo || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheSerpentsSpineEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheSerpentsSpine || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheBuriedSeaEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheBuriedSea || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsSecretsOfFaydwerEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::SecretsOfFaydwer || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsSeedsOfDestructionEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::SeedsOfDestruction || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsUnderfootEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::Underfoot || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsHouseOfThuleEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::HouseOfThule || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsVeilOfAlarisEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::VeilOfAlaris || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsRainOfFearEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::RainOfFear || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsCallOfTheForsakenEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::CallOfTheForsaken || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheDarkendSeaEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheDarkendSea || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheBrokenMirrorEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheBrokenMirror || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsEmpiresOfKunarkEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::EmpiresOfKunark || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsRingOfScaleEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::RingOfScale || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTheBurningLandsEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TheBurningLands || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }
	bool IsTormentOfVeliousEnabled() { return GetCurrentExpansion() >= Expansion::ExpansionNumber::TormentOfVelious || GetCurrentExpansion() == Expansion::EXPANSION_ALL; }

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
