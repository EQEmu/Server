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
#ifndef XMLParser_H
#define XMLParser_H

#include "global_define.h"
#include "tinyxml/tinyxml.h"
#include "../common/types.h"

#include <string>
#include <map>

/*
* See note in XMLParser::ParseFile() before inheriting this class.
*/
class XMLParser {
public:
	typedef void (XMLParser::*ElementHandler)(TiXmlElement *ele);

	XMLParser();
	virtual ~XMLParser() {}

	bool ParseFile(const char *file, const char *root_ele);
	bool ParseStatus() const { return ParseOkay; }

protected:
	const char *ParseTextBlock(TiXmlNode *within, const char *name, bool optional = false);
	const char *GetText(TiXmlNode *within, bool optional = false);

	std::map<std::string,ElementHandler> Handlers;

	bool ParseOkay;

};

#endif

