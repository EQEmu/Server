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

#pragma once

#include "shared_memory_handle.h"

#include <boost/interprocess/containers/string.hpp>

namespace eq
{
	namespace shared
	{
		template<typename Ty>
		using basic_string = boost::interprocess::basic_string<Ty, std::char_traits<Ty>, boost::interprocess::allocator<Ty, boost::interprocess::managed_mapped_file::segment_manager>>;

		using string = basic_string<char>;

		namespace detail
		{
			template<typename Ty>
			struct type_factory<basic_string<Ty>>
			{
				list<Ty>* create(const std::string& key, boost::interprocess::managed_mapped_file* mmf) {
					return mmf->find_or_construct<list<Ty>>(key.c_str())(mmf->get_allocator<void>());
				}
			};
		}

		template<typename Ty>
		std::basic_string<Ty> to_std_string(const eq::shared::basic_string<Ty> &in) {
			return std::basic_string<Ty>(in.begin(), in.end());
		}
	}
}
