/*	EQEMu: Everquest Server Emulator
	Copyright (C) EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 04111-1307 USA
*/

#pragma once

#include "shared_memory_handle.h"

#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/unordered_map.hpp>

namespace eq
{
	namespace shared
	{
		template<typename Ky, typename Ty>
		using unordered_map = boost::unordered_map<int,
			Ty,
			boost::hash<Ky>,
			std::equal_to<Ky>,
			boost::interprocess::allocator<std::pair<const Ky, Ty>, boost::interprocess::managed_mapped_file::segment_manager>>;

		template<typename Ky, typename Ty>
		using map = boost::interprocess::map<Ky,
			Ty,
			std::less<Ky>,
			boost::interprocess::allocator<std::pair<const Ky, Ty>, boost::interprocess::managed_mapped_file::segment_manager>>;

		template<typename Ky, typename Ty>
		using flat_map = boost::interprocess::flat_map<Ky,
			Ty,
			std::less<Ky>,
			boost::interprocess::allocator<std::pair<const Ky, Ty>, boost::interprocess::managed_mapped_file::segment_manager>>;

		namespace detail
		{
			template<typename Ky, typename Ty>
			struct type_factory<unordered_map<Ky, Ty>>
			{
				unordered_map<Ky, Ty>* create(const std::string& key, boost::interprocess::managed_mapped_file* mmf) {
					return mmf->find_or_construct<unordered_map<Ky, Ty>>(key.c_str())(3, boost::hash<Ky>(), std::equal_to<Ky>(), mmf->get_allocator<void>());
				}
			};

			template<typename Ky, typename Ty>
			struct type_factory<map<Ky, Ty>>
			{
				map<Ky, Ty>* create(const std::string& key, boost::interprocess::managed_mapped_file* mmf) {
					return mmf->find_or_construct<map<Ky, Ty>>(key.c_str())(std::less<Ky>(), mmf->get_allocator<void>());
				}
			};

			template<typename Ky, typename Ty>
			struct type_factory<flat_map<Ky, Ty>>
			{
				flat_map<Ky, Ty>* create(const std::string& key, boost::interprocess::managed_mapped_file* mmf) {
					return mmf->find_or_construct<flat_map<Ky, Ty>>(key.c_str())(std::less<Ky>(), mmf->get_allocator<void>());
				}
			};
		}
	}
}
