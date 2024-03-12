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

