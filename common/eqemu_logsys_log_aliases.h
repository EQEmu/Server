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

#ifndef EQEMU_EQEMU_LOGSYS_LOG_ALIASES_H
#define EQEMU_EQEMU_LOGSYS_LOG_ALIASES_H

#ifdef BUILD_LOGGING

/**
 * RFC 5424
 */

#define LogEmergency(message, ...) do {\
    if (LogSys.log_settings[Logs::Emergency].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Emergency, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAlert(message, ...) do {\
    if (LogSys.log_settings[Logs::Alert].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Alert, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCritical(message, ...) do {\
    if (LogSys.log_settings[Logs::Critical].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Critical, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogError(message, ...) do {\
    if (LogSys.log_settings[Logs::Error].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Error, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogWarning(message, ...) do {\
    if (LogSys.log_settings[Logs::Warning].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Warning, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNotice(message, ...) do {\
    if (LogSys.log_settings[Logs::Notice].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Notice, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInfo(message, ...) do {\
    if (LogSys.log_settings[Logs::Info].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Info, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDebug(message, ...) do {\
    if (LogSys.log_settings[Logs::Debug].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Debug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

/**
 * Category
 */

#define LogAA(message, ...) do {\
    if (LogSys.log_settings[Logs::AA].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::AA, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAADetail(message, ...) do {\
    if (LogSys.log_settings[Logs::AA].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::AA, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAI(message, ...) do {\
    if (LogSys.log_settings[Logs::AI].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::AI, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::AI].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::AI, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAggro(message, ...) do {\
    if (LogSys.log_settings[Logs::Aggro].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Aggro, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAggroDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Aggro].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Aggro, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAttack(message, ...) do {\
    if (LogSys.log_settings[Logs::Attack].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Attack, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAttackDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Attack].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Attack, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketClientServer(message, ...) do {\
    if (LogSys.log_settings[Logs::PacketClientServer].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::PacketClientServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketClientServerDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::PacketClientServer].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::PacketClientServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombat(message, ...) do {\
    if (LogSys.log_settings[Logs::Combat].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Combat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombatDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Combat].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Combat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCommands(message, ...) do {\
    if (LogSys.log_settings[Logs::Commands].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Commands, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCommandsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Commands].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Commands, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCrash(message, ...) do {\
    if (LogSys.log_settings[Logs::Crash].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Crash, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCrashDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Crash].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Crash, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDoors(message, ...) do {\
    if (LogSys.log_settings[Logs::Doors].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Doors, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDoorsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Doors].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Doors, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogGroup(message, ...) do {\
    if (LogSys.log_settings[Logs::Group].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Group, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogGroupDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Group].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Group, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogGuilds(message, ...) do {\
    if (LogSys.log_settings[Logs::Guilds].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Guilds, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogGuildsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Guilds].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Guilds, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInventory(message, ...) do {\
    if (LogSys.log_settings[Logs::Inventory].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Inventory, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInventoryDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Inventory].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Inventory, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLauncher(message, ...) do {\
    if (LogSys.log_settings[Logs::Launcher].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Launcher, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLauncherDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Launcher].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Launcher, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetcode(message, ...) do {\
    if (LogSys.log_settings[Logs::Netcode].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Netcode, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetcodeDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Netcode].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Netcode, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNormal(message, ...) do {\
    if (LogSys.log_settings[Logs::Normal].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Normal, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNormalDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Normal].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Normal, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogObject(message, ...) do {\
    if (LogSys.log_settings[Logs::Object].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Object, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogObjectDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Object].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Object, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPathing(message, ...) do {\
    if (LogSys.log_settings[Logs::Pathing].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Pathing, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPathingDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Pathing].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Pathing, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQSServer(message, ...) do {\
    if (LogSys.log_settings[Logs::QSServer].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::QSServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQSServerDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::QSServer].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::QSServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuests(message, ...) do {\
    if (LogSys.log_settings[Logs::Quests].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Quests, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Quests].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Quests, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogRules(message, ...) do {\
    if (LogSys.log_settings[Logs::Rules].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Rules, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogRulesDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Rules].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Rules, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSkills(message, ...) do {\
    if (LogSys.log_settings[Logs::Skills].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Skills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSkillsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Skills].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Skills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpawns(message, ...) do {\
    if (LogSys.log_settings[Logs::Spawns].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Spawns, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpawnsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Spawns].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Spawns, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpells(message, ...) do {\
    if (LogSys.log_settings[Logs::Spells].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Spells, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpellsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Spells].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Spells, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTCPConnection(message, ...) do {\
    if (LogSys.log_settings[Logs::TCPConnection].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::TCPConnection, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTCPConnectionDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::TCPConnection].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::TCPConnection, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTasks(message, ...) do {\
    if (LogSys.log_settings[Logs::Tasks].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Tasks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTasksDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Tasks].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Tasks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTradeskills(message, ...) do {\
    if (LogSys.log_settings[Logs::Tradeskills].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Tradeskills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTradeskillsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Tradeskills].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Tradeskills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTrading(message, ...) do {\
    if (LogSys.log_settings[Logs::Trading].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Trading, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTradingDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Trading].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Trading, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTribute(message, ...) do {\
    if (LogSys.log_settings[Logs::Tribute].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Tribute, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTributeDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Tribute].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Tribute, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLError(message, ...) do {\
    if (LogSys.log_settings[Logs::MySQLError].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::MySQLError, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLErrorDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::MySQLError].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::MySQLError, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLQuery(message, ...) do {\
    if (LogSys.log_settings[Logs::MySQLQuery].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::MySQLQuery, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLQueryDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::MySQLQuery].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::MySQLQuery, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMercenaries(message, ...) do {\
    if (LogSys.log_settings[Logs::Mercenaries].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Mercenaries, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMercenariesDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Mercenaries].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Mercenaries, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestDebug(message, ...) do {\
    if (LogSys.log_settings[Logs::QuestDebug].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::QuestDebug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestDebugDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::QuestDebug].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::QuestDebug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLoginserver(message, ...) do {\
    if (LogSys.log_settings[Logs::Loginserver].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Loginserver, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLoginserverDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Loginserver].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Loginserver, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientLogin(message, ...) do {\
    if (LogSys.log_settings[Logs::ClientLogin].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::ClientLogin, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientLoginDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::ClientLogin].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::ClientLogin, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHeadlessClient(message, ...) do {\
    if (LogSys.log_settings[Logs::HeadlessClient].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::HeadlessClient, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHeadlessClientDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::HeadlessClient].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::HeadlessClient, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHPUpdate(message, ...) do {\
    if (LogSys.log_settings[Logs::HPUpdate].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::HPUpdate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHPUpdateDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::HPUpdate].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::HPUpdate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFixZ(message, ...) do {\
    if (LogSys.log_settings[Logs::FixZ].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::FixZ, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFixZDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::FixZ].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::FixZ, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFood(message, ...) do {\
    if (LogSys.log_settings[Logs::Food].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Food, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFoodDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Food].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Food, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTraps(message, ...) do {\
    if (LogSys.log_settings[Logs::Traps].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Traps, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTrapsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Traps].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Traps, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCRoamBox(message, ...) do {\
    if (LogSys.log_settings[Logs::NPCRoamBox].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::NPCRoamBox, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCRoamBoxDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::NPCRoamBox].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::NPCRoamBox, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCScaling(message, ...) do {\
    if (LogSys.log_settings[Logs::NPCScaling].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::NPCScaling, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCScalingDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::NPCScaling].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::NPCScaling, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMobAppearance(message, ...) do {\
    if (LogSys.log_settings[Logs::MobAppearance].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::MobAppearance, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMobAppearanceDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::MobAppearance].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::MobAppearance, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogStatus(message, ...) do {\
    if (LogSys.log_settings[Logs::Status].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Status, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogStatusDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Status].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Status, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIScanClose(message, ...) do {\
    if (LogSys.log_settings[Logs::AIScanClose].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::AIScanClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIScanCloseDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::AIScanClose].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::AIScanClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIYellForHelp(message, ...) do {\
    if (LogSys.log_settings[Logs::AIYellForHelp].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::AIYellForHelp, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIYellForHelpDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::AIYellForHelp].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::AIYellForHelp, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAICastBeneficialClose(message, ...) do {\
    if (LogSys.log_settings[Logs::AICastBeneficialClose].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::AICastBeneficialClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAICastBeneficialCloseDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::AICastBeneficialClose].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::AICastBeneficialClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAoeCast(message, ...) do {\
    if (LogSys.log_settings[Logs::AoeCast].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::AoeCast, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAoeCastDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::AoeCast].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::AoeCast, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEntityManagement(message, ...) do {\
    if (LogSys.log_settings[Logs::EntityManagement].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::EntityManagement, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEntityManagementDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::EntityManagement].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::EntityManagement, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFlee(message, ...) do {\
    if (LogSys.log_settings[Logs::Flee].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Flee, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFleeDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Flee].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Flee, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAura(message, ...) do {\
    if (LogSys.log_settings[Logs::Aura].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Aura, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAuraDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Aura].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Aura, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHotReload(message, ...) do {\
    if (LogSys.log_settings[Logs::HotReload].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::HotReload, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHotReloadDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::HotReload].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::HotReload, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMerchants(message, ...) do {\
    if (LogSys.log_settings[Logs::Merchants].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Merchants, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMerchantsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Merchants].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Merchants, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZonePoints(message, ...) do {\
    if (LogSys.log_settings[Logs::ZonePoints].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::ZonePoints, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZonePointsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::ZonePoints].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::ZonePoints, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLoot(message, ...) do {\
    if (LogSys.log_settings[Logs::Loot].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Loot, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLootDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Loot].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Loot, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogExpeditions(message, ...) do {\
    if (LogSys.log_settings[Logs::Expeditions].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Expeditions, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogExpeditionsModerate(message, ...) do {\
    if (LogSys.log_settings[Logs::Expeditions].is_category_enabled == 1)\
        OutF(LogSys, Logs::Moderate, Logs::Expeditions, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogExpeditionsDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Expeditions].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Expeditions, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDynamicZones(message, ...) do {\
    if (LogSys.log_settings[Logs::DynamicZones].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::DynamicZones, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDynamicZonesDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::DynamicZones].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::DynamicZones, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogScheduler(message, ...) do {\
    if (LogSys.log_settings[Logs::Scheduler].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Scheduler, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSchedulerDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Scheduler].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Scheduler, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCheat(message, ...) do {\
    if (LogSys.log_settings[Logs::Cheat].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Cheat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCheatDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Cheat].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Cheat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientList(message, ...) do {\
    if (LogSys.log_settings[Logs::ClientList].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::ClientList, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientListDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::ClientList].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::ClientList, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiaWind(message, ...) do {\
    if (LogSys.log_settings[Logs::DiaWind].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::DiaWind, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiaWindDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::DiaWind].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::DiaWind, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHTTP(message, ...) do {\
    if (LogSys.log_settings[Logs::HTTP].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::HTTP, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHTTPDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::HTTP].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::HTTP, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSaylink(message, ...) do {\
    if (LogSys.log_settings[Logs::Saylink].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Saylink, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSaylinkDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Saylink].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Saylink, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogChecksumVerification(message, ...) do {\
    if (LogSys.log_settings[Logs::ChecksumVerification].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::ChecksumVerification, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogChecksumVerificationDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::ChecksumVerification].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::ChecksumVerification, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombatRecord(message, ...) do {\
    if (LogSys.log_settings[Logs::CombatRecord].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::CombatRecord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombatRecordDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::CombatRecord].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::CombatRecord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHate(message, ...) do {\
    if (LogSys.log_settings[Logs::Hate].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Hate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHateDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Hate].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Hate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiscord(message, ...) do {\
    if (LogSys.log_settings[Logs::Discord].is_category_enabled == 1)\
        OutF(LogSys, Logs::General, Logs::Discord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiscordDetail(message, ...) do {\
    if (LogSys.log_settings[Logs::Discord].is_category_enabled == 1)\
        OutF(LogSys, Logs::Detail, Logs::Discord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define Log(debug_level, log_category, message, ...) do {\
    if (LogSys.log_settings[log_category].is_category_enabled == 1)\
        LogSys.Out(debug_level, log_category, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogF(debug_level, log_category, message, ...) do {\
    if (LogSys.log_settings[log_category].is_category_enabled == 1)\
        OutF(LogSys, debug_level, log_category, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#else
#define LogEmergency(message, ...) do {\
} while (0)

#define LogAlert(message, ...) do {\
} while (0)

#define LogCritical(message, ...) do {\
} while (0)

#define LogError(message, ...) do {\
} while (0)

#define LogWarning(message, ...) do {\
} while (0)

#define LogNotice(message, ...) do {\
} while (0)

#define LogInfo(message, ...) do {\
} while (0)

#define LogDebug(message, ...) do {\
} while (0)

/**
 * Category
 */

#define LogAA(message, ...) do {\
} while (0)

#define LogAADetail(message, ...) do {\
} while (0)

#define LogAI(message, ...) do {\
} while (0)

#define LogAIDetail(message, ...) do {\
} while (0)

#define LogAggro(message, ...) do {\
} while (0)

#define LogAggroDetail(message, ...) do {\
} while (0)

#define LogAttack(message, ...) do {\
} while (0)

#define LogAttackDetail(message, ...) do {\
} while (0)

#define LogPacketClientServer(message, ...) do {\
} while (0)

#define LogPacketClientServerDetail(message, ...) do {\
} while (0)

#define LogCombat(message, ...) do {\
} while (0)

#define LogCombatDetail(message, ...) do {\
} while (0)

#define LogCommands(message, ...) do {\
} while (0)

#define LogCommandsDetail(message, ...) do {\
} while (0)

#define LogCrash(message, ...) do {\
} while (0)

#define LogCrashDetail(message, ...) do {\
} while (0)

#define LogDoors(message, ...) do {\
} while (0)

#define LogDoorsDetail(message, ...) do {\
} while (0)

#define LogGroup(message, ...) do {\
} while (0)

#define LogGroupDetail(message, ...) do {\
} while (0)

#define LogGuilds(message, ...) do {\
} while (0)

#define LogGuildsDetail(message, ...) do {\
} while (0)

#define LogInventory(message, ...) do {\
} while (0)

#define LogInventoryDetail(message, ...) do {\
} while (0)

#define LogLauncher(message, ...) do {\
} while (0)

#define LogLauncherDetail(message, ...) do {\
} while (0)

#define LogNetcode(message, ...) do {\
} while (0)

#define LogNetcodeDetail(message, ...) do {\
} while (0)

#define LogNormal(message, ...) do {\
} while (0)

#define LogNormalDetail(message, ...) do {\
} while (0)

#define LogObject(message, ...) do {\
} while (0)

#define LogObjectDetail(message, ...) do {\
} while (0)

#define LogPathing(message, ...) do {\
} while (0)

#define LogPathingDetail(message, ...) do {\
} while (0)

#define LogQSServer(message, ...) do {\
} while (0)

#define LogQSServerDetail(message, ...) do {\
} while (0)

#define LogQuests(message, ...) do {\
} while (0)

#define LogQuestsDetail(message, ...) do {\
} while (0)

#define LogRules(message, ...) do {\
} while (0)

#define LogRulesDetail(message, ...) do {\
} while (0)

#define LogSkills(message, ...) do {\
} while (0)

#define LogSkillsDetail(message, ...) do {\
} while (0)

#define LogSpawns(message, ...) do {\
} while (0)

#define LogSpawnsDetail(message, ...) do {\
} while (0)

#define LogSpells(message, ...) do {\
} while (0)

#define LogSpellsDetail(message, ...) do {\
} while (0)

#define LogTCPConnection(message, ...) do {\
} while (0)

#define LogTCPConnectionDetail(message, ...) do {\
} while (0)

#define LogTasks(message, ...) do {\
} while (0)

#define LogTasksDetail(message, ...) do {\
} while (0)

#define LogTradeskills(message, ...) do {\
} while (0)

#define LogTradeskillsDetail(message, ...) do {\
} while (0)

#define LogTrading(message, ...) do {\
} while (0)

#define LogTradingDetail(message, ...) do {\
} while (0)

#define LogTribute(message, ...) do {\
} while (0)

#define LogTributeDetail(message, ...) do {\
} while (0)

#define LogMySQLError(message, ...) do {\
} while (0)

#define LogMySQLErrorDetail(message, ...) do {\
} while (0)

#define LogMySQLQuery(message, ...) do {\
} while (0)

#define LogMySQLQueryDetail(message, ...) do {\
} while (0)

#define LogMercenaries(message, ...) do {\
} while (0)

#define LogMercenariesDetail(message, ...) do {\
} while (0)

#define LogQuestDebug(message, ...) do {\
} while (0)

#define LogQuestDebugDetail(message, ...) do {\
} while (0)

#define LogLoginserver(message, ...) do {\
} while (0)

#define LogLoginserverDetail(message, ...) do {\
} while (0)

#define LogClientLogin(message, ...) do {\
} while (0)

#define LogClientLoginDetail(message, ...) do {\
} while (0)

#define LogHeadlessClient(message, ...) do {\
} while (0)

#define LogHeadlessClientDetail(message, ...) do {\
} while (0)

#define LogHPUpdate(message, ...) do {\
} while (0)

#define LogHPUpdateDetail(message, ...) do {\
} while (0)

#define LogFixZ(message, ...) do {\
} while (0)

#define LogFixZDetail(message, ...) do {\
} while (0)

#define LogFood(message, ...) do {\
} while (0)

#define LogFoodDetail(message, ...) do {\
} while (0)

#define LogTraps(message, ...) do {\
} while (0)

#define LogTrapsDetail(message, ...) do {\
} while (0)

#define LogNPCRoamBox(message, ...) do {\
} while (0)

#define LogNPCRoamBoxDetail(message, ...) do {\
} while (0)

#define LogNPCScaling(message, ...) do {\
} while (0)

#define LogNPCScalingDetail(message, ...) do {\
} while (0)

#define LogMobAppearance(message, ...) do {\
} while (0)

#define LogMobAppearanceDetail(message, ...) do {\
} while (0)

#define LogStatus(message, ...) do {\
} while (0)

#define LogStatusDetail(message, ...) do {\
} while (0)

#define LogAIScanClose(message, ...) do {\
} while (0)

#define LogAIScanCloseDetail(message, ...) do {\
} while (0)

#define LogAIYellForHelp(message, ...) do {\
} while (0)

#define LogAIYellForHelpDetail(message, ...) do {\
} while (0)

#define LogAICastBeneficialClose(message, ...) do {\
} while (0)

#define LogAICastBeneficialCloseDetail(message, ...) do {\
} while (0)

#define LogAoeCast(message, ...) do {\
} while (0)

#define LogAoeCastDetail(message, ...) do {\
} while (0)

#define LogEntityManagement(message, ...) do {\
} while (0)

#define LogEntityManagementDetail(message, ...) do {\
} while (0)

#define LogFlee(message, ...) do {\
} while (0)

#define LogFleeDetail(message, ...) do {\
} while (0)

#define LogAura(message, ...) do {\
} while (0)

#define LogAuraDetail(message, ...) do {\
} while (0)

#define LogHotReload(message, ...) do {\
} while (0)

#define LogHotReloadDetail(message, ...) do {\
} while (0)

#define LogMerchants(message, ...) do {\
} while (0)

#define LogMerchantsDetail(message, ...) do {\
} while (0)

#define LogZonePoints(message, ...) do {\
} while (0)

#define LogZonePointsDetail(message, ...) do {\
} while (0)

#define LogExpeditions(message, ...) do {\
} while (0)

#define LogExpeditionsModerate(message, ...) do {\
} while (0)

#define LogExpeditionsDetail(message, ...) do {\
} while (0)

#define LogDynamicZones(message, ...) do {\
} while (0)

#define LogDynamicZonesDetail(message, ...) do {\
} while (0)

#define LogCheatList(message, ...) do {\
} while (0)

#define LogCheatDetail(message, ...) do {\
} while (0)

#define LogClientList(message, ...) do {\
} while (0)

#define LogClientListDetail(message, ...) do {\
} while (0)

#define LogDiaWind(message, ...) do {\
} while (0)

#define LogDiaWindDetail(message, ...) do {\
} while (0)

#define LogHTTP(message, ...) do {\
} while (0)

#define LogHTTPDetail(message, ...) do {\
} while (0)

#define LogSaylink(message, ...) do {\
} while (0)

#define LogSaylinkDetail(message, ...) do {\
} while (0)

#define LogScheduler(message, ...) do {\
} while (0)

#define LogSchedulerDetail(message, ...) do {\
} while (0)

#define LogCheat(message, ...) do {\
} while (0)

#define LogCheatDetail(message, ...) do {\
} while (0)

#define LogLoot(message, ...) do {\
} while (0)

#define LogLootDetail(message, ...) do {\
} while (0)

#define LogFood(message, ...) do {\
} while (0)

#define LogFoodDetail(message, ...) do {\
} while (0)

#define LogChecksumVerification(message, ...) do {\
} while (0)

#define LogChecksumVerificationDetail(message, ...) do {\
} while (0)

#define LogCombatRecord(message, ...) do {\
} while (0)

#define LogCombatRecordDetail(message, ...) do {\
} while (0)

#define LogHate(message, ...) do {\
} while (0)

#define LogHateDetail(message, ...) do {\
} while (0)

#define Log(debug_level, log_category, message, ...) do {\
} while (0)

#define LogF(debug_level, log_category, message, ...) do {\
} while (0)
#endif

#endif //EQEMU_EQEMU_LOGSYS_LOG_ALIASES_H
