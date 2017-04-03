/*	EQEMu: Everquest Server Emulator
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
#include "eql_config.h"
#include "worlddb.h"
#include "launcher_link.h"
#include "launcher_list.h"
#include "../common/string_util.h"
#include <cstdlib>
#include <cstring>

extern LauncherList launcher_list;

EQLConfig::EQLConfig(const char *launcher_name)
: m_name(launcher_name)
{
	LoadSettings();
}

void EQLConfig::LoadSettings() {

	LauncherZone tmp;
	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';

    std::string query = StringFormat("SELECT dynamics FROM launcher WHERE name = '%s'", namebuf);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "EQLConfig::LoadSettings: %s", results.ErrorMessage().c_str());
	}
    else {
        auto row = results.begin();
        m_dynamics = atoi(row[0]);
    }

	query = StringFormat("SELECT zone, port FROM launcher_zones WHERE launcher = '%s'", namebuf);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
        Log(Logs::General, Logs::Error, "EQLConfig::LoadSettings: %s", results.ErrorMessage().c_str());
        return;
    }

    LauncherZone zs;
    for (auto row = results.begin(); row != results.end(); ++row) {
        zs.name = row[0];
		zs.port = atoi(row[1]);
		m_zones[zs.name] = zs;
    }

}

EQLConfig *EQLConfig::CreateLauncher(const char *name, uint8 dynamic_count) {

	char namebuf[128];
	database.DoEscapeString(namebuf, name, strlen(name)&0x3F);	//limit len to 64
	namebuf[127] = '\0';

    std::string query = StringFormat("INSERT INTO launcher (name, dynamics) VALUES('%s', %d)", namebuf, dynamic_count);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return nullptr;
	}

	return new EQLConfig(name);
}

void EQLConfig::GetZones(std::vector<LauncherZone> &result) {
	std::map<std::string, LauncherZone>::iterator cur, end;
	cur = m_zones.begin();
	end = m_zones.end();
	for(; cur != end; ++cur) {
		result.push_back(cur->second);
	}
}

std::vector<std::string> EQLConfig::ListZones() {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	std::vector<std::string> res;
	if(ll == nullptr) {
		//if the launcher isnt connected, use the list from the database.
		std::map<std::string, LauncherZone>::iterator cur, end;
		cur = m_zones.begin();
		end = m_zones.end();
		for(; cur != end; ++cur) {
			res.push_back(cur->first);
		}
	} else {
		//otherwise, use the zone list from the launcher link.
		ll->GetZoneList(res);
	}
	return(res);
}

void EQLConfig::DeleteLauncher() {

	launcher_list.Remove(m_name.c_str());

	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';

    std::string query = StringFormat("DELETE FROM launcher WHERE name = '%s'", namebuf);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

    query = StringFormat("DELETE FROM launcher_zones WHERE launcher = '%s'", namebuf);
    results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}
}

bool EQLConfig::IsConnected() const {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	return(ll != nullptr);
}

void EQLConfig::RestartZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->RestartZone(zone_ref);
}

void EQLConfig::StopZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->StopZone(zone_ref);
}

void EQLConfig::StartZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->StartZone(zone_ref);
}

bool EQLConfig::BootStaticZone(Const_char *short_name, uint16 port) {
	//make sure the short name is valid.
	if(database.GetZoneID(short_name) == 0)
		return false;

	//database update
	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';
	char zonebuf[32];
	database.DoEscapeString(zonebuf, short_name, strlen(short_name)&0xF);	//limit len to 16
	zonebuf[31] = '\0';

    std::string query = StringFormat("INSERT INTO launcher_zones (launcher, zone, port) "
                                    "VALUES('%s', '%s', %d)", namebuf, zonebuf, port);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	//update our internal state.
	LauncherZone lz;
	lz.name = short_name;
	lz.port = port;
	m_zones[lz.name] = lz;

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->BootZone(short_name, port);
	}

	return true;
}

bool EQLConfig::ChangeStaticZone(Const_char *short_name, uint16 port) {
	//make sure the short name is valid.
	if(database.GetZoneID(short_name) == 0)
		return false;

	//check internal state
	std::map<std::string, LauncherZone>::iterator res;
	res = m_zones.find(short_name);
	if(res == m_zones.end()) {
		//not found.
		Log(Logs::General, Logs::Error, "Update for unknown zone %s", short_name);
		return false;
	}

	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';
	char zonebuf[32];
	database.DoEscapeString(zonebuf, short_name, strlen(short_name)&0xF);	//limit len to 16
	zonebuf[31] = '\0';

    std::string query = StringFormat("UPDATE launcher_zones SET port=%d WHERE "
                                    "launcher = '%s' AND zone = '%s'",port, namebuf, zonebuf);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	//update internal state
	res->second.port = port;

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->RestartZone(short_name);
	}

	return true;
}

bool EQLConfig::DeleteStaticZone(Const_char *short_name) {
	//check internal state
	std::map<std::string, LauncherZone>::iterator res;
	res = m_zones.find(short_name);
	if(res == m_zones.end()) {
		//not found.
		Log(Logs::General, Logs::Error, "Update for unknown zone %s", short_name);
		return false;
	}

	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';
	char zonebuf[32];
	database.DoEscapeString(zonebuf, short_name, strlen(short_name)&0xF);	//limit len to 16
	zonebuf[31] = '\0';

    std::string query = StringFormat("DELETE FROM launcher_zones WHERE "
                                    "launcher = '%s' AND zone = '%s'", namebuf, zonebuf);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	//internal update.
	m_zones.erase(res);

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->StopZone(short_name);
	}

	return true;
}

bool EQLConfig::SetDynamicCount(int count) {

	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';

    std::string query = StringFormat("UPDATE launcher SET dynamics=%d WHERE name='%s'", count, namebuf);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	//update in-memory version.
	m_dynamics = count;

	//if the launcher is connected, update it.
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll != nullptr) {
		ll->BootDynamics(count);
	}

	return false;
}

int EQLConfig::GetDynamicCount() const {
	return(m_dynamics);
}

std::map<std::string,std::string> EQLConfig::GetZoneDetails(Const_char *zone_ref) {
	std::map<std::string,std::string> res;

	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr) {
		res["name"] = zone_ref;
		res["up"] = "0";
		res["starts"] = "0";
		res["port"] = "0";
	} else {
		ll->GetZoneDetails(zone_ref, res);
	}

	return(res);
}

