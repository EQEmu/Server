/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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

#ifndef __ucsconfig_H
#define __ucsconfig_H

#include "../common/eqemu_config.h"

class ucsconfig : public EQEmuConfig {
public:
	virtual std::string GetByName(const std::string &var_name) const;

private:

	static ucsconfig *_chat_config;

public:

	// Produce a const singleton
	static const ucsconfig *get() {
		if (_chat_config == nullptr)
			LoadConfig();
		return(_chat_config);
	}

	// Load the config
	static bool LoadConfig() {
		if (_chat_config != nullptr)
			delete _chat_config;
		_chat_config=new ucsconfig;
		_config=_chat_config;

		return _config->parseFile();
	}

};

#endif

