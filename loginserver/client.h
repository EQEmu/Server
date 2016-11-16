/*	EQEMu: Everquest Server Emulator
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
#ifndef EQEMU_CLIENT_H
#define EQEMU_CLIENT_H

#include "../common/global_define.h"
#include "../common/opcodemgr.h"
#include "../common/random.h"

#include <memory>
#include "../common/eq_stream_intf.h"

enum LSClientVersion
{
	cv_titanium,
	cv_sod
};

enum LSClientStatus
{
	cs_not_sent_session_ready,
	cs_waiting_for_login,
	cs_failed_to_login,
	cs_logged_in
};

/**
* Client class, controls a single client and it's
* connection to the login server.
*/
class Client
{
public:
	/**
	* Constructor, sets our connection to c and version to v
	*/
	Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v);

	/**
	* Destructor.
	*/
	~Client() { }

	/**
	* Processes the client's connection and does various actions.
	*/
	bool Process();

	/**
	* Sends our reply to session ready packet.
	*/
	void Handle_SessionReady(const char* data, unsigned int size);

	/**
	* Verifies login and send a reply.
	*/
	void Handle_Login(const char* data, unsigned int size);

	/**
	* Sends a packet to the requested server to see if the client is allowed or not.
	*/
	void Handle_Play(const char* data);

	/**
	* Sends a server list packet to the client.
	*/
	void SendServerListPacket(uint32 seq);

	/**
	* Sends the input packet to the client and clears our play response states.
	*/
	void SendPlayResponse(EQApplicationPacket *outapp);

	/**
	* Generates a random login key for the client during login.
	*/
	void GenerateKey();

	/**
	* Gets the account id of this client.
	*/
	unsigned int GetAccountID() const { return account_id; }

	/**
	* Gets the account name of this client.
	*/
	std::string GetAccountName() const { return account_name; }

	/**
	* Gets the key generated at login for this client.
	*/
	std::string GetKey() const { return key; }

	/**
	* Gets the server selected to be played on for this client.
	*/
	unsigned int GetPlayServerID() const { return play_server_id; }

	/**
	* Gets the play sequence state for this client.
	*/
	unsigned int GetPlaySequence() const { return play_sequence_id; }

	/**
	* Gets the connection for this client.
	*/
	std::shared_ptr<EQStreamInterface> GetConnection() { return connection; }

	EQEmu::Random random;
private:
	std::shared_ptr<EQStreamInterface> connection;
	LSClientVersion version;
	LSClientStatus status;

	std::string account_name;
	unsigned int account_id;
	unsigned int play_server_id;
	unsigned int play_sequence_id;
	std::string key;
};

#endif

