/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#ifndef COMMON_MEMORY_BUFFER_H
#define COMMON_MEMORY_BUFFER_H

#include "types.h"
#include <type_traits>
#include <string.h>
#include <string>

namespace EQEmu
{
	class MemoryBuffer
	{
	public:
		MemoryBuffer();
		MemoryBuffer(size_t sz);
		MemoryBuffer(const MemoryBuffer &other);
		MemoryBuffer(MemoryBuffer &&other);
		MemoryBuffer& operator=(const MemoryBuffer &other);
		MemoryBuffer& operator=(MemoryBuffer &&other);
		~MemoryBuffer();
		
		uchar& operator[](size_t pos);
		const uchar& operator[](size_t pos) const;

		template<typename T>
		operator T*() {
			return reinterpret_cast<T*>(buffer_);
		}

		template<typename T>
		operator T*() const {
			return reinterpret_cast<T*>(buffer_);
		}

		operator bool() { return buffer_ != nullptr; }
		operator bool() const { return buffer_ != nullptr; }

		bool Empty();
		bool Empty() const;
		size_t Size();
		size_t Size() const;
		size_t Capacity();
		size_t Capacity() const;
		
		void Resize(size_t sz);
		void Clear();
		void Zero();

		template<typename T>
		void Write(T val) {
			static_assert(std::is_pod<T>::value, "MemoryBuffer::Write<T>(T val) only works on pod and string types.");
			Write((const char*)&val, sizeof(T));
		}

		template<typename T>
		T Read() {
			static_assert(std::is_pod<T>::value, "MemoryBuffer::Read<T>() only works on pod and string types.");
			T temp;
			Read((uchar*)&temp, sizeof(T));
			return temp;
		}

		template<>
		void Write<std::string>(std::string val) {
			Write(val.c_str(), val.length());
			Write((uint8)0);
		}

		template<>
		void Write<const std::string&>(const std::string &val) {
			Write(val.c_str(), val.length());
			Write((uint8)0);
		}

		template<>
		void Write<const char*>(const char *val) {
			size_t len = strlen(val);
			Write(val, len);
			Write((uint8)0);
		}

		template<>
		std::string Read<std::string>() {
			std::string ret;
			size_t len = strlen((const char*)&buffer_[read_pos_]);
			ret.resize(len);
			memcpy(&ret[0], &buffer_[read_pos_], len);
			read_pos_ += len + 1;
			return ret;
		}
	
		void Write(const char *val, size_t len);
		void Read(uchar *buf, size_t len);
		void Read(char *str);

		inline size_t GetWritePosition() { return write_pos_; }
		inline void SetWritePosition(size_t wp) { write_pos_ = wp; }
		inline void WriteSkipBytes(size_t skip) { write_pos_ += skip; }
		inline size_t GetReadPosition() { return read_pos_; }
		inline void SetReadPosition(size_t wp) { read_pos_ = wp; }
		inline void ReadSkipBytes(size_t skip) { read_pos_ += skip; }
		
	private:
		uchar *buffer_;
		size_t size_;
		size_t capacity_;
		size_t write_pos_;
		size_t read_pos_;
	};

} // EQEmu

#endif