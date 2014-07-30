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
#ifndef WI_RPC_H
#define WI_RPC_H

void WriteWebCallResponseString(per_session_data_eqemu *session, rapidjson::Document &doc, std::string result, bool error, bool send_no_id = false);
void WriteWebCallResponseInt(per_session_data_eqemu *session, rapidjson::Document &doc, int result, bool error, bool send_no_id = false);
void WriteWebCallResponseBoolean(per_session_data_eqemu *session, rapidjson::Document &doc, bool result, bool error, bool send_no_id = false);
int CheckTokenAuthorization(per_session_data_eqemu*);

#endif

