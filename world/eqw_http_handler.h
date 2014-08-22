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
#ifndef EQWHTTPHandler_H
#define EQWHTTPHandler_H

#include "../common/tcp_server.h"
#include "../common/tcp_connection.h"
#include "../common/SocketLib/HttpdSocket.h"
#include "../common/SocketLib/Mime.h"
#include "../common/types.h"

class EQWParser;

class EQWHTTPHandler : public HttpdSocket {
	static const int READ_BUFFER_LEN;
public:
	EQWHTTPHandler(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort);
	virtual ~EQWHTTPHandler();

	void SetResponseCode(const char *code) { m_responseCode = code; }

	//HttpdSocket interface:
	virtual void Exec();
	virtual void OnHeader(const std::string& key,const std::string& value);

	static bool LoadMimeTypes(const char *filename);
protected:
	bool CheckAuth() const;
	void SendPage(const std::string &file);

	//credentials
	std::string m_username;
	std::string m_password;

	bool m_closeOnFinish;
	std::string m_responseCode;
	bool m_sentHeaders;


	//our mime type manager
	static Mime s_mime;


#ifdef EMBPERL
	void ProcessAndSend(const std::string &entire_html_page);
	void ProcessScript(const std::string &script_body);
	void ProcessText(const char *txt, int len);

	static EQWParser *GetParser();

private:
	static EQWParser *s_parser;
#endif
};

class EQWHTTPServer : protected TCPServer<EQWHTTPHandler> {
public:
	EQWHTTPServer();

	bool Start(uint16 port, const char *mime_file);
	void Stop();


protected:
	volatile bool m_running;
	uint16 m_port;

	virtual void CreateNewConnection(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort);

/*	//I decided to put this into its own thread so that the HTTP pages
	//cannot block the main world server's operation.
	static ThreadReturnType ThreadProc(void* tmp);
	void Run();



#ifndef WIN32
	pthread_t m_thread;
#endif*/
};

#endif

