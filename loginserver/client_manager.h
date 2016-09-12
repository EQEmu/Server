/* EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

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
#ifndef EQEMU_CLIENTMANAGER_H
#define EQEMU_CLIENTMANAGER_H

#include "../common/global_define.h"
#include "../common/net/eqstream.h"
#include "../common/patch/login_sod.h"
#include "../common/patch/login_titanium.h"
#include "client.h"
#include <list>
#include <memory>

/**
* Client manager class, holds all the client objects and does basic processing.
*/
class ClientManager
{
public:
	/**
	* Constructor, sets up the stream factories and opcode managers.
	*/
	ClientManager();

	/**
	* Destructor, shuts down the streams and opcode managers.
	*/
	~ClientManager();

	/**
	* Sends a new server list to every client.
	*/
	void UpdateServerList();

	/**
	* Removes a client with a certain account id.
	*/
	void RemoveExistingClient(unsigned int account_id);

	/**
	* Gets a client (if exists) by their account id.
	*/
	Client *GetClient(unsigned int account_id);
private:
	void HandleNewConnectionTitanium(std::shared_ptr<EQ::Net::EQStream> connection);
	void HandleNewConnectionSod(std::shared_ptr<EQ::Net::EQStream> connection);
	void HandleConnectionChange(std::shared_ptr<EQ::Net::EQStream> connection, EQ::Net::DbProtocolStatus old_status, EQ::Net::DbProtocolStatus new_status);
	void HandlePacket(std::shared_ptr<EQ::Net::EQStream> connection, EmuOpcode opcode, EQ::Net::Packet &p);

	std::list<std::unique_ptr<Client>> clients;
	std::unique_ptr<EQ::Net::EQStreamManager> titanium_stream;
	std::unique_ptr<EQ::Net::EQStreamManager> sod_stream;
	std::unique_ptr<EQ::Patches::LoginTitaniumPatch> titanium_patch;
	std::unique_ptr<EQ::Patches::LoginSoDPatch> sod_patch;
};

#endif

