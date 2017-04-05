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
#include <iomanip>
#include <stdlib.h>
#include "../common/version.h"

#define IGNORE_LS_FATAL_ERROR

#include "../common/servertalk.h"
#include "login_server.h"
#include "login_server_list.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "zonelist.h"
#include "clientlist.h"
#include "world_config.h"

extern ZSList zoneserver_list;
extern LoginServerList loginserverlist;
extern ClientList client_list;
extern uint32 numzones;
extern uint32 numplayers;
extern volatile bool	RunLoops;

LoginServerList::LoginServerList() {
}

LoginServerList::~LoginServerList() {
}

void LoginServerList::Add(const char* iAddress, uint16 iPort, const char* Account, const char* Password)
{
	auto loginserver = new LoginServer(iAddress, iPort, Account, Password);
	list.Insert(loginserver);
}

bool LoginServerList::Process() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		iterator.GetData()->Process();
		iterator.Advance();
	}
	return true;
}

#ifdef _WINDOWS
void AutoInitLoginServer(void *tmp) {
#else
void *AutoInitLoginServer(void *tmp) {
#endif
	loginserverlist.InitLoginServer();
#ifndef WIN32
	return 0;
#endif
}

void LoginServerList::InitLoginServer() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		iterator.GetData()->InitLoginServer();
		iterator.Advance();
	}
}

bool LoginServerList::SendInfo() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		iterator.GetData()->SendInfo();
		iterator.Advance();
	}
	return true;
}

bool LoginServerList::SendNewInfo() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		iterator.GetData()->SendNewInfo();
		iterator.Advance();
	}
	return true;
}

bool LoginServerList::SendStatus() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		iterator.GetData()->SendStatus();
		iterator.Advance();
	}
	return true;
}

bool LoginServerList::SendPacket(ServerPacket* pack) {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		iterator.GetData()->SendPacket(pack);
		iterator.Advance();
	}
	return true;
}

bool LoginServerList::SendAccountUpdate(ServerPacket* pack) {
	LinkedListIterator<LoginServer*> iterator(list);

	Log(Logs::Detail, Logs::World_Server, "Requested to send ServerOP_LSAccountUpdate packet to all loginservers");
	iterator.Reset();
	while(iterator.MoreElements()){
		if(iterator.GetData()->CanUpdate()) {
			iterator.GetData()->SendAccountUpdate(pack);
		}
		iterator.Advance();
	}
	return true;
}

bool LoginServerList::Connected() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		if(iterator.GetData()->Connected())
			return true;
		iterator.Advance();
	}
	return false;
}

bool LoginServerList::AllConnected() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		if(iterator.GetData()->Connected() == false)
			return false;
		iterator.Advance();
	}
	return true;
}

bool LoginServerList::MiniLogin() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		if(iterator.GetData()->MiniLogin())
			return true;
		iterator.Advance();
	}
	return false;
}

bool LoginServerList::CanUpdate() {
	LinkedListIterator<LoginServer*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()){
		if(iterator.GetData()->CanUpdate())
			return true;
		iterator.Advance();
	}
	return false;
}

