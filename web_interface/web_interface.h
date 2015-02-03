/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

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
#ifndef WI_WEB_INTERFACE_H
#define WI_WEB_INTERFACE_H

#include "../common/global_define.h"
#include "../common/opcodemgr.h"
#include "../common/eq_stream_factory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/eqemu_config.h"
#include "../common/web_interface_utils.h"
#include "../common/string_util.h"
#include "../common/uuid.h"
#include "../common/shareddb.h"
#include "worldserver.h"
#include "lib/libwebsockets.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <signal.h>
#include <list>
#include <map>

struct per_session_data_eqemu {
	std::string auth;
	std::string uuid;
	std::list<std::string> *send_queue;
};

typedef void(*MethodHandler)(per_session_data_eqemu*, rapidjson::Document&, std::string&);

#endif