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
#include "world_tcp_connection.h"
#include "worlddb.h"
#include "console.h"
#include "world_config.h"
#include "../common/servertalk.h"
#include "../common/string_util.h"
#include "../common/random.h"

extern uint32			numzones;
extern bool holdzones;
extern ConsoleList		console_list;
extern EQEmu::Random emu_random;
void CatchSignal(int sig_num);

ZSList::ZSList()
{
	NextID = 1;
	CurGroupID = 1;
	LastAllocatedPort=0;
	memset(pLockedZones, 0, sizeof(pLockedZones));
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
	list.Insert(zoneserver);
	zoneserver->SendGroupIDs();	//send its initial set of group ids
}

void ZSList::KillAll() {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		iterator.GetData()->Disconnect();
		iterator.RemoveCurrent();
		numzones--;
	}
}

void ZSList::Process() {

	if(shutdowntimer && shutdowntimer->Check()){
		Log.Out(Logs::Detail, Logs::World_Server, "Shutdown timer has expired. Telling all zones to shut down and exiting. (fake sigint)");
		auto pack2 = new ServerPacket;
		pack2->opcode = ServerOP_ShutdownAll;
		pack2->size=0;
		SendPacket(pack2);
		safe_delete(pack2);
		Process();
		CatchSignal(2);
	}
	if(reminder && reminder->Check()){
		SendEmoteMessage(0,0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now. World will shut down in %i minutes...", ((shutdowntimer->GetRemainingTime()/1000) / 60));
	}
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!iterator.GetData()->Process()) {
			ZoneServer* zs = iterator.GetData();
			struct in_addr in;
			in.s_addr = zs->GetIP();
			Log.Out(Logs::Detail, Logs::World_Server,"Removing zoneserver #%d at %s:%d",zs->GetID(),zs->GetCAddress(),zs->GetCPort());
			zs->LSShutDownUpdate(zs->GetZoneID());
			if (holdzones){
				Log.Out(Logs::Detail, Logs::World_Server,"Hold Zones mode is ON - rebooting lost zone");
				if(!zs->IsStaticZone())
					RebootZone(inet_ntoa(in),zs->GetCPort(),zs->GetCAddress(),zs->GetID());
				else
					RebootZone(inet_ntoa(in),zs->GetCPort(),zs->GetCAddress(),zs->GetID(),database.GetZoneID(zs->GetZoneName()));
			}

			iterator.RemoveCurrent();
			numzones--;
		}
		else {
			iterator.Advance();
		}
	}
}

bool ZSList::SendPacket(ServerPacket* pack) {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		iterator.GetData()->SendPacket(pack);
		iterator.Advance();
	}
	return true;
}

bool ZSList::SendPacket(uint32 ZoneID, ServerPacket* pack) {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetZoneID() == ZoneID) {
			ZoneServer* tmp = iterator.GetData();
			return(tmp->SendPacket(pack));
		}
		iterator.Advance();
	}
	return(false);
}

bool ZSList::SendPacket(uint32 ZoneID, uint16 instanceID, ServerPacket* pack) {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	if(instanceID != 0)
	{
		while(iterator.MoreElements()) {
			if(iterator.GetData()->GetInstanceID() == instanceID) {
				ZoneServer* tmp = iterator.GetData();
				return(tmp->SendPacket(pack));
			}
			iterator.Advance();
		}
	}
	else
	{
		while(iterator.MoreElements()) {
			if (iterator.GetData()->GetZoneID() == ZoneID
				&& iterator.GetData()->GetInstanceID() == 0) {
				ZoneServer* tmp = iterator.GetData();
				return(tmp->SendPacket(pack));
			}
			iterator.Advance();
		}
	}
	return(false);
}

ZoneServer* ZSList::FindByName(const char* zonename) {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (strcasecmp(iterator.GetData()->GetZoneName(), zonename) == 0) {
			ZoneServer* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

ZoneServer* ZSList::FindByID(uint32 ZoneID) {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == ZoneID) {
			ZoneServer* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

ZoneServer* ZSList::FindByZoneID(uint32 ZoneID) {
	LinkedListIterator<ZoneServer*> iterator(list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZoneServer* tmp = iterator.GetData();
		if (tmp->GetZoneID() == ZoneID && tmp->GetInstanceID() == 0) {
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

ZoneServer* ZSList::FindByPort(uint16 port) {
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetCPort() == port) {
			ZoneServer* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

ZoneServer* ZSList::FindByInstanceID(uint32 InstanceID)
{
	LinkedListIterator<ZoneServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetInstanceID() == InstanceID) {
			ZoneServer* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
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

	LinkedListIterator<ZoneServer*> iterator(list);
	struct in_addr in;

	iterator.Reset();
	char locked[4];
	if (WorldConfig::get()->Locked == true){
		strcpy(locked, "Yes");
	}
	else {
		strcpy(locked, "No");
	}

	char* output = 0;
	uint32 outsize = 0, outlen = 0;

	if (connection->IsConsole()){
		AppendAnyLenString(&output, &outsize, &outlen, "World Locked: %s\r\n", locked);
	}
	else{
		AppendAnyLenString(&output, &outsize, &outlen, "World Locked: %s^", locked);
	}
	if (connection->IsConsole()){
		AppendAnyLenString(&output, &outsize, &outlen, "Zoneservers online:\r\n");
	}
	else{
		AppendAnyLenString(&output, &outsize, &outlen, "Zoneservers online:^");
	}

	int v = 0, w = 0, x = 0, y = 0, z = 0;
	char is_static_string[2] = { 0, 0 }, zone_data_string[64];
	memset(zone_data_string, 0, sizeof(zone_data_string));

	ZoneServer* zone_server_data = 0;

	while (iterator.MoreElements()) {
		zone_server_data = iterator.GetData();
		in.s_addr = zone_server_data->GetIP();

		if (zone_server_data->IsStaticZone()){
			z++;
		}
		else if (zone_server_data->GetZoneID() != 0){
			w++;
		}
		else if (zone_server_data->GetZoneID() == 0 && !zone_server_data->IsBootingUp()){
			v++;
		}

		if (zone_server_data->IsStaticZone())
			is_static_string[0] = 'S';
		else
			is_static_string[0] = 'D';

		if (admin >= 150) {
			if (zone_server_data->GetZoneID()){
				snprintf(zone_data_string, sizeof(zone_data_string), "%s (%i)", zone_server_data->GetZoneName(), zone_server_data->GetZoneID());
			}
			else if (zone_server_data->IsBootingUp()){
				strcpy(zone_data_string, "...");
			}
			else{
				zone_data_string[0] = 0;
			}

			AppendAnyLenString(&output, &outsize, &outlen, 
				"#%-3i :: %s :: %15s:%-5i :: %2i :: %s:%i :: %s :: (%u)", 
				zone_server_data->GetID(), 
				is_static_string, 
				inet_ntoa(in), 
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
				if (connection->IsConsole()){
					AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
				}
				else{
					AppendAnyLenString(&output, &outsize, &outlen, "^");
				}
			}
			x++;
		}
		y++;
		iterator.Advance();
	}

	if (connection->IsConsole()){
		AppendAnyLenString(&output, &outsize, &outlen, "%i servers listed. %i servers online.\r\n", x, y);
	}
	else {
		AppendAnyLenString(&output, &outsize, &outlen, "%i servers listed. %i servers online.^", x, y);
	}

	AppendAnyLenString(&output, &outsize, &outlen, "%i zones are static zones, %i zones are booted zones, %i zones available.", z, w, v);

	if (output){
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
	pack->size = sizeof(ServerChannelMessage_Struct)+strlen(message)+1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*) pack->pBuffer;
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
		strcpy((char *) scm->to, to);
		strcpy((char *) scm->deliverto, to);
	}
	else {
		scm->to[0] = 0;
		scm->deliverto[0] = 0;
	}

	scm->language = language;
	scm->chan_num = chan_num;
	strcpy(&scm->message[0], message);
	if (scm->chan_num == 5 || scm->chan_num == 6 || scm->chan_num == 11) {
		console_list.SendChannelMessage(scm);
	}
	pack->Deflate();
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
	pack->size = sizeof(ServerEmoteMessage_Struct)+strlen(message)+1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*) pack->pBuffer;

	if (to) {
		if (to[0] == '*') {
			Console* con = console_list.FindByAccountName(&to[1]);
			if (con)
				con->SendEmoteMessageRaw(to, to_guilddbid, to_minstatus, type, message);
			delete pack;
			return;
		}
		strcpy((char *) sem->to, to);
	}
	else {
		sem->to[0] = 0;
	}

	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	sem->type = type;
	strcpy(&sem->message[0], message);
	char tempto[64]={0};
	if(to)
		strn0cpy(tempto,to,64);
	pack->Deflate();
	if (tempto[0] == 0) {
		SendPacket(pack);
		if (to_guilddbid == 0)
			console_list.SendEmoteMessageRaw(type, message);
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
	eqTimeOfDay* tod = (eqTimeOfDay*) pack->pBuffer;
	tod->start_eqtime=worldclock.getStartEQTime();
	tod->start_realtime=worldclock.getStartRealTime();
	SendPacket(pack);
	delete pack;
}

void ZSList::NextGroupIDs(uint32 &start, uint32 &end) {
	start = CurGroupID;
	CurGroupID += 1000;	//hand them out 1000 at a time...
	if(CurGroupID < start) {	//handle overflow
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

void ZSList::RebootZone(const char* ip1,uint16 port,const char* ip2, uint32 skipid, uint32 zoneid){
// get random zone
	LinkedListIterator<ZoneServer*> iterator(list);
	uint32 x = 0;
	iterator.Reset();
	while(iterator.MoreElements()) {
		x++;
		iterator.Advance();
	}
	if (x == 0)
		return;
	auto tmp = new ZoneServer *[x];
	uint32 y = 0;
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!strcmp(iterator.GetData()->GetCAddress(),ip2) && !iterator.GetData()->IsBootingUp() && iterator.GetData()->GetID() != skipid) {
			tmp[y++] = iterator.GetData();
		}
		iterator.Advance();
	}
	if (y == 0) {
		safe_delete_array(tmp);
		return;
	}
	uint32 z = emu_random.Int(0, y-1);

	auto pack = new ServerPacket(ServerOP_ZoneReboot, sizeof(ServerZoneReboot_Struct));
	ServerZoneReboot_Struct* s = (ServerZoneReboot_Struct*) pack->pBuffer;
//	strcpy(s->ip1,ip1);
	strcpy(s->ip2,ip2);
	s->port = port;
	s->zoneid = zoneid;
	if(zoneid != 0)
		Log.Out(Logs::Detail, Logs::World_Server,"Rebooting static zone with the ID of: %i",zoneid);
	tmp[z]->SendPacket(pack);
	delete pack;
	safe_delete_array(tmp);
}

uint16	ZSList::GetAvailableZonePort()
{
	const WorldConfig *Config=WorldConfig::get();
	int i;
	uint16 port=0;

	if (LastAllocatedPort==0)
		i=Config->ZonePortLow;
	else
		i=LastAllocatedPort+1;

	while(i!=LastAllocatedPort && port==0) {
		if (i>Config->ZonePortHigh)
			i=Config->ZonePortLow;

		if (!FindByPort(i)) {
			port=i;
			break;
		}
		i++;
	}
	LastAllocatedPort=port;

	return port;
}

uint32 ZSList::TriggerBootup(uint32 iZoneID, uint32 iInstanceID) {
	if(iInstanceID > 0)
	{
		LinkedListIterator<ZoneServer*> iterator(list);
		iterator.Reset();
		while(iterator.MoreElements()) {
			if(iterator.GetData()->GetInstanceID() == iInstanceID)
			{
				return iterator.GetData()->GetID();
			}
			iterator.Advance();
		}

		iterator.Reset();
		while(iterator.MoreElements()) {
			if (iterator.GetData()->GetZoneID() == 0 && !iterator.GetData()->IsBootingUp()) {
				ZoneServer* zone=iterator.GetData();
				zone->TriggerBootup(iZoneID, iInstanceID);
				return zone->GetID();
			}
			iterator.Advance();
		}
		return 0;
	}
	else
	{
		LinkedListIterator<ZoneServer*> iterator(list);
		iterator.Reset();
		while(iterator.MoreElements()) {
			if(iterator.GetData()->GetZoneID() == iZoneID && iterator.GetData()->GetInstanceID() == 0)
			{
				return iterator.GetData()->GetID();
			}
			iterator.Advance();
		}

		iterator.Reset();
		while(iterator.MoreElements()) {
			if (iterator.GetData()->GetZoneID() == 0 && !iterator.GetData()->IsBootingUp()) {
				ZoneServer* zone=iterator.GetData();
				zone->TriggerBootup(iZoneID);
				return zone->GetID();
			}
			iterator.Advance();
		}
		return 0;
	}
	/*Old Random boot zones use this if your server is distributed across computers.
	LinkedListIterator<ZoneServer*> iterator(list);

	srand(time(nullptr));
	uint32 x = 0;
	iterator.Reset();
	while(iterator.MoreElements()) {
		x++;
		iterator.Advance();
	}
	if (x == 0) {
		return 0;
	}

	ZoneServer** tmp = new ZoneServer*[x];
	uint32 y = 0;

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetZoneID() == 0 && !iterator.GetData()->IsBootingUp()) {
			tmp[y++] = iterator.GetData();
		}
		iterator.Advance();
	}
	if (y == 0) {
		safe_delete(tmp);
		return 0;
	}

	uint32 z = rand() % y;

	tmp[z]->TriggerBootup(iZoneID);
	uint32 ret = tmp[z]->GetID();
	safe_delete(tmp);
	return ret;
	*/
}

void ZSList::SendLSZones(){
	LinkedListIterator<ZoneServer*> iterator(list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		ZoneServer* zs = iterator.GetData();
		zs->LSBootUpdate(zs->GetZoneID(),true);
		iterator.Advance();
	}
}

int ZSList::GetZoneCount() {
	return(numzones);
}

void ZSList::GetZoneIDList(std::vector<uint32> &zones) {
	LinkedListIterator<ZoneServer*> iterator(list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		ZoneServer* zs = iterator.GetData();
		zones.push_back(zs->GetID());
		iterator.Advance();
	}
}

void ZSList::WorldShutDown(uint32 time, uint32 interval)
{
	if( time > 0 ) {
		SendEmoteMessage(0,0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down in %i minutes, everyone log out before this time.", (time / 60));

		time *= 1000;
		interval *= 1000;
		if(interval < 5000) { interval = 5000; }

		shutdowntimer->SetTimer(time);
		reminder->SetTimer(interval-1000);
		reminder->SetAtTrigger(interval);
		shutdowntimer->Start();
		reminder->Start();
	}
	else {
		SendEmoteMessage(0,0,0,15,"<SYSTEMWIDE MESSAGE>:SYSTEM MSG:World coming down, everyone log out now.");
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_ShutdownAll;
		pack->size=0;
		SendPacket(pack);
		safe_delete(pack);
		Process();
		CatchSignal(2);
	}
}
