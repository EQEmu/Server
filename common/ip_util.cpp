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

#include <cstring>
#include <fmt/format.h>
#include <csignal>
#include <vector>
#include "ip_util.h"
#include "http/httplib.h"
#include "http/uri.h"
#include "eqemu_logsys.h"
#include "event/event_loop.h"
#include "net/dns.h"
#include "event/task_scheduler.h"

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

/**
 * Gets local address - pings google to inspect what interface was used locally
 * @return
 */
std::string IpUtil::GetLocalIPAddress()
{
	char               my_ip_address[16];
	unsigned int       my_port;
	struct sockaddr_in server_address{};
	struct sockaddr_in my_address{};
	int                sockfd;

	// Connect to server
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return "";
	}

	// Set server_addr
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("172.217.160.99");
	server_address.sin_port        = htons(80);

	// Connect to server
	if (connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		close(sockfd);
		return "";
	}

	// Get my ip address and port
	memset(&my_address, 0, sizeof(my_address));
	socklen_t len = sizeof(my_address);
	getsockname(sockfd, (struct sockaddr *) &my_address, &len);
	inet_ntop(AF_INET, &my_address.sin_addr, my_ip_address, sizeof(my_ip_address));
	my_port = ntohs(my_address.sin_port);

	return fmt::format("{}", my_ip_address);
}

/**
 * Gets public address
 * Uses various websites as options to return raw public IP back to the client
 * @return
 */
std::string IpUtil::GetPublicIPAddress()
{
	std::vector<std::string> endpoints = {
		"http://ifconfig.me",
		"http://api.ipify.org",
		"http://ipinfo.io/ip",
		"http://ipecho.net/plain",
	};

	for (auto &s: endpoints) {
		// http get request
		uri u(s);

		httplib::Client r(
			fmt::format(
				"{}://{}",
				u.get_scheme(),
				u.get_host()
			).c_str()
		);

		httplib::Headers headers = {
			{"Content-type", "text/plain; charset=utf-8"},
			{"User-Agent",   "curl/7.81.0"}
		};

		r.set_connection_timeout(1, 0);
		r.set_read_timeout(1, 0);
		r.set_write_timeout(1, 0);

		if (auto res = r.Get(fmt::format("/{}", u.get_path()).c_str(), headers)) {
			if (res->status == 200) {
				if (res->body.find('.') != std::string::npos) {
					return res->body;
				}
			}
		}
	}

	return {};
}

std::string IpUtil::DNSLookupSync(const std::string &addr, int port)
{
	auto task_runner = new EQ::Event::TaskScheduler();
	auto res         = task_runner->Enqueue(
		[&]() -> std::string {
			bool        running = true;
			std::string ret;

			EQ::Net::DNSLookup(
				addr, port, false, [&](const std::string &addr) {
					ret = addr;
					if (addr.empty()) {
						ret     = "";
						running = false;
					}

					return ret;
				}
			);

			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

			auto &loop = EQ::EventLoop::Get();
			while (running) {
				if (!ret.empty()) {
					running = false;
				}

				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 1500) {
					LogInfo(
						"Deadline exceeded [{}]",
						1500
					);
					running = false;
				}

				loop.Process();
			}

			return ret;
		}
	);

	std::string result = res.get();
	safe_delete(task_runner);

	return result;
}

bool IpUtil::IsIPAddress(const std::string &ip_address)
{
	struct sockaddr_in sa{};
	int                result = inet_pton(AF_INET, ip_address.c_str(), &(sa.sin_addr));
	return result != 0;
}


