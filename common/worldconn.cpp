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
#include "../common/eqemu_logsys.h"
#include <iomanip>

#include "worldconn.h"
#include "eqemu_config.h"
#include "md5.h"
#include "servertalk.h"

WorldConnection::WorldConnection(EmuTCPConnection::ePacketMode mode, const char *password)
: m_password(password)
{
	tcpc.SetPacketMode(mode);
	pTryReconnect = true;
	pConnected = false;
}

WorldConnection::~WorldConnection() {
}

bool WorldConnection::SendPacket(ServerPacket* pack) {
	if (!Connected())
		return false;
	return tcpc.SendPacket(pack);
}

void WorldConnection::OnConnected() {
	const EQEmuConfig *Config=EQEmuConfig::get();
	Log(Logs::General, Logs::Netcode, "[WORLD] Connected to World: %s:%d", Config->WorldIP.c_str(), Config->WorldTCPPort);

	auto pack = new ServerPacket(ServerOP_ZAAuth, 16);
	MD5::Generate((const uchar*) m_password.c_str(), m_password.length(), pack->pBuffer);
	SendPacket(pack);
	safe_delete(pack);
}

void WorldConnection::Process() {
	//persistent connection....
	if (!Connected()) {
		pConnected = tcpc.Connected();
		if (pConnected) {
			OnConnected();
		}
		else
			return;
	}

}

void WorldConnection::AsyncConnect() {
	const EQEmuConfig *Config=EQEmuConfig::get();
	tcpc.AsyncConnect(Config->WorldIP.c_str(), Config->WorldTCPPort);
}

bool WorldConnection::Connect() {
	const EQEmuConfig *Config=EQEmuConfig::get();
	char errbuf[TCPConnection_ErrorBufferSize];
	if (tcpc.Connect(Config->WorldIP.c_str(), Config->WorldTCPPort, errbuf)) {
		return true;
	} else {
		Log(Logs::General, Logs::Netcode, "[WORLD] WorldConnection connect: Connecting to the server %s:%d failed: %s", Config->WorldIP.c_str(), Config->WorldTCPPort, errbuf);
	}
	return false;
}

void WorldConnection::Disconnect() {
	tcpc.Disconnect();
}

