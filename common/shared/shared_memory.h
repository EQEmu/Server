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

#include "../expected.h"
#include "shared_memory_error.h"
#include "shared_memory_handle.h"

#include <boost/interprocess/managed_mapped_file.hpp>
#include <memory>

namespace eq
{
	namespace shared
	{
		namespace detail {
			typedef boost::interprocess::managed_mapped_file::segment_manager segment_manager_t;
			typedef boost::interprocess::allocator<void, segment_manager_t> allocator;
		}

		using allocator = detail::allocator;

		class shared_memory
		{
		public:
			shared_memory(const std::string &path, size_t sz) {
				_path = path;
				_sz = sz;
			}

			~shared_memory() = default;

			template<typename Ty>
			expected<handle<Ty>, shared_memory_error> map(const std::string& key) {
				if (!_memory) {
					try {
						_memory.reset(new boost::interprocess::managed_mapped_file(boost::interprocess::open_or_create, _path.c_str(), _sz));
					}
					catch (boost::interprocess::interprocess_exception) {
						return eq::make_unexpected(shared_memory_error::error_mapping_memory);
					}
				}

				handle<Ty> inst;
				auto ir = inst.initialize(key, _memory.get());
				if (ir) {
					return eq::expected<handle<Ty>, shared_memory_error>(inst);
				}
				else {
					return make_unexpected(ir.error());
				}

				return expected<handle<Ty>, shared_memory_error>(inst);
			}

			template<typename Ty, typename... Args>
			Ty construct(Args&&... args) {
				return Ty(std::forward<Args>(args)..., _memory->get_allocator<void>());
			}
		private:
			std::unique_ptr<boost::interprocess::managed_mapped_file> _memory;
			std::string _path;
			size_t _sz;
		};
	}
}
