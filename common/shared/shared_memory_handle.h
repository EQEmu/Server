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

#include "shared_memory_error.h"
#include "../expected.h"

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <type_traits>

namespace eq
{
	namespace shared
	{
		namespace detail
		{
			template<typename Ty>
			struct type_factory {
				static_assert(std::is_trivially_constructible<Ty>::value, 
					"Non-specialized types must be trivially constructible, consider creating a type_factory in eq::shared::detail if you want to use this type.");
				Ty*	create(const std::string& key, boost::interprocess::managed_mapped_file* mmf) {
					return mmf->find_or_construct<Ty>(key.c_str())();
				}
			};
		}

		template<typename Ty>
		class handle {
		public:
			handle() = default;
			~handle() = default;

			expected<void, shared_memory_error> initialize(const std::string& key, boost::interprocess::managed_mapped_file* mmf) {
				try {
					_mmf = mmf;
					detail::type_factory<Ty> fac;
					_ptr = fac.create(key, mmf);
					return expected<void, shared_memory_error>();
				}
				catch (boost::interprocess::interprocess_exception) {
					return make_unexpected(shared_memory_error::error_mapping_memory);
				}
			}

			operator Ty& () {
				return *_ptr;
			}
		private:
			Ty* _ptr;
			boost::interprocess::managed_mapped_file* _mmf;
		};
	}
}
