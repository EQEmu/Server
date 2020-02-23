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

#ifndef EQEMU_CLIENTMANAGER_H
#define EQEMU_CLIENTMANAGER_H

#include "../common/global_define.h"
#include "../common/opcodemgr.h"
#include "../common/net/eqstream.h"
#include "client.h"
#include <list>

/**
* Client manager class, holds all the client objects and does basic processing.
*/
class ClientManager {
public:
	/**
	 * Constructor: sets up the stream factories and opcode managers
	 */
	ClientManager();

	/**
	 * Destructor: shuts down the streams and opcode managers
	 */
	~ClientManager();

	/**
	 * Processes every client in the internal list, removes them if necessary.
	 */
	void Process();

	/**
	 * Removes a client with a certain account id
	 *
	 * @param account_id
	 * @param loginserver
	 */
	void RemoveExistingClient(unsigned int account_id, const std::string &loginserver);

	/**
	 * Gets a client (if exists) by their account id
	 *
	 * @param account_id
	 * @param loginserver
	 * @return
	 */
	Client *GetClient(unsigned int account_id, const std::string &loginserver);
private:

	/**
	 * Processes disconnected clients, removes them if necessary
	 */
	void ProcessDisconnect();

	std::list<Client *>      clients;
	OpcodeManager            *titanium_ops;
	EQ::Net::EQStreamManager *titanium_stream;
	OpcodeManager            *sod_ops;
	EQ::Net::EQStreamManager *sod_stream;
};

#endif

