/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


#include "../common/version.h"
#include "console.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "../common/packet_dump.h"
#include "../common/seperator.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_packet.h"
#include "login_server.h"
#include "login_server_list.h"
#include "../common/serverinfo.h"
#include "../common/md5.h"
#include "../common/opcodemgr.h"
#include "../common/rulesys.h"
#include "../common/ruletypes.h"
#include "../common/string_util.h"
#include "world_config.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "launcher_list.h"
#include "ucs.h"
#include "queryserv.h"

#ifdef _WINDOWS
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#endif

extern ZSList	zoneserver_list;
extern uint32	numzones;
extern LoginServerList loginserverlist;
extern ClientList client_list;
extern LauncherList launcher_list;
extern UCSConnection UCSLink;
extern QueryServConnection QSLink;
extern volatile bool	RunLoops;

ConsoleList console_list;
void CatchSignal(int sig_num);

Console::Console(EmuTCPConnection* itcpc)
:	WorldTCPConnection(),
	timeout_timer(RuleI(Console, SessionTimeOut)),
	prompt_timer(1)
{
	tcpc = itcpc;
	tcpc->SetEcho(true);
	state = 0;
	paccountid = 0;
	memset(paccountname, 0, sizeof(paccountname));
	admin = 0;
	pAcceptMessages = false;
}

Console::~Console() {
	if (tcpc)
		tcpc->Free();
}

void Console::Die() {
	state = CONSOLE_STATE_CLOSED;
	struct in_addr in;
	in.s_addr = GetIP();
	LogInfo("Removing console from [{}]:[{}]",inet_ntoa(in),GetPort());
	tcpc->Disconnect();
}

bool Console::SendChannelMessage(const ServerChannelMessage_Struct* scm) {
	if (!pAcceptMessages)
		return false;
	switch (scm->chan_num) {
		if(RuleB(Chat, ServerWideAuction)){
			case 4: {
				SendMessage(1, "%s auctions, '%s'", scm->from, scm->message);
				break;
			}
		}
		if(RuleB(Chat, ServerWideOOC)){
			case 5: {
				SendMessage(1, "%s says ooc, '%s'", scm->from, scm->message);
				break;
			}
		}
		case 6: {
			SendMessage(1, "%s BROADCASTS, '%s'", scm->from, scm->message);
			break;
		}
		case 7: {
			SendMessage(1, "[%s] tells you, '%s'", scm->from, scm->message);
			auto pack = new ServerPacket(ServerOP_ChannelMessage,
						     sizeof(ServerChannelMessage_Struct) + strlen(scm->message) + 1);
			memcpy(pack->pBuffer, scm, pack->size);
			ServerChannelMessage_Struct* scm2 = (ServerChannelMessage_Struct*) pack->pBuffer;
			strcpy(scm2->deliverto, scm2->from);
			scm2->noreply = true;
			client_list.SendPacket(scm->from, pack);
			safe_delete(pack);
			break;
		}
		case 11: {
			SendMessage(1, "%s GMSAYS, '%s'", scm->from, scm->message);
			break;
		}
		default: {
			return false;
		}
	}
	return true;
}

bool Console::SendEmoteMessage(uint32 type, const char* message, ...) {
	if (!message)
		return false;
	if (!pAcceptMessages)
		return false;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendMessage(1, message);
	return true;
}

bool Console::SendEmoteMessageRaw(uint32 type, const char* message) {
	if (!message)
		return false;
	if (!pAcceptMessages)
		return false;
	SendMessage(1, message);
	return true;
}

void Console::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	if (!message)
		return;
	if (to_guilddbid != 0 || to_minstatus > Admin())
		return;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendEmoteMessageRaw(to, to_guilddbid, to_minstatus, type, buffer);
}

void Console::SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message) {
	if (!message)
		return;
	if (to_guilddbid != 0 || to_minstatus > Admin())
		return;
	SendMessage(1, message);
}

void Console::SendMessage(uint8 newline, const char* message, ...) {
	if (!message)
		return;
	char* buffer = 0;
	uint32 bufsize = 1500;
	if (message)
		bufsize += strlen(message);
	buffer = new char[bufsize];
	memset(buffer, 0, bufsize);
	if (message != 0) {
		va_list argptr;

		va_start(argptr, message);
		vsnprintf(buffer, bufsize - 512, message, argptr);
		va_end(argptr);
	}

	if (newline) {
		char outbuf[3];
		outbuf[0] = 13;
		outbuf[1] = 10;
		outbuf[2] = 0;
		for (int i=0; i < newline; i++)
			strcat(buffer, outbuf);
	}
	tcpc->Send((uchar*) buffer, strlen(buffer));
	safe_delete_array(buffer);
}

bool Console::Process() {
	if (state == CONSOLE_STATE_CLOSED)
		return false;

	if (!tcpc->Connected()) {
		struct in_addr in;
		in.s_addr = GetIP();
		LogInfo("Removing console (!tcpc->Connected) from [{}]:[{}]",inet_ntoa(in),GetPort());
		return false;
	}
	//if we have not gotten the special markers after this timer, send login prompt
	if(prompt_timer.Check()) {
		struct in_addr in;
		in.s_addr = GetIP();

		std::string connecting_ip = inet_ntoa(in);

		SendMessage(2, StringFormat("Establishing connection from IP: %s Port: %d", inet_ntoa(in), GetPort()).c_str());

		if (connecting_ip.find("127.0.0.1") != std::string::npos) {
			SendMessage(2, StringFormat("Connecting established from local host, auto assuming admin").c_str());
			state = CONSOLE_STATE_CONNECTED;
			tcpc->SetEcho(false);
			admin = 255;
			SendPrompt();
		}
		else {
			if (tcpc->GetMode() == EmuTCPConnection::modeConsole)
				tcpc->Send((const uchar*) "Username: ", strlen("Username: "));
		}

		prompt_timer.Disable();

	}

	if (timeout_timer.Check()) {
		SendMessage(1, 0);
		SendMessage(1, "Timeout, disconnecting...");
		struct in_addr in;
		in.s_addr = GetIP();
		LogInfo("TCP connection timeout from [{}]:[{}]",inet_ntoa(in),GetPort());
		return false;
	}

	if (tcpc->GetMode() == EmuTCPConnection::modePacket) {
		struct in_addr	in;
		in.s_addr = GetIP();
		if(tcpc->GetPacketMode() == EmuTCPConnection::packetModeZone) {
			auto zs = new ZoneServer(tcpc);
			LogInfo("New zoneserver #[{}] from [{}]:[{}]", zs->GetID(), inet_ntoa(in), GetPort());
			zoneserver_list.Add(zs);
			numzones++;
			tcpc = 0;
		} else if(tcpc->GetPacketMode() == EmuTCPConnection::packetModeLauncher) {
			LogInfo("New launcher from [{}]:[{}]", inet_ntoa(in), GetPort());
			launcher_list.Add(tcpc);
			tcpc = 0;
		}
		else if(tcpc->GetPacketMode() == EmuTCPConnection::packetModeUCS)
		{
			LogInfo("New UCS Connection from [{}]:[{}]", inet_ntoa(in), GetPort());
			UCSLink.SetConnection(tcpc);
			tcpc = 0;
		}
		else if(tcpc->GetPacketMode() == EmuTCPConnection::packetModeQueryServ)
		{
			LogInfo("New QS Connection from [{}]:[{}]", inet_ntoa(in), GetPort());
			QSLink.SetConnection(tcpc);
			tcpc = 0;
		}
		else {
			LogInfo("Unsupported packet mode from [{}]:[{}]", inet_ntoa(in), GetPort());
		}
		return false;
	}
	char* command = 0;
	while ((command = tcpc->PopLine())) {
		timeout_timer.Start();
		ProcessCommand(command);
		delete command;
	}
	return true;
}

void ConsoleList::Add(Console* con) {
	list.Insert(con);
}

void ConsoleList::Process() {
	LinkedListIterator<Console*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements()) {
		if (!iterator.GetData()->Process())
			iterator.RemoveCurrent();
		else
			iterator.Advance();
	}
}

void ConsoleList::KillAll() {
	LinkedListIterator<Console*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements()) {
		iterator.GetData()->Die();
		iterator.RemoveCurrent();
	}
}

void ConsoleList::SendConsoleWho(WorldTCPConnection* connection, const char* to, int16 admin, char** output, uint32* outsize, uint32* outlen) {
	LinkedListIterator<Console*> iterator(list);
	iterator.Reset();
	struct in_addr in;
	int x = 0;

	while(iterator.MoreElements()) {
		in.s_addr = iterator.GetData()->GetIP();
		if (admin >= iterator.GetData()->Admin())
			AppendAnyLenString(output, outsize, outlen, "  Console: %s:%i AccID: %i AccName: %s", inet_ntoa(in), iterator.GetData()->GetPort(), iterator.GetData()->AccountID(), iterator.GetData()->AccountName());
		else
			AppendAnyLenString(output, outsize, outlen, "  Console: AccID: %i AccName: %s", iterator.GetData()->AccountID(), iterator.GetData()->AccountName());
		if (*outlen >= 3584) {
			connection->SendEmoteMessageRaw(to, 0, 0, 10, *output);
			safe_delete(*output);
			*outsize = 0;
			*outlen = 0;
		}
		else {
			if (connection->IsConsole())
				AppendAnyLenString(output, outsize, outlen, "\r\n");
			else
				AppendAnyLenString(output, outsize, outlen, "\n");
		}
		x++;
		iterator.Advance();
	}
	AppendAnyLenString(output, outsize, outlen, "%i consoles connected", x);
}

void ConsoleList::SendChannelMessage(const ServerChannelMessage_Struct* scm) {
	LinkedListIterator<Console*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements()) {
		iterator.GetData()->SendChannelMessage(scm);
		iterator.Advance();
	}
}

void ConsoleList::SendEmoteMessage(uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);

	SendEmoteMessageRaw(type, buffer);
}

void ConsoleList::SendEmoteMessageRaw(uint32 type, const char* message) {
	LinkedListIterator<Console*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements()) {
		iterator.GetData()->SendEmoteMessageRaw(type, message);
		iterator.Advance();
	}
}

Console* ConsoleList::FindByAccountName(const char* accname) {
	LinkedListIterator<Console*> iterator(list);
	iterator.Reset();

	while(iterator.MoreElements()) {
		if (strcasecmp(iterator.GetData()->AccountName(), accname) == 0)
			return iterator.GetData();

		iterator.Advance();
	}
	return 0;
}

void Console::ProcessCommand(const char* command) {
	switch(state)
	{
		case CONSOLE_STATE_USERNAME:
		{
			if (strlen(command) >= 16) {
				SendMessage(1, 0);
				SendMessage(2, "Username buffer overflow.");
				SendMessage(1, "Bye Bye.");
				state = CONSOLE_STATE_CLOSED;
				return;
			}
			strcpy(paccountname, command);
			state = CONSOLE_STATE_PASSWORD;
			SendMessage(0, "Password: ");
			tcpc->SetEcho(false);
			break;
		}
		case CONSOLE_STATE_PASSWORD:
		{
			if (strlen(command) >= 16) {
				SendMessage(1, 0);
				SendMessage(2, "Password buffer overflow.");
				SendMessage(1, "Bye Bye.");
				state = CONSOLE_STATE_CLOSED;
				return;
			}
			paccountid = database.CheckLogin(paccountname ,command);
			if (paccountid == 0) {
				SendMessage(1, 0);
				SendMessage(2, "Login failed.");
				SendMessage(1, "Bye Bye.");
				state = CONSOLE_STATE_CLOSED;
				return;
			}
			database.GetAccountName(paccountid, paccountname); // fixes case and stuff
			admin = database.CheckStatus(paccountid);
			if (!(admin >= consoleLoginStatus)) {
				SendMessage(1, 0);
				SendMessage(2, "Access denied.");
				SendMessage(1, "Bye Bye.");
				state = CONSOLE_STATE_CLOSED;
				return;
			}
			LogInfo("TCP console authenticated: Username=[{}], Admin=[{}]",paccountname,admin);
			SendMessage(1, 0);
			SendMessage(2, "Login accepted.");
			state = CONSOLE_STATE_CONNECTED;
			tcpc->SetEcho(true);
			SendPrompt();
			break;
		}
		case CONSOLE_STATE_CONNECTED: {
			Log(Logs::Detail, Logs::World_Server,"TCP command: %s: \"%s\"",paccountname ,command);
			Seperator sep(command);
			if (strcasecmp(sep.arg[0], "help") == 0 || strcmp(sep.arg[0], "?") == 0) {
				SendMessage(1, "  whoami");
				SendMessage(1, "  who");
				SendMessage(1, "  zonestatus");
				SendMessage(1, "  uptime [zoneID#]");
				SendMessage(1, "  emote [zonename or charname or world] [type] [message]");
				SendMessage(1, "  echo [on/off]");
				SendMessage(1, "  acceptmessages [on/off]");
				SendMessage(1, "  tell [name] [message]");
				SendMessage(1, "  broadcast [message]");
				SendMessage(1, "  gmsay [message]");
				SendMessage(1, "  ooc [message]");
				SendMessage(1, "  auction [message]");
				if (admin >= consoleKickStatus)
					SendMessage(1, "  kick [charname]");
				if (admin >= consoleLockStatus)
					SendMessage(1, "  lock/unlock");
				if (admin >= consoleZoneStatus) {
					SendMessage(1, "  zoneshutdown [zonename or ZoneServerID]");
					SendMessage(1, "  zonebootup [ZoneServerID] [zonename]");
					SendMessage(1, "  zonelock [list|lock|unlock] [zonename]");
				}
				if (admin >= consoleFlagStatus)
					SendMessage(1, "  flag [status] [accountname]");
				if (admin >= consolePassStatus)
					SendMessage(1, "  setpass [accountname] [newpass]");
				if (admin >= consoleWorldStatus) {
					SendMessage(1, "  version");
					SendMessage(1, "  worldshutdown");
				}
				if (admin >= AccountStatus::GMMgmt) {
					SendMessage(1, "  IPLookup [name]");
				}
				if (admin >= AccountStatus::GMAdmin) {
					SendMessage(1, "  LSReconnect");
					SendMessage(1, "  signalcharbyname charname ID");
					SendMessage(1, "  reloadworld");
				}
			}
			else if (strcasecmp(sep.arg[0], "ping") == 0) {
				// do nothing
			}
			else if (strcasecmp(sep.arg[0], "signalcharbyname") == 0) {
				SendMessage(1, "Signal Sent to %s with ID %i", (char*) sep.arg[1], atoi(sep.arg[2]));
				uint32 message_len = strlen((char*) sep.arg[1]) + 1;
				auto pack = new ServerPacket(ServerOP_CZSignalClientByName,
							     sizeof(CZClientSignalByName_Struct) + message_len);
				CZClientSignalByName_Struct* CZSC = (CZClientSignalByName_Struct*) pack->pBuffer;
				strn0cpy(CZSC->Name, (char*) sep.arg[1], 64);
				CZSC->data = atoi(sep.arg[2]);
				zoneserver_list.SendPacket(pack);
				safe_delete(pack);
			}
			else if (strcasecmp(sep.arg[0], "setpass") == 0 && admin >= consolePassStatus) {
				if (sep.argnum != 2)
					SendMessage(1, "Format: setpass accountname password");
				else {

					int16 tmpstatus = 0;
					uint32 tmpid = database.GetAccountIDByName(sep.arg[1], &tmpstatus);
					if (!tmpid)
						SendMessage(1, "Error: Account not found");
					else if (tmpstatus > admin)
						SendMessage(1, "Cannot change password: Account's status is higher than yours");
					else if (database.SetLocalPassword(tmpid, sep.arg[2]))
						SendMessage(1, "Password changed.");
					else
						SendMessage(1, "Error changing password.");
				}
			}
			else if (strcasecmp(sep.arg[0], "uptime") == 0) {
				if (sep.IsNumber(1) && atoi(sep.arg[1]) > 0) {
					auto pack = new ServerPacket(ServerOP_Uptime, sizeof(ServerUptime_Struct));
					ServerUptime_Struct* sus = (ServerUptime_Struct*) pack->pBuffer;
					snprintf(sus->adminname, sizeof(sus->adminname), "*%s", GetName());
					sus->zoneserverid = atoi(sep.arg[1]);
					ZoneServer* zs = zoneserver_list.FindByID(sus->zoneserverid);
					if (zs)
						zs->SendPacket(pack);
					else
						SendMessage(1, "Zoneserver not found.");
					delete pack;
				}
				else {
					ZSList::ShowUpTime(this);
				}
			}
			else if (strcasecmp(sep.arg[0], "md5") == 0) {
				uint8 md5[16];
				MD5::Generate((const uchar*) sep.argplus[1], strlen(sep.argplus[1]), md5);
				SendMessage(1, "MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", md5[0], md5[1], md5[2], md5[3], md5[4], md5[5], md5[6], md5[7], md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);
			}
			else if (strcasecmp(sep.arg[0], "whoami") == 0) {
				SendMessage(1, "You are logged in as '%s'", AccountName());
				SendMessage(1, "You are known as '*%s'", AccountName());
				SendMessage(1, "AccessLevel: %d", Admin());
			}
			else if (strcasecmp(sep.arg[0], "echo") == 0) {
				if (strcasecmp(sep.arg[1], "on") == 0)
					tcpc->SetEcho(true);
				else if (strcasecmp(sep.arg[1], "off") == 0) {
					if (pAcceptMessages)
						SendMessage(1, "Echo can not be turned off while acceptmessages is on");
					else
						tcpc->SetEcho(false);
				}
				else
					SendMessage(1, "Usage: echo [on/off]");
			}
			else if (strcasecmp(sep.arg[0], "acceptmessages") == 0) {
				if (strcasecmp(sep.arg[1], "on") == 0)
					if (tcpc->GetEcho())
						SendMessage(1, "AcceptMessages can not be turned on while echo is on");
					else
						pAcceptMessages = true;
				else if (strcasecmp(sep.arg[1], "off") == 0)
					pAcceptMessages = false;
				else
					SendMessage(1, "Usage: acceptmessages [on/off]");
			}
			else if (strcasecmp(sep.arg[0], "tell") == 0) {
				char tmpname[64];
				tmpname[0] = '*';
				strcpy(&tmpname[1], paccountname);
				zoneserver_list.SendChannelMessage(tmpname, sep.arg[1], 7, 0, sep.argplus[2]);
			}
			else if (strcasecmp(sep.arg[0], "broadcast") == 0) {
				char tmpname[64];
				tmpname[0] = '*';
				strcpy(&tmpname[1], paccountname);
				zoneserver_list.SendChannelMessage(tmpname, 0, 6, 0, sep.argplus[1]);
			}
			else if (strcasecmp(sep.arg[0], "ooc") == 0) {
				char tmpname[64];
				tmpname[0] = '*';
				strcpy(&tmpname[1], paccountname);
				zoneserver_list.SendChannelMessage(tmpname, 0, 5, 0, sep.argplus[1]);
			}
			else if (strcasecmp(sep.arg[0], "auction") == 0) {
				char tmpname[64];
				tmpname[0] = '*';
				strcpy(&tmpname[1], paccountname);
				zoneserver_list.SendChannelMessage(tmpname, 0, 4, 0, sep.argplus[1]);
			}
			else if (strcasecmp(sep.arg[0], "gmsay") == 0 || strcasecmp(sep.arg[0], "pr") == 0) {
				char tmpname[64];
				tmpname[0] = '*';
				strcpy(&tmpname[1], paccountname);
				zoneserver_list.SendChannelMessage(tmpname, 0, 11, 0, sep.argplus[1]);
			}
			else if (strcasecmp(sep.arg[0], "emote") == 0) {
				if (strcasecmp(sep.arg[1], "world") == 0)
					zoneserver_list.SendEmoteMessageRaw(0, 0, 0, atoi(sep.arg[2]), sep.argplus[3]);
				else {
					ZoneServer* zs = zoneserver_list.FindByName(sep.arg[1]);
					if (zs != 0)
						zs->SendEmoteMessageRaw(0, 0, 0, atoi(sep.arg[2]), sep.argplus[3]);
					else
						zoneserver_list.SendEmoteMessageRaw(sep.arg[1], 0, 0, atoi(sep.arg[2]), sep.argplus[3]);
				}
			}
			else if (strcasecmp(sep.arg[0], "movechar") == 0) {
				if(sep.arg[1][0]==0 || sep.arg[2][0] == 0)
					SendMessage(1, "Usage: movechar [charactername] [zonename]");
				else {
					if (!ZoneID(sep.arg[2]))
						SendMessage(1, "Error: Zone '%s' not found", sep.arg[2]);
					else if (!database.CheckUsedName((char*) sep.arg[1])) {
						if (!database.MoveCharacterToZone((char*) sep.arg[1], (char*) sep.arg[2]))
							SendMessage(1, "Character Move Failed!");
						else
							SendMessage(1, "Character has been moved.");
					}
					else
						SendMessage(1, "Character Does Not Exist");
				}
			}
			else if (strcasecmp(sep.arg[0], "flag") == 0 && Admin() >= consoleFlagStatus) {
// SCORPIOUS2K - reversed parameter order for flag
				if(sep.arg[2][0]==0 || !sep.IsNumber(1))
					SendMessage(1, "Usage: flag [status] [accountname]");
				else
				{
					if (atoi(sep.arg[1]) > Admin())
						SendMessage(1, "You cannot set people's status to higher than your own");
					else if (atoi(sep.arg[1]) < 0 && Admin() < consoleFlagStatus)
							SendMessage(1, "You have too low of status to change flags");
					else if (!database.SetAccountStatus(sep.arg[2], atoi(sep.arg[1])))
							SendMessage(1, "Unable to flag account!");
					else
							SendMessage(1, "Account Flaged");
				}
			}
			else if (strcasecmp(sep.arg[0], "kick") == 0 && admin >= consoleKickStatus) {
				char tmpname[64];
				tmpname[0] = '*';
				strcpy(&tmpname[1], paccountname);
				auto pack = new ServerPacket;
				pack->opcode = ServerOP_KickPlayer;
				pack->size = sizeof(ServerKickPlayer_Struct);
				pack->pBuffer = new uchar[pack->size];
				ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*) pack->pBuffer;
				strcpy(skp->adminname, tmpname);
				strcpy(skp->name, sep.arg[1]);
				skp->adminrank = Admin();
				zoneserver_list.SendPacket(pack);
				delete pack;
			}
			else if (strcasecmp(sep.arg[0], "who") == 0) {
				auto whom = new Who_All_Struct;
				memset(whom, 0, sizeof(Who_All_Struct));
				whom->lvllow = 0xFFFF;
				whom->lvlhigh = 0xFFFF;
				whom->wclass = 0xFFFF;
				whom->wrace = 0xFFFF;
				whom->gmlookup = 0xFFFF;
				for (int i=1; i<=sep.argnum; i++) {
					if (strcasecmp(sep.arg[i], "gm") == 0)
						whom->gmlookup = 1;
					else if (sep.IsNumber(i)) {
						if (whom->lvllow == 0xFFFF) {
							whom->lvllow = atoi(sep.arg[i]);
							whom->lvlhigh = whom->lvllow;
						}
						else if (atoi(sep.arg[i]) > int(whom->lvllow))
							whom->lvlhigh = atoi(sep.arg[i]);
						else
							whom->lvllow = atoi(sep.arg[i]);
					}
					else
						strn0cpy(whom->whom, sep.arg[i], sizeof(whom->whom));
				}
				client_list.ConsoleSendWhoAll(0, admin, whom, this);
				delete whom;
			}
			else if (strcasecmp(sep.arg[0], "zonestatus") == 0) {
				zoneserver_list.SendZoneStatus(0, admin, this);
			}
			else if (strcasecmp(sep.arg[0], "exit") == 0 || strcasecmp(sep.arg[0], "quit") == 0) {
				SendMessage(1, "Bye Bye.");
				state = CONSOLE_STATE_CLOSED;
			}
			else if (strcasecmp(sep.arg[0], "zoneshutdown") == 0 && admin >= consoleZoneStatus) {
				if (sep.arg[1][0] == 0) {
					SendMessage(1, "Usage: zoneshutdown zoneshortname");
				} else {
					char tmpname[64];
					tmpname[0] = '*';
					strcpy(&tmpname[1], paccountname);

					auto pack = new ServerPacket;
					pack->size = sizeof(ServerZoneStateChange_struct);
					pack->pBuffer = new uchar[pack->size];
					memset(pack->pBuffer, 0, sizeof(ServerZoneStateChange_struct));
					ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
					pack->opcode = ServerOP_ZoneShutdown;
					strcpy(s->adminname, tmpname);
					if (sep.arg[1][0] >= '0' && sep.arg[1][0] <= '9')
						s->ZoneServerID = atoi(sep.arg[1]);
					else
						s->zoneid = ZoneID(sep.arg[1]);

					ZoneServer* zs = 0;
					if (s->ZoneServerID != 0)
						zs = zoneserver_list.FindByID(s->ZoneServerID);
					else if (s->zoneid != 0)
						zs = zoneserver_list.FindByName(ZoneName(s->zoneid));
					else
						SendMessage(1, "Error: ZoneShutdown: neither ID nor name specified");

					if (zs == 0)
						SendMessage(1, "Error: ZoneShutdown: zoneserver not found");
					else
						zs->SendPacket(pack);

					delete pack;
				}
			}
			else if (strcasecmp(sep.arg[0], "zonebootup") == 0 && admin >= consoleZoneStatus) {
				if (sep.arg[2][0] == 0 || !sep.IsNumber(1)) {
					SendMessage(1, "Usage: zonebootup ZoneServerID# zoneshortname");
				} else {
					char tmpname[64];
					tmpname[0] = '*';
					strcpy(&tmpname[1], paccountname);

					LogInfo("Console ZoneBootup: [{}], [{}], [{}]",tmpname,sep.arg[2],sep.arg[1]);
					zoneserver_list.SOPZoneBootup(tmpname, atoi(sep.arg[1]), sep.arg[2], (bool) (strcasecmp(sep.arg[3], "static") == 0));
				}
			}
			else if (strcasecmp(sep.arg[0], "worldshutdown") == 0 && admin >= consoleWorldStatus) {
				int32 time, interval;
				if(sep.IsNumber(1) && sep.IsNumber(2) && ((time=atoi(sep.arg[1]))>0) && ((interval=atoi(sep.arg[2]))>0)) {
					zoneserver_list.WorldShutDown(time, interval);
				}
				else if(strcasecmp(sep.arg[1], "now") == 0) {
					zoneserver_list.WorldShutDown(0, 0);
				}
				else if(strcasecmp(sep.arg[1], "disable") == 0) {
        			SendEmoteMessage(
						0,
						0,
						0,
						Chat::Yellow,
						"[SYSTEM] World shutdown has been aborted."
					);
					zoneserver_list.SendEmoteMessage(
						0,
						0,
						0,
						Chat::Yellow,
						"[SYSTEM] World shutdown has been aborted."
					);
        			zoneserver_list.shutdowntimer->Disable();
			        zoneserver_list.reminder->Disable();
				}
				else {
					SendMessage(1, "Usage: worldshutdown [now] [disable] ([time] [interval])");
					//Go ahead and shut down since that's what this used to do when invoked this way.
					zoneserver_list.WorldShutDown(0, 0);
				}
			}
			else if (strcasecmp(sep.arg[0], "lock") == 0 && admin >= consoleLockStatus) {
				WorldConfig::LockWorld();
				if (loginserverlist.Connected()) {
					loginserverlist.SendStatus();
					SendMessage(1, "World locked.");
				}
				else {
					SendMessage(1, "World locked, but login server not connected.");
				}
			}
			else if (strcasecmp(sep.arg[0], "unlock") == 0 && admin >= consoleLockStatus) {
				WorldConfig::UnlockWorld();
				if (loginserverlist.Connected()) {
					loginserverlist.SendStatus();
					SendMessage(1, "World unlocked.");
				}
				else {
					SendMessage(1, "World unlocked, but login server not connected.");
				}
			}
			else if (strcasecmp(sep.arg[0], "version") == 0 && admin >= consoleWorldStatus) {
				SendMessage(1, "Current version information.");
				SendMessage(1, "  %s", CURRENT_VERSION);
				SendMessage(1, "  Compiled on: %s at %s", COMPILE_DATE, COMPILE_TIME);
				SendMessage(1, "  Last modified on: %s", LAST_MODIFIED);
			}
			else if (strcasecmp(sep.arg[0], "serverinfo") == 0 && admin >= AccountStatus::GMMgmt) {
				if (strcasecmp(sep.arg[1], "os") == 0)	{
				#ifdef _WINDOWS
					GetOS();
					char intbuffer [sizeof(unsigned long)];
					SendMessage(1, "Operating system information.");
					SendMessage(1, "  %s", Ver_name);
					SendMessage(1, "  Build number: %s", ultoa(Ver_build, intbuffer, 10));
					SendMessage(1, "  Minor version: %s", ultoa(Ver_min, intbuffer, 10));
					SendMessage(1, "  Major version: %s", ultoa(Ver_maj, intbuffer, 10));
					SendMessage(1, "  Platform Id: %s", ultoa(Ver_pid, intbuffer, 10));
				#else
					char os_string[100];
					SendMessage(1, "Operating system information.");
					SendMessage(1, "  %s", GetOS(os_string));
				#endif
				}
				else {
					SendMessage(1, "Usage: Serverinfo [type]");
					SendMessage(1, "  OS - Operating system version information.");
				}
			}
			else if (strcasecmp(sep.arg[0], "IPLookup") == 0 && admin >= AccountStatus::GMMgmt) {
				client_list.SendCLEList(admin, 0, this, sep.argplus[1]);
			}
			else if (strcasecmp(sep.arg[0], "LSReconnect") == 0 && admin >= AccountStatus::GMAdmin) {
				#ifdef _WINDOWS
					_beginthread(AutoInitLoginServer, 0, nullptr);
				#else
					pthread_t thread;
					pthread_create(&thread, nullptr, &AutoInitLoginServer, nullptr);
				#endif
				RunLoops = true;
				SendMessage(1, "  Login Server Reconnect manually restarted by Console");
				LogInfo("Login Server Reconnect manually restarted by Console");
			}
			else if (strcasecmp(sep.arg[0], "zonelock") == 0 && admin >= consoleZoneStatus) {
				if (strcasecmp(sep.arg[1], "list") == 0) {
					zoneserver_list.ListLockedZones(0, this);
				}
				else if (strcasecmp(sep.arg[1], "lock") == 0 && admin >= AccountStatus::GMAdmin) {
					uint16 tmp = ZoneID(sep.arg[2]);
					if (tmp) {
						if (zoneserver_list.SetLockedZone(tmp, true))
							zoneserver_list.SendEmoteMessage(0, 0, 80, 15, "Zone locked: %s", ZoneName(tmp));
						else
							SendMessage(1, "Failed to change lock");
					}
					else
						SendMessage(1, "Usage: #zonelock lock [zonename]");
				}
				else if (strcasecmp(sep.arg[1], "unlock") == 0 && admin >= AccountStatus::GMAdmin) {
					uint16 tmp = ZoneID(sep.arg[2]);
					if (tmp) {
						if (zoneserver_list.SetLockedZone(tmp, false))
							zoneserver_list.SendEmoteMessage(0, 0, 80, 15, "Zone unlocked: %s", ZoneName(tmp));
						else
							SendMessage(1, "Failed to change lock");
					}
					else
						SendMessage(1, "Usage: #zonelock unlock [zonename]");
				}
				else {
					SendMessage(1, "#zonelock sub-commands");
					SendMessage(1, "  list");
					if (admin >= AccountStatus::GMAdmin) {
						SendMessage(1, "  lock [zonename]");
						SendMessage(1, "  unlock [zonename]");
					}
				}
			}
			else if (strcasecmp(sep.arg[0], "reloadworld") == 0 && admin > AccountStatus::GMAdmin)
			{
				SendEmoteMessage(0,0,0,15,"Reloading World...");
				auto pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
				ReloadWorld_Struct* RW = (ReloadWorld_Struct*) pack->pBuffer;
				RW->global_repop = ReloadWorld::Repop;
				zoneserver_list.SendPacket(pack);
				safe_delete(pack);
			}
			else if (strcasecmp(sep.arg[0], "") == 0){
				/* Hit Enter with no command */
			}
			else {
				SendMessage(1, "Command unknown.");
			}
			if (state == CONSOLE_STATE_CONNECTED)
				SendPrompt();
			break;
		}
		default: {
			break;
		}
	}
}

void Console::SendPrompt() {
	if (tcpc->GetEcho())
		SendMessage(0, "%s> ", paccountname);
}

