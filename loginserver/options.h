/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_OPTIONS_H
#define EQEMU_OPTIONS_H

/**
 * Collects options on one object, because having a bunch of global variables floating around is
 * really ugly and just a little dangerous.
 */
class Options {
public:

	/**
	 * Constructor: Default options
	 */
	Options() :
		allow_unregistered(true),
		trace(false),
		dump_in_packets(false),
		dump_out_packets(false),
		encryption_mode(5),
		reject_duplicate_servers(false),
		allow_password_login(true),
		allow_token_login(false),
		auto_create_accounts(false) {}

	/**
	* Sets allow_unregistered.
	*/
	inline void AllowUnregistered(bool b) { allow_unregistered = b; }

	/**
	* Returns the value of allow_unregistered.
	*/
	inline bool IsUnregisteredAllowed() const { return allow_unregistered; }

	/**
	* Sets trace.
	*/
	inline void Trace(bool b) { trace = b; }

	/**
	* Returns the value of trace.
	*/
	inline bool IsTraceOn() const { return trace; }

	/**
	* Sets trace.
	*/
	inline void WorldTrace(bool b) { world_trace = b; }

	/**
	* Returns the value of trace.
	*/
	inline bool IsWorldTraceOn() const { return world_trace; }

	/**
	* Sets dump_in_packets.
	*/
	inline void DumpInPackets(bool b) { dump_in_packets = b; }

	/**
	* Returns the value of dump_in_packets.
	*/
	inline bool IsDumpInPacketsOn() const { return dump_in_packets; }

	/**
	* Sets dump_out_packets.
	*/
	inline void DumpOutPackets(bool b) { dump_out_packets = b; }

	/**
	* Returns the value of dump_out_packets.
	*/
	inline bool IsDumpOutPacketsOn() const { return dump_out_packets; }

	/**
	* Sets encryption_mode.
	*/
	inline void EncryptionMode(int m) { encryption_mode = m; }

	/**
	* Returns the value of encryption_mode.
	*/
	inline int GetEncryptionMode() const { return encryption_mode; }

	/**
	* Sets whether we are rejecting duplicate servers or not.
	*/
	inline void RejectDuplicateServers(bool b) { reject_duplicate_servers = b; }

	/**
	* Returns whether we are rejecting duplicate servers or not.
	*/
	inline bool IsRejectingDuplicateServers() { return reject_duplicate_servers; }

	inline void AllowTokenLogin(bool b) { allow_token_login = b; }
	inline bool IsTokenLoginAllowed() const { return allow_token_login; }

	inline void AllowPasswordLogin(bool b) { allow_password_login = b; }
	inline bool IsPasswordLoginAllowed() const { return allow_password_login; }

	inline void AutoCreateAccounts(bool b) { auto_create_accounts = b; }
	inline bool CanAutoCreateAccounts() const { return auto_create_accounts; }

	inline void AutoLinkAccounts(bool b) { auto_link_accounts = b; }
	inline bool CanAutoLinkAccounts() const { return auto_link_accounts; }

	inline void EQEmuLoginServerAddress(std::string v) { eqemu_loginserver_address = v; }
	inline std::string GetEQEmuLoginServerAddress() const { return eqemu_loginserver_address; }

	inline void DefaultLoginServerName(std::string v) { default_loginserver_name = v; }
	inline std::string GetDefaultLoginServerName() const { return default_loginserver_name; }

	inline void UpdateInsecurePasswords(bool b) { update_insecure_passwords = b; }
	inline bool IsUpdatingInsecurePasswords() const { return update_insecure_passwords; }

private:
	bool        allow_unregistered;
	bool        trace;
	bool        world_trace;
	bool        dump_in_packets;
	bool        dump_out_packets;
	bool        reject_duplicate_servers;
	bool        allow_token_login;
	bool        allow_password_login;
	bool        auto_create_accounts;
	bool        auto_link_accounts;
	bool        update_insecure_passwords;
	int         encryption_mode;
	std::string eqemu_loginserver_address;
	std::string default_loginserver_name;
};

#endif

