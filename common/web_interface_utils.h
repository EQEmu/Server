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
#ifndef COMMON_WEBINTUTIL_H
#define COMMON_WEBINTUTIL_H

#include "../common/global_define.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>

std::vector<std::string> explode_string(std::string const & s, char delim);
std::string MakeJSON(std::string json);

#endif

