#ifndef LOGINSERVERLIST_H_
#define LOGINSERVERLIST_H_

#include "../common/servertalk.h"
#include "../common/timer.h"
#include "../common/queue.h"
#include "../common/eq_packet_structs.h"
#include "../common/mutex.h"
#include <list>

class LoginServer;

class LoginServerList{
public:
	LoginServerList();
	~LoginServerList();

	void	Add(const char*, uint16, const char*, const char*, bool);

	bool	SendInfo();
	bool	SendNewInfo();
	bool	SendStatus();

	bool	SendPacket(ServerPacket *pack);
	bool	SendAccountUpdate(ServerPacket *pack);
	bool	Connected();
	bool	AllConnected();
	bool	MiniLogin();
	bool	CanUpdate();
	size_t GetServerCount() const { return m_list.size(); }

protected:
	std::list<std::unique_ptr<LoginServer>> m_list;
};




#endif /*LOGINSERVERLIST_H_*/
