/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "common/data_verification.h"
#include "zone/bot_command.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/doors.h"
#include "zone/object.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"

void Client::SpawnRaidBotsOnConnect(Raid* raid) {
	std::list<BotsAvailableList> bots_list;
	if (!database.botdb.LoadBotsList(CharacterID(), bots_list) || bots_list.empty()) {
		return;
	}

	raid->ForEachMember([&](const auto& m) {
		for (const auto& b : bots_list) {
			if (m.name == b.bot_name) {
				std::string buffer = "^spawn ";
				buffer.append(m.name);
				std::string silent = " silent";
				buffer.append(silent);
				bot_command_real_dispatch(this, buffer.c_str());
				auto bot = entity_list.GetBotByBotName(m.name.c_str());

				if (bot) {
					// MemberZonedIn() -> AttachLocalMember() calls
					// entity_list.SetRaid(bot, this) once it attaches
					// this bot's Mob*, which is what IsRaided() now derives
					// from (m_raid != nullptr).
					raid->MemberZonedIn(bot->CastToClient());
					bot->SetVerifiedRaid(true);
				}
			}
		}
	});
}

