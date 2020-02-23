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

#include "ip_util.h"

/**
 * @param ip
 * @return
 */
uint32_t IpUtil::IPToUInt(const std::string &ip)
{
	int      a, b, c, d;
	uint32_t addr = 0;

	if (sscanf(ip.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
		return 0;
	}

	addr = a << 24;
	addr |= b << 16;
	addr |= c << 8;
	addr |= d;
	return addr;
}

/**
 * @param ip
 * @param network
 * @param mask
 * @return
 */
bool IpUtil::IsIpInRange(const std::string &ip, const std::string &network, const std::string &mask)
{
	uint32_t ip_addr      = IpUtil::IPToUInt(ip);
	uint32_t network_addr = IpUtil::IPToUInt(network);
	uint32_t mask_addr    = IpUtil::IPToUInt(mask);

	uint32_t net_lower = (network_addr & mask_addr);
	uint32_t net_upper = (net_lower | (~mask_addr));

	return ip_addr >= net_lower && ip_addr <= net_upper;
}

/**
 * @param ip
 * @return
 */
bool IpUtil::IsIpInPrivateRfc1918(const std::string &ip)
{
	return (
		IpUtil::IsIpInRange(ip, "10.0.0.0", "255.0.0.0") ||
		IpUtil::IsIpInRange(ip, "172.16.0.0", "255.240.0.0") ||
		IpUtil::IsIpInRange(ip, "192.168.0.0", "255.255.0.0")
	);
}
