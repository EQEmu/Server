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
#include "http_request.h"
#include "eqw_http_handler.h"
#include "../common/eqdb.h"
#include "../common/SocketLib/HttpdForm.h"
#include <cstdlib>

HTTPRequest::HTTPRequest(EQWHTTPHandler *h, HttpdForm *form)
: m_handler(h)
{
	std::string name, value;
	if(form->getfirst(name, value)) {
		m_values[name] = value;
		while(form->getnext(name, value))
			m_values[name] = value;
	}
}

const char *HTTPRequest::getEscaped(const char *name, const char *default_value) const {
	return(EQDB::Singleton()->escape_string(get(name, default_value)));
}

const char *HTTPRequest::get(const char *name, const char *default_value) const {
	std::map<std::string, std::string>::const_iterator res;
	res = m_values.find(name);
	if(res == m_values.end())
		return(default_value);
	return(res->second.c_str());
}

std::map<std::string,std::string> HTTPRequest::get_all() const {
	return m_values;
}

int HTTPRequest::getInt(const char *name, int default_value) const {
	std::map<std::string, std::string>::const_iterator res;
	res = m_values.find(name);
	if(res == m_values.end())
		return(default_value);
	return(atoi(res->second.c_str()));
}

float HTTPRequest::getFloat(const char *name, float default_value) const {
	std::map<std::string, std::string>::const_iterator res;
	res = m_values.find(name);
	if(res == m_values.end())
		return(default_value);
	return(atof(res->second.c_str()));
}

void HTTPRequest::header(Const_char *name, Const_char *value) {
	m_handler->AddResponseHeader(name, value);
}

void HTTPRequest::SetResponseCode(Const_char *code) {
	m_handler->SetResponseCode(code);
}

void HTTPRequest::redirect(Const_char *URL) {
	header("Location", URL);
	SetResponseCode("302");
}

