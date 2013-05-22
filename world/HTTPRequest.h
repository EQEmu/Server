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
#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include "../common/types.h"
#include <map>
#include <string>

//this object acts as a friendlier interface to the HttpdForm object (perl exportable)
//which does more effecient lookups


class HttpdForm;
class EQWHTTPHandler;

class HTTPRequest {
public:
	HTTPRequest(EQWHTTPHandler *h, HttpdForm *form);

//BEGIN PERL EXPORT

	Const_char * get(Const_char *name, Const_char *default_value = "") const;
	int getInt(Const_char *name, int default_value = 0) const;
	float getFloat(Const_char *name, float default_value = 0.0) const;

	//returns a database-safe string
	Const_char * getEscaped(Const_char *name, Const_char *default_value = "") const;

	std::map<std::string,std::string> get_all() const;

	void redirect(Const_char *URL);
	void SetResponseCode(Const_char *code);
	void header(Const_char *name, Const_char *value);
//END PERL EXPORT

protected:
	EQWHTTPHandler *const m_handler;
	std::map<std::string, std::string> m_values;
};

#endif /*HTTPREQUEST_H_*/

