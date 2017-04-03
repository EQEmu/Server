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
#include "eqw_http_handler.h"
#include "../common/SocketLib/Base64.h"
#include "eqw_parser.h"
#include "eqw.h"
#include "http_request.h"

#include "worlddb.h"
#include "console.h"

Mime EQWHTTPHandler::s_mime;
#ifdef EMBPERL
EQWParser *EQWHTTPHandler::s_parser = nullptr;
#endif
const int EQWHTTPHandler::READ_BUFFER_LEN = 1024;	//for page IO, was a static const member, but VC6 got mad.

EQWHTTPHandler::EQWHTTPHandler(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort)
:	HttpdSocket(ID,in_socket,irIP,irPort),
	m_closeOnFinish(false)
{
}

EQWHTTPHandler::~EQWHTTPHandler() {

}

#ifdef EMBPERL
EQWParser *EQWHTTPHandler::GetParser() {
	if(s_parser == nullptr) {
		EQW::Singleton()->ClearOutput();
		s_parser = new EQWParser();
		const std::string &res = EQW::Singleton()->GetOutput();
		if(!res.empty()) {
			printf("EQWParser Init output:\n%s\n\n", res.c_str());
			EQW::Singleton()->ClearOutput();
		}
	}
	return(s_parser);
}
#endif

/*void EQWHTTPHandler::OnWrite() {
	HttpdSocket::OnWrite();
	if(m_closeOnFinish && GetOutputLength() == 0) {
//		printf("CLOSING\n");
		Close();
	}
}*/


void EQWHTTPHandler::Exec() {
	m_sentHeaders = false;
	m_responseCode = "200";
//	printf("Request: %s, %s, %s, %s.\n", GetMethod().c_str(), GetUrl().c_str(), GetUri().c_str(), GetQueryString().c_str());

	SetHttpVersion("HTTP/1.0");
	AddResponseHeader("Connection", "close");

	if(GetUri().find("..") != std::string::npos) {
		SendResponse("403", "Forbidden");
		printf("%s is forbidden.\n", GetUri().c_str());
		return;
	}

	if(!CheckAuth()) {
		AddResponseHeader("Content-type", "text/plain");
		AddResponseHeader("WWW-Authenticate", "Basic realm=\"EQEmulator\"");
		SendResponse("401", "Authorization Required");
		SendString("Gotta Authenticate.");
	} else {
		std::string::size_type start = GetUri().find_first_not_of('/');
		std::string page;
		if(start != std::string::npos)
			page = GetUri().substr(start);
		else
			page = "index.html";
		SendPage(page);
	}
/*	if (!Detach()) {
		printf("Unable to detach...\n");
	}
	if(GetOutputLength() > 0) {
		//we cannot close yet
		m_closeOnFinish = true;
	} else {
		Close();
	}*/
	Free();	//the "app" side (us) is done with this connection too...
	Disconnect();
}

void EQWHTTPHandler::OnHeader(const std::string& key,const std::string& value) {
	HttpdSocket::OnHeader(key, value);

	if (!strcasecmp(key.c_str(),"Authorization")) {
		if(strncasecmp(value.c_str(), "Basic ", 6)) {
			printf("Invalid auth type. Expected Basic: %s\n", value.c_str());
			return;
		}

		std::string dec;
		Base64::decode(value.c_str() + 6, dec);

		std::string::size_type cpos;
		cpos = dec.find_first_of(':');
		if(cpos == std::string::npos) {
			printf("Invalid auth string: %s\n", dec.c_str());
			return;
		}

		m_username = dec.substr(0, cpos);
		m_password = dec.substr(cpos+1);
	}
}

//we should prolly cache login info here... if we load a fresh page, we could be checking
//their auth dozens of times rather quickly...
bool EQWHTTPHandler::CheckAuth() const {
	if(m_username.length() < 1)
		return(false);

	int16 status = 0;
	uint32 acctid = database.CheckLogin(m_username.c_str(), m_password.c_str(), &status);
	if(acctid == 0) {
		Log(Logs::Detail, Logs::World_Server, "Login autentication failed for %s with '%s'", m_username.c_str(), m_password.c_str());
		return(false);
	}
	if(status < httpLoginStatus) {
		Log(Logs::Detail, Logs::World_Server, "Login of %s failed: status too low.", m_username.c_str());
		return(false);
	}

	return(true);
}

void EQWHTTPHandler::SendPage(const std::string &file) {

	std::string path = "templates/";
	path += file;

	FILE *f = fopen(path.c_str(), "rb");
	if(f == nullptr) {
		SendResponse("404", "Not Found");
		SendString("Not found.");
		printf("%s not found.\n", file.c_str());
		return;
	}

	std::string type = s_mime.GetMimeFromFilename(file);
	AddResponseHeader("Content-type", type);

	bool process = false;
#ifdef EMBPERL
	if(type == "text/html")
		process = true;
	else {
		//not processing, send headers right away
#endif
		SendResponse("200", "OK");
#ifdef EMBPERL
	}
#endif

	auto buffer = new char[READ_BUFFER_LEN + 1];
	size_t len;
	std::string to_process;
	while((len = fread(buffer, 1, READ_BUFFER_LEN, f)) > 0) {
		buffer[len] = '\0';
		if(process)
			to_process += buffer;
		else
			SendBuf(buffer, len);
	}
	delete[] buffer;
	fclose(f);
#ifdef EMBPERL
	if(process) {
		//convert the base form into a useful perl exportable form
		HTTPRequest req(this, GetHttpForm());
		GetParser()->SetHTTPRequest("testing", &req);

		//parse out the page and potentially pass some stuff on to perl.
		ProcessAndSend(to_process);

		//clear out the form, just in case (since it gets destroyed next)
		GetParser()->SetHTTPRequest("testing", nullptr);
	}
#endif
}

bool EQWHTTPHandler::LoadMimeTypes(const char *filename) {
	return(s_mime.LoadMimeFile(filename));
}

#ifdef EMBPERL
void EQWHTTPHandler::ProcessAndSend(const std::string &str) {
	std::string::size_type len = str.length();
	std::string::size_type start = 0;
	std::string::size_type pos, end;

	while((pos = str.find("<?", start)) != std::string::npos) {
		//send all the crap leading up to the script block
		if(pos != start) {
			ProcessText(str.c_str() + start, pos-start);
		}

		//look for the end of this script block...
		end = str.find("?>", pos+2);
		if(end == std::string::npos) {
			//terminal ?> not found... should issue a warning or something...
			std::string scriptBody = str.substr(pos+2);
			ProcessScript(scriptBody);
			start = len;
			break;
		} else {
			//script only consumes some of this buffer...
			std::string scriptBody = str.substr(pos+2, end-pos-2);
			ProcessScript(scriptBody);
			start = end + 2;
		}
	}

	//send whatever is left over
	if(start != len)
		ProcessText(str.c_str() + start, len-start);
}

void EQWHTTPHandler::ProcessScript(const std::string &script_body) {
	const char *script = script_body.c_str();
	if(strcmp("perl", script) == 0)
		script += 4;	//allow <?perl

//	printf("Script: ''''%s''''\n\n", script_body.c_str());

	GetParser()->EQW_eval("testing", script_body.c_str());
	const std::string &res = EQW::Singleton()->GetOutput();
	if(!res.empty()) {
		ProcessText(res.c_str(), res.length());
		EQW::Singleton()->ClearOutput();
	}
}

void EQWHTTPHandler::ProcessText(const char *txt, int len) {
	if(!m_sentHeaders) {
		SendResponse(m_responseCode, "OK");
		m_sentHeaders = true;
	}
	SendBuf(txt, len);
}
#endif


EQWHTTPServer::EQWHTTPServer()
: m_port(0)
{
}

void EQWHTTPServer::CreateNewConnection(uint32 ID, SOCKET in_socket, uint32 irIP, uint16 irPort) {
	auto conn = new EQWHTTPHandler(ID, in_socket, irIP, irPort);
	AddConnection(conn);
}

void EQWHTTPServer::Stop() {
	Log(Logs::Detail, Logs::World_Server, "Requesting that HTTP Service stop.");
	m_running = false;
	Close();
}

bool EQWHTTPServer::Start(uint16 port, const char *mime_file) {
	if(m_running) {
		Log(Logs::Detail, Logs::World_Server, "HTTP Service is already running on port %d", m_port);
		return(false);
	}

	//load up our nice mime types
	if(!EQWHTTPHandler::LoadMimeTypes(mime_file)) {
		Log(Logs::Detail, Logs::World_Server, "Failed to load mime types from '%s'", mime_file);
		return(false);
	} else {
		Log(Logs::Detail, Logs::World_Server, "Loaded mime types from %s", mime_file);
	}

	//fire up the server thread
	char errbuf[TCPServer_ErrorBufferSize];
	if(!Open(port, errbuf)) {
		Log(Logs::Detail, Logs::World_Server, "Unable to bind to port %d for HTTP service: %s", port, errbuf);
		return(false);
	}

	m_running = true;
	m_port = port;

	/*

#ifdef _WINDOWS
	_beginthread(ThreadProc, 0, this);
#else
	pthread_create(&m_thread, nullptr, ThreadProc, this);
#endif*/

	return(true);
}

/*
void EQWHTTPServer::Run() {
	Log.LogDebugType(Logs::Detail, Logs::World_Server, "HTTP Processing thread started on port %d", m_port);
	do {
#warning DELETE THIS IF YOU DONT USE IT
		Sleep(10);
	} while(m_running);
	Log.LogDebugType(Logs::Detail, Logs::World_Server, "HTTP Processing thread terminating on port %d", m_port);
}

ThreadReturnType EQWHTTPServer::ThreadProc(void *data) {
	((EQWHTTPServer *) data)->Run();
	THREAD_RETURN(nullptr);
}*/

