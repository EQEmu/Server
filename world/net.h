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
#ifndef WIN32
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
#else
	#include <cerrno>
	#include <fcntl.h>
	#include <winsock2.h>
	#include <windows.h>
#endif

void CatchSignal(int sig_num);
void UpdateWindowTitle(char* iNewTitle);

#define EQ_WORLD_PORT 9000		//mandated by the client
#define LOGIN_PORT	5997

class NetConnection
{
public:
	NetConnection() {
		world_locked = false;
		for (int i=0; i<5; i++) {
			memset(loginaddress[i], 0, sizeof(loginaddress[i]));
			loginport[i] = LOGIN_PORT;
		}
		memset(worldname, 0, sizeof(worldname));
		memset(worldshortname, 0, sizeof(worldshortname));
		memset(worldaccount, 0, sizeof(worldaccount));
		memset(worldpassword, 0, sizeof(worldpassword));
		memset(worldaddress, 0, sizeof(worldaddress));
		memset(chataddress, 0, sizeof(chataddress));
		DEFAULTSTATUS=0;
		LoginServerInfo = 0;//ReadLoginINI();
		UpdateStats = false;
	}
	~NetConnection() { }

	bool ReadLoginINI();
	bool LoginServerInfo;
	bool UpdateStats;
	char* GetLoginInfo(uint16* oPort);
	inline char* GetLoginAddress(uint8 i)	{ return loginaddress[i]; }
	inline uint16 GetLoginPort(uint8 i)		{ return loginport[i]; }
	inline char* GetWorldName()			{ return worldname; }
	inline char* GetWorldShortName()		{ return worldshortname; }
	inline char* GetWorldAccount()			{ return worldaccount; }
	inline char* GetWorldPassword()		{ return worldpassword; }
	inline char* GetWorldAddress()			{ return worldaddress; }
	inline uint8 GetDefaultStatus()			{ return DEFAULTSTATUS; }
	inline char* GetChatAddress()			{ return chataddress; }
	uint16 GetChatPort()			{ return chatport; }
	bool world_locked;
private:
	int		listening_socket;
	char	loginaddress[5][255];
	uint16	loginport[5];
	uint16	chatport;
	char	worldname[201];
	char	worldshortname[31];
	char	worldaccount[31];
	char	worldpassword[31];
	char	worldaddress[255];
	char	chataddress[255];
	uint8	DEFAULTSTATUS;

};
