/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#ifdef EMBPERL

#include "../common/global_define.h"
#include "eqw.h"
#include "eqw_parser.h"
#include "world_config.h"
#include "../common/races.h"
#include "../common/classes.h"
#include "../common/misc.h"
#include "../common/string_util.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "cliententry.h"
#include "login_server.h"
#include "login_server_list.h"
#include "worlddb.h"
#include "client.h"
#include "launcher_list.h"
#include "launcher_link.h"
#include "wguild_mgr.h"

#ifdef seed
#undef seed
#endif

#include <algorithm>

extern ZSList	zoneserver_list;
extern ClientList	client_list;
extern uint32	numzones;
extern LoginServerList loginserverlist;
extern LauncherList launcher_list;
extern volatile bool	RunLoops;

EQW EQW::s_EQW;

//IO Capture routine
XS(XS_EQWIO_PRINT); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQWIO_PRINT)
{
	dXSARGS;
	if (items < 2)
		return;

	int r;
	for(r = 1; r < items; r++) {
		char *str = SvPV_nolen(ST(r));
		EQW::Singleton()->AppendOutput(str);
	}

	XSRETURN_EMPTY;
}

EQW::EQW() {
}

void EQW::AppendOutput(const char *str) {
	m_outputBuffer += str;
//	Log.LogDebugType(Logs::Detail, Logs::World_Server, "Append %d chars, yeilding result of length %d", strlen(str), m_outputBuffer.length());
}

const std::string &EQW::GetOutput() const {
//	Log.LogDebugType(Logs::Detail, Logs::World_Server, "Getting, length %d", m_outputBuffer.length());
	return(m_outputBuffer);
}

void EQW::LockWorld() {
	WorldConfig::LockWorld();
	if (loginserverlist.Connected()) {
		loginserverlist.SendStatus();
	}
}

void EQW::UnlockWorld() {
	WorldConfig::UnlockWorld();
	if (loginserverlist.Connected()) {
		loginserverlist.SendStatus();
	}
}

Const_char *EQW::GetConfig(Const_char *var_name) {
	m_returnBuffer = WorldConfig::get()->GetByName(var_name);
	return(m_returnBuffer.c_str());
}

bool EQW::LSConnected() {
	return(loginserverlist.Connected());
}

int EQW::CountZones() {
	return(zoneserver_list.GetZoneCount());
}

//returns an array of zone_refs (opaque)
std::vector<std::string> EQW::ListBootedZones() {
	std::vector<std::string> res;

	std::vector<uint32> zones;
	zoneserver_list.GetZoneIDList(zones);

	std::vector<uint32>::iterator cur, end;
	cur = zones.begin();
	end = zones.end();
	for(; cur != end; ++cur) {
		res.push_back(itoa(*cur));
	}

	return(res);
}

std::map<std::string,std::string> EQW::GetZoneDetails(Const_char *zone_ref) {
	std::map<std::string,std::string> res;

	ZoneServer *zs = zoneserver_list.FindByID(atoi(zone_ref));
	if(zs == nullptr) {
		res["error"] = "Invalid zone.";
		return(res);
	}

	res["type"] = zs->IsStaticZone()?"static":"dynamic";
	res["zone_id"] = itoa(zs->GetZoneID());
	res["launch_name"] = zs->GetLaunchName();
	res["launched_name"] = zs->GetLaunchedName();
	res["short_name"] = zs->GetZoneName();
	res["long_name"] = zs->GetZoneLongName();
	res["port"] = itoa(zs->GetCPort());
	res["player_count"] = itoa(zs->NumPlayers());

	//this isnt gunna work for dynamic zones...
	res["launcher"] = "";
	if(zs->GetZoneID() != 0) {
		LauncherLink *ll = launcher_list.FindByZone(zs->GetLaunchName());
		if(ll != nullptr)
			res["launcher"] = ll->GetName();
	}

	return(res);
}

int EQW::CountPlayers() {
	return(client_list.GetClientCount());
}

//returns an array of character names in the zone (empty=all zones)
std::vector<std::string> EQW::ListPlayers(Const_char *zone_name) {
	std::vector<std::string> res;

	std::vector<ClientListEntry *> list;
	client_list.GetClients(zone_name, list);

	std::vector<ClientListEntry *>::iterator cur, end;
	cur = list.begin();
	end = list.end();
	for(; cur != end; ++cur) {
		res.push_back((*cur)->name());
	}
	return(res);
}

std::map<std::string,std::string> EQW::GetPlayerDetails(Const_char *char_name) {
	std::map<std::string,std::string> res;

	ClientListEntry *cle = client_list.FindCharacter(char_name);
	if(cle == nullptr) {
		res["error"] = "1";
		return(res);
	}

	res["character"] = cle->name();
	res["account"] = cle->AccountName();
	res["account_id"] = itoa(cle->AccountID());
	res["location_short"] = cle->zone()?ZoneName(cle->zone()):"No Zone";
	res["location_long"] = res["location_short"];
	res["location_id"] = itoa(cle->zone());
	res["ip"] = long2ip(cle->GetIP());
	res["level"] = itoa(cle->level());
	res["race"] = GetRaceIDName(cle->race());
	res["race_id"] = itoa(cle->race());
	res["class"] = GetClassIDName(cle->class_());
	res["class_id"] = itoa(cle->class_());
	res["guild_id"] = itoa(cle->GuildID());
	res["guild"] = guild_mgr.GetGuildName(cle->GuildID());
	res["status"] = itoa(cle->Admin());
//	res["patch"] = cle->DescribePatch();

	return(res);
}

int EQW::CountLaunchers(bool active_only) {
	if(active_only)
		return(launcher_list.GetLauncherCount());

	std::vector<std::string> it(EQW::ListLaunchers());
	return(it.size());
}
/*
vector<string> EQW::ListActiveLaunchers() {
	vector<string> launchers;
	launcher_list.GetLauncherNameList(launchers);
	return(launchers);
}*/

std::vector<std::string> EQW::ListLaunchers() {
//	vector<string> list;
//	database.GetLauncherList(list);
	std::vector<std::string> launchers;
	launcher_list.GetLauncherNameList(launchers);
	return(launchers);

/*	if(list.empty()) {
		return(launchers);
	} else if(launchers.empty()) {
		return(list);
	}

	//union the two lists.
	vector<string>::iterator curo, endo, curi, endi;
	curo = list.begin();
	endo = list.end();
	for(; curo != endo; curo++) {
		bool found = false;
		curi = launchers.begin();
		endi = launchers.end();
		for(; curi != endi; curi++) {
			if(*curo == *curi) {
				found = true;
				break;
			}
		}
		if(found)
			break;
		launchers.push_back(*curo);
	}
	return(launchers);*/
}

EQLConfig * EQW::GetLauncher(Const_char *launcher_name) {
	return(launcher_list.GetConfig(launcher_name));
}

void EQW::CreateLauncher(Const_char *launcher_name, int dynamic_count) {
	launcher_list.CreateLauncher(launcher_name, dynamic_count);
}

uint32 EQW::CreateGuild(const char* name, uint32 leader_char_id) {
	uint32 id = guild_mgr.CreateGuild(name, leader_char_id);
	if(id != GUILD_NONE)
		client_list.UpdateClientGuild(leader_char_id, id);
	return(id);
}

bool EQW::DeleteGuild(uint32 guild_id) {
	return(guild_mgr.DeleteGuild(guild_id));
}

bool EQW::RenameGuild(uint32 guild_id, const char* name) {
	return(guild_mgr.RenameGuild(guild_id, name));
}

bool EQW::SetGuildMOTD(uint32 guild_id, const char* motd, const char *setter) {
	return(guild_mgr.SetGuildMOTD(guild_id, motd, setter));
}

bool EQW::SetGuildLeader(uint32 guild_id, uint32 leader_char_id) {
	return(guild_mgr.SetGuildLeader(guild_id, leader_char_id));
}

bool EQW::SetGuild(uint32 charid, uint32 guild_id, uint8 rank) {
	client_list.UpdateClientGuild(charid, guild_id);
	return(guild_mgr.SetGuild(charid, guild_id, rank));
}

bool EQW::SetGuildRank(uint32 charid, uint8 rank) {
	return(guild_mgr.SetGuildRank(charid, rank));
}

bool EQW::SetBankerFlag(uint32 charid, bool is_banker) {
	return(guild_mgr.SetBankerFlag(charid, is_banker));
}

bool EQW::SetTributeFlag(uint32 charid, bool enabled) {
	return(guild_mgr.SetTributeFlag(charid, enabled));
}

bool EQW::SetPublicNote(uint32 charid, const char *note) {
	return(guild_mgr.SetPublicNote(charid, note));
}

int EQW::CountBugs() {
	std::string query = "SELECT count(*) FROM bugs where status = 0";
	auto results = database.QueryDatabase(query);
	if (!results.Success())
        return 0;

    if (results.RowCount() == 0)
        return 0;

    auto row = results.begin();
    return atoi(row[0]);
}

std::vector<std::string> EQW::ListBugs(uint32 offset) {
	std::vector<std::string> res;
	std::string query = StringFormat("SELECT id FROM bugs WHERE status = 0 limit %d, 30", offset);
	auto results = database.QueryDatabase(query);

    if (!results.Success())
        return res;

    for (auto row = results.begin();row != results.end(); ++row)
        res.push_back(row[0]);

	return res;
}

std::map<std::string,std::string> EQW::GetBugDetails(Const_char *id) {
	std::map<std::string,std::string> res;
	std::string query = StringFormat("SELECT name, zone, x, y, z, target, bug FROM bugs WHERE id = %s", id);
	auto results = database.QueryDatabase(query);

	if (!results.Success())
        return res;

    for(auto row = results.begin(); row != results.end(); ++row) {
        res["name"] = row[0];
        res["zone"] = row[1];
        res["x"] = row[2];
        res["y"] = row[3];
        res["z"] = row[4];
        res["target"] = row[5];
        res["bug"] = row[6];
        res["id"] = id;
    }
	return res;
}

void EQW::ResolveBug(const char *id) {
	std::vector<std::string> res;
	std::string query = StringFormat("UPDATE bugs SET status=1 WHERE id=%s", id);
	database.QueryDatabase(query);
}

void EQW::SendMessage(uint32 type, const char *msg) {
    zoneserver_list.SendEmoteMessage(0, 0, 0, type, msg);
}

void EQW::WorldShutDown(uint32 time, uint32 interval) {
	zoneserver_list.WorldShutDown(time, interval);
}

#endif //EMBPERL

