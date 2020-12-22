/*	 EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "eqemu_config.h"
#include "misc_functions.h"

#include <iostream>
#include <sstream>

std::string EQEmuConfig::ConfigFile = "eqemu_config.json";
EQEmuConfig *EQEmuConfig::_config = nullptr;

void EQEmuConfig::parse_config()
{

	ShortName    = _root["server"]["world"].get("shortname", "").asString();
	LongName     = _root["server"]["world"].get("longname", "").asString();
	WorldAddress = _root["server"]["world"].get("address", "").asString();
	LocalAddress = _root["server"]["world"].get("localaddress", "").asString();
	MaxClients   = atoi(_root["server"]["world"].get("maxclients", "-1").asString().c_str());
	SharedKey    = _root["server"]["world"].get("key", "").asString();
	LoginCount   = 0;

	if (_root["server"]["world"]["loginserver"].isObject()) {
		LoginHost   = _root["server"]["world"]["loginserver"].get("host", "login.eqemulator.net").asString();
		LoginPort   = atoi(_root["server"]["world"]["loginserver"].get("port", "5998").asString().c_str());
		LoginLegacy = false;
		if (_root["server"]["world"]["loginserver"].get("legacy", "0").asString() == "1") { LoginLegacy = true; }
		LoginAccount  = _root["server"]["world"]["loginserver"].get("account", "").asString();
		LoginPassword = _root["server"]["world"]["loginserver"].get("password", "").asString();
	}
	else {
		char str[32];
		loginlist.Clear();
		do {
			sprintf(str, "loginserver%i", ++LoginCount);
			if (!_root["server"]["world"][str].isObject()) {
				break;
			}

			auto loginconfig = new LoginConfig;
			loginconfig->LoginHost     = _root["server"]["world"][str].get("host", "login.eqemulator.net").asString();
			loginconfig->LoginPort     = atoi(_root["server"]["world"][str].get("port", "5998").asString().c_str());
			loginconfig->LoginAccount  = _root["server"]["world"][str].get("account", "").asString();
			loginconfig->LoginPassword = _root["server"]["world"][str].get("password", "").asString();

			loginconfig->LoginLegacy = false;
			if (_root["server"]["world"][str].get("legacy", "0").asString() == "1") { loginconfig->LoginLegacy = true; }
			loginlist.Insert(loginconfig);
		} while (LoginCount < 100);
	}


	//<locked> from xml converts to json as locked: "", so i default to "false". 
	//The only way to enable locked is by switching to true, meaning this value is always false until manually set true
	Locked = false;
	if (_root["server"]["world"].get("locked", "false").asString() == "true") { Locked = true; }
	WorldIP      = _root["server"]["world"]["tcp"].get("host", "127.0.0.1").asString();
	WorldTCPPort = atoi(_root["server"]["world"]["tcp"].get("port", "9000").asString().c_str());

	TelnetIP      = _root["server"]["world"]["telnet"].get("ip", "127.0.0.1").asString();
	TelnetTCPPort = atoi(_root["server"]["world"]["telnet"].get("port", "9001").asString().c_str());
	TelnetEnabled = false;
	if (_root["server"]["world"]["telnet"].get("enabled", "false").asString() == "true") { TelnetEnabled = true; }

	WorldHTTPMimeFile = _root["server"]["world"]["http"].get("mimefile", "mime.types").asString();
	WorldHTTPPort     = atoi(_root["server"]["world"]["http"].get("port", "9080").asString().c_str());
	WorldHTTPEnabled  = false;

	if (_root["server"]["world"]["http"].get("enabled", "false").asString() == "true") {
		WorldHTTPEnabled = true;
	}

	/**
	 * UCS
	 */
	ChatHost = _root["server"]["chatserver"].get("host", "eqchat.eqemulator.net").asString();
	ChatPort = atoi(_root["server"]["chatserver"].get("port", "7778").asString().c_str());
	MailHost = _root["server"]["mailserver"].get("host", "eqmail.eqemulator.net").asString();
	MailPort = atoi(_root["server"]["mailserver"].get("port", "7778").asString().c_str());

	/**
	 * Database
	 */
	DatabaseUsername = _root["server"]["database"].get("username", "eq").asString();
	DatabasePassword = _root["server"]["database"].get("password", "eq").asString();
	DatabaseHost     = _root["server"]["database"].get("host", "localhost").asString();
	DatabasePort     = atoi(_root["server"]["database"].get("port", "3306").asString().c_str());
	DatabaseDB       = _root["server"]["database"].get("db", "eq").asString();

	/**
	 * Content Database
	 */
	ContentDbUsername = _root["server"]["content_database"].get("username", "").asString();
	ContentDbPassword = _root["server"]["content_database"].get("password", "").asString();
	ContentDbHost     = _root["server"]["content_database"].get("host", "").asString();
	ContentDbPort     = atoi(_root["server"]["content_database"].get("port", 0).asString().c_str());
	ContentDbName     = _root["server"]["content_database"].get("db", "").asString();

	/**
	 * QS
	 */
	QSDatabaseHost     = _root["server"]["qsdatabase"].get("host", "localhost").asString();
	QSDatabasePort     = atoi(_root["server"]["qsdatabase"].get("port", "3306").asString().c_str());
	QSDatabaseUsername = _root["server"]["qsdatabase"].get("username", "eq").asString();
	QSDatabasePassword = _root["server"]["qsdatabase"].get("password", "eq").asString();
	QSDatabaseDB       = _root["server"]["qsdatabase"].get("db", "eq").asString();

	/**
	 * Zones
	 */
	DefaultStatus = atoi(_root["server"]["zones"].get("defaultstatus", 0).asString().c_str());
	ZonePortLow   = atoi(_root["server"]["zones"]["ports"].get("low", "7000").asString().c_str());
	ZonePortHigh  = atoi(_root["server"]["zones"]["ports"].get("high", "7999").asString().c_str());

	/**
	 * Files
	 */
	SpellsFile      = _root["server"]["files"].get("spells", "spells_us.txt").asString();
	OpCodesFile     = _root["server"]["files"].get("opcodes", "opcodes.conf").asString();
	MailOpCodesFile = _root["server"]["files"].get("mail_opcodes", "mail_opcodes.conf").asString();
	PluginPlFile    = _root["server"]["files"].get("plugin.pl", "plugin.pl").asString();

	/**
	 * Directories
	 */
	MapDir       = _root["server"]["directories"].get("maps", "Maps/").asString();
	QuestDir     = _root["server"]["directories"].get("quests", "quests/").asString();
	PluginDir    = _root["server"]["directories"].get("plugins", "plugins/").asString();
	LuaModuleDir = _root["server"]["directories"].get("lua_modules", "lua_modules/").asString();
	PatchDir     = _root["server"]["directories"].get("patches", "./").asString();
	SharedMemDir = _root["server"]["directories"].get("shared_memory", "shared/").asString();
	LogDir       = _root["server"]["directories"].get("logs", "logs/").asString();

	/**
	 * Logs
	 */
	LogPrefix = _root["server"]["launcher"].get("logprefix", "logs/zone-").asString();
	LogSuffix = _root["server"]["launcher"].get("logsuffix", ".log").asString();

	/**
	 * Launcher
	 */
	RestartWait      = atoi(_root["server"]["launcher"]["timers"].get("restart", "10000").asString().c_str());
	TerminateWait    = atoi(_root["server"]["launcher"]["timers"].get("reterminate", "10000").asString().c_str());
	InitialBootWait  = atoi(_root["server"]["launcher"]["timers"].get("initial", "20000").asString().c_str());
	ZoneBootInterval = atoi(_root["server"]["launcher"]["timers"].get("interval", "2000").asString().c_str());
#ifdef WIN32
	ZoneExe = _root["server"]["launcher"].get("exe", "zone.exe").asString();
#else
	ZoneExe = _root["server"]["launcher"].get("exe", "./zone").asString();
#endif

}
std::string EQEmuConfig::GetByName(const std::string &var_name) const
{
	if (var_name == "ShortName") {
		return (ShortName);
	}
	if (var_name == "LongName") {
		return (LongName);
	}
	if (var_name == "WorldAddress") {
		return (WorldAddress);
	}
	if (var_name == "LoginHost") {
		return (LoginHost);
	}
	if (var_name == "LoginAccount") {
		return (LoginAccount);
	}
	if (var_name == "LoginPassword") {
		return (LoginPassword);
	}
	if (var_name == "LoginPort") {
		return (itoa(LoginPort));
	}
	if (var_name == "LoginLegacy") {
		return (itoa(LoginLegacy ? 1 : 0));
	}
	if (var_name == "Locked") {
		return (Locked ? "true" : "false");
	}
	if (var_name == "WorldTCPPort") {
		return (itoa(WorldTCPPort));
	}
	if (var_name == "WorldIP") {
		return (WorldIP);
	}
	if (var_name == "TelnetTCPPort") {
		return (itoa(TelnetTCPPort));
	}
	if (var_name == "TelnetIP") {
		return (TelnetIP);
	}
	if (var_name == "TelnetEnabled") {
		return (TelnetEnabled ? "true" : "false");
	}
	if (var_name == "WorldHTTPPort") {
		return (itoa(WorldHTTPPort));
	}
	if (var_name == "WorldHTTPMimeFile") {
		return (WorldHTTPMimeFile);
	}
	if (var_name == "WorldHTTPEnabled") {
		return (WorldHTTPEnabled ? "true" : "false");
	}
	if (var_name == "ChatHost") {
		return (ChatHost);
	}
	if (var_name == "ChatPort") {
		return (itoa(ChatPort));
	}
	if (var_name == "MailHost") {
		return (MailHost);
	}
	if (var_name == "MailPort") {
		return (itoa(MailPort));
	}
	if (var_name == "DatabaseHost") {
		return (DatabaseHost);
	}
	if (var_name == "DatabaseUsername") {
		return (DatabaseUsername);
	}
	if (var_name == "DatabasePassword") {
		return (DatabasePassword);
	}
	if (var_name == "DatabaseDB") {
		return (DatabaseDB);
	}
	if (var_name == "DatabasePort") {
		return (itoa(DatabasePort));
	}
	if (var_name == "QSDatabaseHost") {
		return (QSDatabaseHost);
	}
	if (var_name == "QSDatabaseUsername") {
		return (QSDatabaseUsername);
	}
	if (var_name == "QSDatabasePassword") {
		return (QSDatabasePassword);
	}
	if (var_name == "QSDatabaseDB") {
		return (QSDatabaseDB);
	}
	if (var_name == "QSDatabasePort") {
		return (itoa(QSDatabasePort));
	}
	if (var_name == "SpellsFile") {
		return (SpellsFile);
	}
	if (var_name == "OpCodesFile") {
		return (OpCodesFile);
	}
	if (var_name == "MailOpCodesFile") {
		return (MailOpCodesFile);
	}
	if (var_name == "PluginPlFile") {
		return (PluginPlFile);
	}
	if (var_name == "MapDir") {
		return (MapDir);
	}
	if (var_name == "QuestDir") {
		return (QuestDir);
	}
	if (var_name == "PluginDir") {
		return (PluginDir);
	}
	if (var_name == "LuaModuleDir") {
		return (LuaModuleDir);
	}
	if (var_name == "PatchDir") {
		return (PatchDir);
	}
	if (var_name == "SharedMemDir") {
		return (SharedMemDir);
	}
	if (var_name == "LogDir") {
		return (LogDir);
	}
	if (var_name == "LogPrefix") {
		return (LogPrefix);
	}
	if (var_name == "LogSuffix") {
		return (LogSuffix);
	}
	if (var_name == "ZoneExe") {
		return (ZoneExe);
	}
	if (var_name == "ZonePortLow") {
		return (itoa(ZonePortLow));
	}
	if (var_name == "ZonePortHigh") {
		return (itoa(ZonePortHigh));
	}
	if (var_name == "DefaultStatus") {
		return (itoa(DefaultStatus));
	}
//	if(var_name == "DynamicCount")
//		return(itoa(DynamicCount));
	return ("");
}

void EQEmuConfig::Dump() const
{
	std::cout << "ShortName = " << ShortName << std::endl;
	std::cout << "LongName = " << LongName << std::endl;
	std::cout << "WorldAddress = " << WorldAddress << std::endl;
	std::cout << "LoginHost = " << LoginHost << std::endl;
	std::cout << "LoginAccount = " << LoginAccount << std::endl;
	std::cout << "LoginPassword = " << LoginPassword << std::endl;
	std::cout << "LoginPort = " << LoginPort << std::endl;
	std::cout << "LoginLegacy = " << LoginLegacy << std::endl;
	std::cout << "Locked = " << Locked << std::endl;
	std::cout << "WorldTCPPort = " << WorldTCPPort << std::endl;
	std::cout << "WorldIP = " << WorldIP << std::endl;
	std::cout << "TelnetTCPPort = " << TelnetTCPPort << std::endl;
	std::cout << "TelnetIP = " << TelnetIP << std::endl;
	std::cout << "TelnetEnabled = " << TelnetEnabled << std::endl;
	std::cout << "WorldHTTPPort = " << WorldHTTPPort << std::endl;
	std::cout << "WorldHTTPMimeFile = " << WorldHTTPMimeFile << std::endl;
	std::cout << "WorldHTTPEnabled = " << WorldHTTPEnabled << std::endl;
	std::cout << "ChatHost = " << ChatHost << std::endl;
	std::cout << "ChatPort = " << ChatPort << std::endl;
	std::cout << "MailHost = " << MailHost << std::endl;
	std::cout << "MailPort = " << MailPort << std::endl;
	std::cout << "DatabaseHost = " << DatabaseHost << std::endl;
	std::cout << "DatabaseUsername = " << DatabaseUsername << std::endl;
	std::cout << "DatabasePassword = " << DatabasePassword << std::endl;
	std::cout << "DatabaseDB = " << DatabaseDB << std::endl;
	std::cout << "DatabasePort = " << DatabasePort << std::endl;
	std::cout << "QSDatabaseHost = " << QSDatabaseHost << std::endl;
	std::cout << "QSDatabaseUsername = " << QSDatabaseUsername << std::endl;
	std::cout << "QSDatabasePassword = " << QSDatabasePassword << std::endl;
	std::cout << "QSDatabaseDB = " << QSDatabaseDB << std::endl;
	std::cout << "QSDatabasePort = " << QSDatabasePort << std::endl;
	std::cout << "SpellsFile = " << SpellsFile << std::endl;
	std::cout << "OpCodesFile = " << OpCodesFile << std::endl;
	std::cout << "MailOpcodesFile = " << MailOpCodesFile << std::endl;
	std::cout << "PluginPlFile = " << PluginPlFile << std::endl;
	std::cout << "MapDir = " << MapDir << std::endl;
	std::cout << "QuestDir = " << QuestDir << std::endl;
	std::cout << "PluginDir = " << PluginDir << std::endl;
	std::cout << "LuaModuleDir = " << LuaModuleDir << std::endl;
	std::cout << "PatchDir = " << PatchDir << std::endl;
	std::cout << "SharedMemDir = " << SharedMemDir << std::endl;
	std::cout << "LogDir = " << LogDir << std::endl;
	std::cout << "ZonePortLow = " << ZonePortLow << std::endl;
	std::cout << "ZonePortHigh = " << ZonePortHigh << std::endl;
	std::cout << "DefaultStatus = " << (int) DefaultStatus << std::endl;
//	std::cout << "DynamicCount = " << DynamicCount << std::endl;
}
