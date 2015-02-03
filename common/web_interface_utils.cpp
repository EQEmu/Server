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

#include "../common/global_define.h"
#include "web_interface_utils.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>


using namespace rapidjson;
using namespace std;

std::vector<std::string> explode_string(std::string const & s, char delim)
{
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim);)
	{
		result.push_back(std::move(token));
	}

	return result;
}

std::string MakeJSON(std::string json)
{
	StringBuffer s;
	Writer<StringBuffer> writer(s);
	writer.StartObject();

	auto arg_c = explode_string(json, ',');
	if (arg_c.size() == 0)
	{
		auto arg_v = explode_string(json, ':');
		if (arg_v.size() > 0)
		{
			for (int j = 0; j < arg_v.size(); j++)
			{
				writer.String(arg_v[j].c_str());
			}
		}
	}
	else
	{
		for (int i = 0; i < arg_c.size(); i++)
		{
			auto arg_v = explode_string(arg_c[i], ':');
			for (int j = 0; j < arg_v.size(); j++)
			{
				writer.String(arg_v[j].c_str());
			}
		}
	}

	writer.EndObject();
	return s.GetString();
}