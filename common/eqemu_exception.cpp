/*	EQEmu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEmu Development Team (http://www.eqemulator.net)

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

#include "eqemu_exception.h"

namespace EQ
{
	Exception::Exception(const char* name, const std::string& description, const char* file, long line)
		: line_(line),
		file_(file),
		desc_(description),
		name_(name)
	{
	}

	Exception::Exception(const Exception& e)
		: line_(e.line_),
		file_(e.file_),
		desc_(e.desc_),
		name_(e.name_)
	{
	}

#ifdef EQEMU_RVALUE_MOVE
	Exception::Exception(const Exception&& e)
		: line_(e.line_),
		file_(e.file_),
		desc_(e.desc_),
		name_(e.name_)
	{
	}
#endif

	void Exception::operator=(const Exception& e) {
		line_ = e.line_;
		file_ = e.file_;
		desc_ = e.desc_;
		name_ = e.name_;
	}

	const std::string& Exception::full_description() const {
		if(full_desc_.empty()) {
			std::stringstream ss;
			ss << "EQEmu Exception (" << name_ << ") in " << file_;
			ss << " at line (" << line_ << "): " << desc_;
			full_desc_ = ss.str();
		}
		return full_desc_;
	}
} // EQEmu
