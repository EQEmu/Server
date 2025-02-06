#ifndef EQEMU_CLIENTMANAGER_H
#define EQEMU_CLIENTMANAGER_H

#include "../common/global_define.h"
#include "../common/opcodemgr.h"
#include "../common/net/eqstream.h"
#include "client.h"
#include <list>

class ClientManager {
public:
	ClientManager();
	~ClientManager();
	void Process();
	void RemoveExistingClient(unsigned int c, const std::string &loginserver);
	Client *GetClient(unsigned int c, const std::string &loginserver);
private:
	void ProcessDisconnect();

	std::list<Client *>      m_clients;
	OpcodeManager            *m_titanium_ops;
	EQ::Net::EQStreamManager *m_titanium_stream;
	OpcodeManager            *m_sod_ops;
	EQ::Net::EQStreamManager *m_sod_stream;
	OpcodeManager            *m_larion_ops;
	EQ::Net::EQStreamManager *m_larion_stream;
};

#endif

