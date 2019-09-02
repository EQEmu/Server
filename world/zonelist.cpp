/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
#include "zonelist.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "world_config.h"
#include "../common/misc_functions.h"
#include "../common/servertalk.h"
#include "../common/string_util.h"
#include "../common/random.h"
#include "../common/json/json.h"
#include "../common/event_sub.h"
#include "web_interface.h"

extern uint32 numzones;
extern bool holdzones;
extern EQEmu::Random emu_random;
extern WebInterfaceList web_interface;
volatile bool UCSServerAvailable_ = false;
void CatchSignal(int sig_num);

ZSList::ZSList()
{
	NextID = 1;
	CurGroupID = 1;
	LastAllocatedPort = 0;
	memset(pLockedZones, 0, sizeof(pLockedZones));

	m_tick.reset(new EQ::Timer(5000, true, std::bind(&ZSList::OnTick, this, std::placeholders::_1)));
	m_keepalive.reset(new EQ::Timer(2500, true, std::bind(&ZSList::OnKeepAlive, this, std::placeholders::_1)));
}

ZSList::~ZSList() {
}

void ZSList::ShowUpTime(WorldTCPConnection* con, const char* adminname) {
	uint32 ms = Timer::GetCurrentTime();
	uint32 d = ms / 86400000;
	ms -= d * 86400000;
	uint32 h = ms / 3600000;
	ms -= h * 3600000;
	uint32 m = ms / 60000;
	ms -= m * 60000;
	uint32 s = ms / 1000;
	if (d)
		con->SendEmoteMessage(adminname, 0, 0, 0, "Worldserver Uptime: %02id %02ih %02im %02is", d, h, m, s);
	else if (h)
		con->SendEmoteMessage(adminname, 0, 0, 0, "Worldserver Uptime: %02ih %02im %02is", h, m, s);
	else
		con->SendEmoteMessage(adminname, 0, 0, 0, "Worldserver Uptime: %02im %02is", m, s);
}

void ZSList::Add(ZoneServer* zoneserver) {
	zone_server_list.push_back(std::unique_ptr<ZoneServer>(zoneserver));
	zoneserver->SendGroupIDs();
}

void ZSList::Remove(const std::string &uuid)
{
	auto iter = zone_server_list.begin();
	while (iter != zone_server_list.end()) {
		if ((*iter)->GetUUID().compare(uuid) == 0) {
			zone_server_list.erase(iter);
			return;
		}
		iter++;
	}
}

void ZSList::KillAll() {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		(*iterator)->Disconnect();
		iterator = zone_server_list.erase(iterator);
	}
}

void ZSList::Process() {

	if (shutdowntimer && shutdowntimer->Check()) {
		Log(Logs::Detail, Logs::World_Server, "Shutdown timer has expired. Telling all zones to shut down and exiting. (fake sigint)");
		auto pack2 = new ServerPacket;
		pack2->opcode = ServerOP_ShutdownAll;
		pack2->size = 0;
		SendPacket(pack2);
		safe_delete(pack2);
		Process();
		CatchSignal(2);
	}

	if (reminder && reminder->Check() && shutdowntimer) {
		SendEmoteMessage(0, 0, 0, 15, "<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now. World will shut down in %i minutes...", ((shutdowntimer->GetRemainingTime() / 1000) / 60));
	}
}

bool ZSList::SendPacket(ServerPacket* pack) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		(*iterator)->SendPacket(pack);
		iterator++;
	}
	return true;
}

bool ZSList::SendPacket(uint32 ZoneID, ServerPacket* pack) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetZoneID() == ZoneID) {
			ZoneServer* tmp = (*iterator).get();
			tmp->SendPacket(pack);
			return true;
		}
		iterator++;
	}
	return(false);
}

bool ZSList::SendPacket(uint32 ZoneID, uint16 instanceID, ServerPacket* pack) {
	if (instanceID != 0)
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetInstanceID() == instanceID) {
				ZoneServer* tmp = (*iterator).get();
				tmp->SendPacket(pack);
				return true;
			}
			iterator++;
		}
	}
	else
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == ZoneID
				&& (*iterator)->GetInstanceID() == 0) {
				ZoneServer* tmp = (*iterator).get();
				tmp->SendPacket(pack);
				return true;
			}
			iterator++;
		}
	}
	return(false);
}

ZoneServer* ZSList::FindByName(const char* zonename) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if (strcasecmp((*iterator)->GetZoneName(), zonename) == 0) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByID(uint32 ZoneID) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetID() == ZoneID) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByZoneID(uint32 ZoneID) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		ZoneServer* tmp = (*iterator).get();
		if (tmp->GetZoneID() == ZoneID && tmp->GetInstanceID() == 0) {
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByPort(uint16 port) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetCPort() == port) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

ZoneServer* ZSList::FindByInstanceID(uint32 InstanceID)
{
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if ((*iterator)->GetInstanceID() == InstanceID) {
			ZoneServer* tmp = (*iterator).get();
			return tmp;
		}
		iterator++;
	}
	return 0;
}

bool ZSList::SetLockedZone(uint16 iZoneID, bool iLock) {
	for (auto &zone : pLockedZones) {
		if (iLock) {
			if (zone == 0) {
				zone = iZoneID;
				return true;
			}
		}
		else {
			if (zone == iZoneID) {
				zone = 0;
				return true;
			}
		}
	}
	return false;
}

bool ZSList::IsZoneLocked(uint16 iZoneID) {
	for (auto &zone : pLockedZones) {
		if (zone == iZoneID)
			return true;
	}
	return false;
}

void ZSList::ListLockedZones(const char* to, WorldTCPConnection* connection) {
	int x = 0;
	for (auto &zone : pLockedZones) {
		if (zone) {
			connection->SendEmoteMessageRaw(to, 0, 0, 0, database.GetZoneName(zone, true));
			x++;
		}
	}
	connection->SendEmoteMessage(to, 0, 0, 0, "%i zones locked.", x);
}

void ZSList::SendZoneStatus(const char* to, int16 admin, WorldTCPConnection* connection) {
	char locked[4];
	if (WorldConfig::get()->Locked == true) {
		strcpy(locked, "Yes");
	}
	else {
		strcpy(locked, "No");
	}

	char* output = 0;
	uint32 outsize = 0, outlen = 0;

	if (connection->IsConsole()) {
		AppendAnyLenString(&output, &outsize, &outlen, "World Locked: %s\r\n", locked);
	}
	else {
		AppendAnyLenString(&output, &outsize, &outlen, "World Locked: %s^", locked);
	}
	if (connection->IsConsole()) {
		AppendAnyLenString(&output, &outsize, &outlen, "Zoneservers online:\r\n");
	}
	else {
		AppendAnyLenString(&output, &outsize, &outlen, "Zoneservers online:^");
	}

	int v = 0, w = 0, x = 0, y = 0, z = 0;
	char is_static_string[2] = { 0, 0 }, zone_data_string[64];
	memset(zone_data_string, 0, sizeof(zone_data_string));

	ZoneServer* zone_server_data = 0;

	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		zone_server_data = (*iterator).get();
		auto addr = zone_server_data->GetIP();

		if (zone_server_data->IsStaticZone()) {
			z++;
		}
		else if (zone_server_data->GetZoneID() != 0) {
			w++;
		}
		else if (zone_server_data->GetZoneID() == 0 && !zone_server_data->IsBootingUp()) {
			v++;
		}

		if (zone_server_data->IsStaticZone())
			is_static_string[0] = 'S';
		else
			is_static_string[0] = 'D';

		if (admin >= 150) {
			if (zone_server_data->GetZoneID()) {
				snprintf(zone_data_string, sizeof(zone_data_string), "%s (%i)", zone_server_data->GetZoneName(), zone_server_data->GetZoneID());
			}
			else if (zone_server_data->IsBootingUp()) {
				strcpy(zone_data_string, "...");
			}
			else {
				zone_data_string[0] = 0;
			}

			AppendAnyLenString(&output, &outsize, &outlen,
				"#%-3i :: %s :: %15s:%-5i :: %2i :: %s:%i :: %s :: (%u)",
				zone_server_data->GetID(),
				is_static_string,
				addr.c_str(),
				zone_server_data->GetPort(),
				zone_server_data->NumPlayers(),
				zone_server_data->GetCAddress(),
				zone_server_data->GetCPort(),
				zone_data_string,
				zone_server_data->GetZoneOSProcessID()
				);

			if (outlen >= 3584) {
				connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
				safe_delete(output);
				outsize = 0;
				outlen = 0;
			}
			else {
				if (connection->IsConsole())
					AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
				else
					AppendAnyLenString(&output, &outsize, &outlen, "^");
			}
			x++;
		}
		else if (zone_server_data->GetZoneID() != 0) {
			if (zone_server_data->GetZoneID())
				strcpy(zone_data_string, zone_server_data->GetZoneName());
			else
				zone_data_string[0] = 0;
			AppendAnyLenString(&output, &outsize, &outlen, "  #%i %s  %s", zone_server_data->GetID(), is_static_string, zone_data_string);
			if (outlen >= 3584) {
				connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
				safe_delete(output);
				outsize = 0;
				outlen = 0;
			}
			else {
				if (connection->IsConsole()) {
					AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
				}
				else {
					AppendAnyLenString(&output, &outsize, &outlen, "^");
				}
			}
			x++;
		}
		y++;
		iterator++;
	}

	if (connection->IsConsole()) {
		AppendAnyLenString(&output, &outsize, &outlen, "%i servers listed. %i servers online.\r\n", x, y);
	}
	else {
		AppendAnyLenString(&output, &outsize, &outlen, "%i servers listed. %i servers online.^", x, y);
	}

	AppendAnyLenString(&output, &outsize, &outlen, "%i zones are static zones, %i zones are booted zones, %i zones available.", z, w, v);

	if (output) {
		connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
	}

	safe_delete(output);
}

void ZSList::SendChannelMessage(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message, ...) {
	if (!message)
		return;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendChannelMessageRaw(from, to, chan_num, language, buffer);
}

void ZSList::SendChannelMessageRaw(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message) {
	if (!message)
		return;
	auto pack = new ServerPacket;

	pack->opcode = ServerOP_ChannelMessage;
	pack->size = sizeof(ServerChannelMessage_Struct) + strlen(message) + 1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*)pack->pBuffer;
	if (from == 0) {
		strcpy(scm->from, "WServer");
		scm->noreply = true;
	}
	else if (from[0] == 0) {
		strcpy(scm->from, "WServer");
		scm->noreply = true;
	}
	else
		strcpy(scm->from, from);
	if (to != 0) {
		strcpy((char *)scm->to, to);
		strcpy((char *)scm->deliverto, to);
	}
	else {
		scm->to[0] = 0;
		scm->deliverto[0] = 0;
	}

	scm->language = language;
	scm->chan_num = chan_num;
	strcpy(&scm->message[0], message);

	SendPacket(pack);
	delete pack;
}


void ZSList::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	if (!message)
		return;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendEmoteMessageRaw(to, to_guilddbid, to_minstatus, type, buffer);
}

void ZSList::SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message) {
	if (!message)
		return;
	auto pack = new ServerPacket;

	pack->opcode = ServerOP_EmoteMessage;
	pack->size = sizeof(ServerEmoteMessage_Struct) + strlen(message) + 1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*)pack->pBuffer;

	if (to) {
		strcpy((char *)sem->to, to);
	}
	else {
		sem->to[0] = 0;
	}

	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	sem->type = type;
	strcpy(&sem->message[0], message);
	char tempto[64] = { 0 };
	if (to)
		strn0cpy(tempto, to, 64);

	if (tempto[0] == 0) {
		SendPacket(pack);
	}
	else {
		ZoneServer* zs = FindByName(to);

		if (zs != 0)
			zs->SendPacket(pack);
		else
			SendPacket(pack);
	}
	delete pack;
}

void ZSList::SendTimeSync() {
	auto pack = new ServerPacket(ServerOP_SyncWorldTime, sizeof(eqTimeOfDay));
	eqTimeOfDay* tod = (eqTimeOfDay*)pack->pBuffer;
	tod->start_eqtime = worldclock.getStartEQTime();
	tod->start_realtime = worldclock.getStartRealTime();
	SendPacket(pack);
	delete pack;
}

void ZSList::NextGroupIDs(uint32 &start, uint32 &end) {
	start = CurGroupID;
	CurGroupID += 1000;	//hand them out 1000 at a time...
	if (CurGroupID < start) {	//handle overflow
		start = 1;
		CurGroupID = 1001;
	}
	end = CurGroupID - 1;
}

void ZSList::SOPZoneBootup(const char* adminname, uint32 ZoneServerID, const char* zonename, bool iMakeStatic) {
	ZoneServer* zs = 0;
	ZoneServer* zs2 = 0;
	uint32 zoneid;
	if (!(zoneid = database.GetZoneID(zonename)))
		SendEmoteMessage(adminname, 0, 0, 0, "Error: SOP_ZoneBootup: zone '%s' not found in 'zone' table. Typo protection=ON.", zonename);
	else {
		if (ZoneServerID != 0)
			zs = FindByID(ZoneServerID);
		else
			SendEmoteMessage(adminname, 0, 0, 0, "Error: SOP_ZoneBootup: ServerID must be specified");

		if (zs == 0)
			SendEmoteMessage(adminname, 0, 0, 0, "Error: SOP_ZoneBootup: zoneserver not found");
		else {
			zs2 = FindByName(zonename);
			if (zs2 != 0)
				SendEmoteMessage(adminname, 0, 0, 0, "Error: SOP_ZoneBootup: zone '%s' already being hosted by ZoneServer #%i", zonename, zs2->GetID());
			else {
				zs->TriggerBootup(zoneid, 0, adminname, iMakeStatic);
			}
		}
	}
}

void ZSList::RebootZone(const char* ip1, uint16 port, const char* ip2, uint32 skipid, uint32 zoneid) {
	// get random zone
	uint32 x = 0;
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		x++;
		iterator++;
	}
	if (x == 0)
		return;
	auto tmp = new ZoneServer *[x];
	uint32 y = 0;

	iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		if (!strcmp((*iterator)->GetCAddress(), ip2) && !(*iterator)->IsBootingUp() && (*iterator)->GetID() != skipid) {
			tmp[y++] = (*iterator).get();
		}
		iterator++;
	}
	if (y == 0) {
		safe_delete_array(tmp);
		return;
	}
	uint32 z = emu_random.Int(0, y - 1);

	auto pack = new ServerPacket(ServerOP_ZoneReboot, sizeof(ServerZoneReboot_Struct));
	ServerZoneReboot_Struct* s = (ServerZoneReboot_Struct*)pack->pBuffer;
	//	strcpy(s->ip1,ip1);
	strcpy(s->ip2, ip2);
	s->port = port;
	s->zoneid = zoneid;
	if (zoneid != 0)
		Log(Logs::Detail, Logs::World_Server, "Rebooting static zone with the ID of: %i", zoneid);
	tmp[z]->SendPacket(pack);
	delete pack;
	safe_delete_array(tmp);
}

uint16	ZSList::GetAvailableZonePort()
{
	const WorldConfig *Config = WorldConfig::get();
	int i;
	uint16 port = 0;

	if (LastAllocatedPort == 0)
		i = Config->ZonePortLow;
	else
		i = LastAllocatedPort + 1;

	while (i != LastAllocatedPort && port == 0) {
		if (i>Config->ZonePortHigh)
			i = Config->ZonePortLow;

		if (!FindByPort(i)) {
			port = i;
			break;
		}
		i++;
	}
	LastAllocatedPort = port;

	return port;
}

uint32 ZSList::TriggerBootup(uint32 iZoneID, uint32 iInstanceID) {
	if (iInstanceID > 0)
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetInstanceID() == iInstanceID)
			{
				return (*iterator)->GetID();
			}
			iterator++;
		}

		iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == 0 && !(*iterator)->IsBootingUp()) {
				ZoneServer* zone = (*iterator).get();
				zone->TriggerBootup(iZoneID, iInstanceID);
				return zone->GetID();
			}
			iterator++;
		}
		return 0;
	}
	else
	{
		auto iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == iZoneID && (*iterator)->GetInstanceID() == 0)
			{
				return (*iterator)->GetID();
			}
			iterator++;
		}

		iterator = zone_server_list.begin();
		while (iterator != zone_server_list.end()) {
			if ((*iterator)->GetZoneID() == 0 && !(*iterator)->IsBootingUp()) {
				ZoneServer* zone = (*iterator).get();
				zone->TriggerBootup(iZoneID);
				return zone->GetID();
			}
			iterator++;
		}
		return 0;
	}
}

void ZSList::SendLSZones() {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		ZoneServer* zs = (*iterator).get();
		zs->LSBootUpdate(zs->GetZoneID(), true);
		iterator++;
	}
}

int ZSList::GetZoneCount() {
	return(zone_server_list.size());
}

void ZSList::GetZoneIDList(std::vector<uint32> &zones) {
	auto iterator = zone_server_list.begin();
	while (iterator != zone_server_list.end()) {
		ZoneServer* zs = (*iterator).get();
		zones.push_back(zs->GetID());
		iterator++;
	}
}

void ZSList::UpdateUCSServerAvailable(bool ucss_available) {
	UCSServerAvailable_ = ucss_available;
	auto outapp = new ServerPacket(ServerOP_UCSServerStatusReply, sizeof(UCSServerStatus_Struct));
	auto ucsss = (UCSServerStatus_Struct*)outapp->pBuffer;
	ucsss->available = (ucss_available ? 1 : 0);
	ucsss->timestamp = Timer::GetCurrentTime();
	SendPacket(outapp);
	safe_delete(outapp);
}

void ZSList::WorldShutDown(uint32 time, uint32 interval)
{
	if (time > 0) {
		SendEmoteMessage(0, 0, 0, 15, "<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down in %i minutes, everyone log out before this time.", (time / 60));

		time *= 1000;
		interval *= 1000;
		if (interval < 5000) { interval = 5000; }

		shutdowntimer->SetTimer(time);
		reminder->SetTimer(interval - 1000);
		reminder->SetAtTrigger(interval);
		shutdowntimer->Start();
		reminder->Start();
	}
	else {
		SendEmoteMessage(0, 0, 0, 15, "<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now.");
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_ShutdownAll;
		pack->size = 0;
		SendPacket(pack);
		safe_delete(pack);
		Process();
		CatchSignal(2);
	}
}

void ZSList::DropClient(uint32 lsid, ZoneServer *ignore_zoneserver) {
	ServerPacket packet(ServerOP_DropClient, sizeof(ServerZoneDropClient_Struct));
	auto drop = (ServerZoneDropClient_Struct*)packet.pBuffer;
	drop->lsid = lsid;

	for (auto &zs : zone_server_list) {
		if (zs.get() != ignore_zoneserver) {
			zs->SendPacket(&packet);
		}
	}
}

void ZSList::OnTick(EQ::Timer *t)
{
	if (!EventSubscriptionWatcher::Get()->IsSubscribed("EQW::ZoneUpdate")) {
		return;
	}

	Json::Value out;
	out["event"] = "EQW::ZoneUpdate";
	out["data"] = Json::Value();

	for (auto &zone : zone_server_list)
	{
		Json::Value outzone;

		outzone["CAddress"] = zone->GetCAddress();
		outzone["CLocalAddress"] = zone->GetCLocalAddress();
		outzone["CompileTime"] = zone->GetCompileTime();
		outzone["CPort"] = zone->GetCPort();
		outzone["ID"] = zone->GetID();
		outzone["InstanceID"] = zone->GetInstanceID();
		outzone["IP"] = zone->GetIP();
		outzone["LaunchedName"] = zone->GetLaunchedName();
		outzone["LaunchName"] = zone->GetLaunchName();
		outzone["Port"] = zone->GetPort();
		outzone["PrevZoneID"] = zone->GetPrevZoneID();
		outzone["UUID"] = zone->GetUUID();
		outzone["ZoneID"] = zone->GetZoneID();
		outzone["ZoneLongName"] = zone->GetZoneLongName();
		outzone["ZoneName"] = zone->GetZoneName();
		outzone["ZoneOSProcessID"] = zone->GetZoneOSProcessID();
		outzone["NumPlayers"] = zone->NumPlayers();
		outzone["BootingUp"] = zone->IsBootingUp();
		outzone["StaticZone"] = zone->IsStaticZone();

		out["data"].append(outzone);
	}

	web_interface.SendEvent(out);
}

void ZSList::OnKeepAlive(EQ::Timer *t)
{
	for (auto &zone : zone_server_list) {
		zone->SendKeepAlive();
	}
}

const std::list<std::unique_ptr<ZoneServer>> &ZSList::getZoneServerList() const
{
	return zone_server_list;
}
