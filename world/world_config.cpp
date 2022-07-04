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
#include "../common/eqemu_logsys.h"
#include "world_config.h"
#include "../common/ip_util.h"

WorldConfig *WorldConfig::_world_config = nullptr;

std::string WorldConfig::GetByName(const std::string &var_name) const
{
	if (var_name == "UpdateStats") {
		return (UpdateStats ? "true" : "false");
	}
	if (var_name == "LoginDisabled") {
		return (LoginDisabled ? "true" : "false");
	}
	return (EQEmuConfig::GetByName(var_name));
}

void WorldConfig::CheckForPossibleConfigurationIssues()
{
	if (_config->DisableConfigChecks) {
		LogInfo("Configuration checking [disabled]");
		return;
	}

	const std::string   local_address  = IpUtil::GetLocalIPAddress();
	const std::string   public_address = IpUtil::GetPublicIPAddress();
	const std::string   config_file    = "eqemu_config.json";
	const std::ifstream is_in_docker("/.dockerenv");

	LogInfo("Checking for possible configuration issues");
	LogInfo("To disable configuration checks, set [server->disable_config_checks] to [true] in [{}]", config_file);

	std::cout << std::endl;

	// lan detection
	if (local_address != public_address
		&& IpUtil::IsIpInPrivateRfc1918(local_address)
		&& local_address != _config->LocalAddress
		&& !is_in_docker
		) {
		LogWarning("# LAN detection (Configuration)");
		LogWarning("");
		LogWarning("You appear to be on a LAN and your localaddress may not be properly set!");
		LogWarning("This can prevent local clients from properly connecting to your server");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server->world] variable [localaddress]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", _config->LocalAddress, local_address);
		std::cout << std::endl;
	}

	// docker configuration
	if ((_config->LocalAddress.empty() && _config->WorldAddress.empty()) && is_in_docker) {
		LogWarning("# Docker Configuration (Configuration)");
		LogWarning("");
		LogWarning("You appear to running EQEmu in a docker container");
		LogWarning("In order for networking to work properly you will need to properly configure your server");
		LogWarning("");
		LogWarning("If your Docker host is on a [LAN], your [localaddress] variable under [server->world] will need to");
		LogWarning("be set to your LAN address on the host, not the container address. [address] will need to be your ");
		LogWarning("public address");
		LogWarning("");
		LogWarning("If your Docker host is on the [public internet], your [localaddress] variable under [server->world] can be empty or set to [127.0.0.1]. ");
		LogWarning("");
		LogWarning("[address] will need to be your public address");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server->world] variable(s) [localaddress] [address]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", _config->LocalAddress, local_address);
		LogWarning("Public address (eqemu_config) value [{}] detected value [{}]", _config->WorldAddress, public_address);
		std::cout << std::endl;
	}

	// public address different from configuration
	if (!_config->WorldAddress.empty() && public_address != _config->WorldAddress) {
		LogWarning("# Public address (Configuration)");
		LogWarning("");
		LogWarning("Your configured public address appears to be different from your configuration!");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server->world] variable [address]", config_file);
		LogWarning("");
		LogWarning("Public address (eqemu_config) value [{}] detected value [{}]", _config->WorldAddress, public_address);
		std::cout << std::endl;
	}

	// public address set to meta-address
	if (_config->WorldAddress == "0.0.0.0") {
		LogWarning("# Public meta-address (Configuration)");
		LogWarning("");
		LogWarning("Your configured public address is set to a meta-address (0.0.0.0) (all-interfaces)");
		LogWarning(
			"The meta-address may not work properly and it is recommended you configure your public address explicitly");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server->world] variable [address]", config_file);
		LogWarning("");
		LogWarning("Public address (eqemu_config) value [{}] detected value [{}]", _config->WorldAddress, public_address);
		std::cout << std::endl;
	}

	// local address set to meta-address
	if (_config->LocalAddress == "0.0.0.0") {
		LogWarning("# Local meta-address (Configuration)");
		LogWarning("");
		LogWarning("Your configured local address is set to a meta-address (0.0.0.0) (all-interfaces)");
		LogWarning(
			"The meta-address may not work properly and it is recommended you configure your local address explicitly");
		LogWarning("");
		LogWarning("Docs [https://docs.eqemu.io/server/installation/configure-your-eqemu_config/#world]");
		LogWarning("");
		LogWarning("Config file [{}] path [server->world] variable [localaddress]", config_file);
		LogWarning("");
		LogWarning("Local address (eqemu_config) value [{}] detected value [{}]", _config->LocalAddress, local_address);
		std::cout << std::endl;
	}
}
























