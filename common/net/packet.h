#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>
#include <cstring>
#include "../util/memory_stream.h"
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

namespace EQ {
	namespace Net {
		class Packet
		{
		public:
			Packet() : m_stream(std::ios::out | std::ios::binary) { }
			virtual ~Packet() { }

			virtual const void *Data() const = 0;
			virtual void *Data() = 0;
			virtual uint64_t Length() const = 0;
			virtual uint64_t Length() = 0;
			virtual bool Clear() = 0;
			virtual bool Resize(uint64_t new_size) = 0;
			virtual void Reserve(uint64_t new_size) = 0;

			template<typename T>
			T GetSerialize(uint64_t offset) const
			{		
				T ret;
				Util::MemoryStreamReader reader(((char*)Data() + offset), (size_t)Length());
				cereal::BinaryInputArchive input(reader);
				input(ret);
				return ret;
			}

			template<typename T>
			void PutSerialize(uint64_t offset, const T &value) {
				m_stream.clear();
				cereal::BinaryOutputArchive output(m_stream);
				output(value);
			
				auto str = m_stream.str();
				if (Length() < offset + str.length()) {
					if (!Resize(offset + str.length())) {
						throw std::out_of_range("Packet::PutSerialize(), could not resize packet and would of written past the end.");
					}
				}
			
				memcpy((char*)Data() + offset, &str[0], str.length());
			}

			void PutInt8(uint64_t offset, int8_t value);
			void PutInt16(uint64_t offset, int16_t value);
			void PutInt32(uint64_t offset, int32_t value);
			void PutInt64(uint64_t offset, int64_t value);
			void PutUInt8(uint64_t offset, uint8_t value);
			void PutUInt16(uint64_t offset, uint16_t value);
			void PutUInt32(uint64_t offset, uint32_t value);
			void PutUInt64(uint64_t offset, uint64_t value);
			void PutFloat(uint64_t offset, float value);
			void PutDouble(uint64_t offset, double value);
			void PutString(uint64_t offset, const std::string &str);
			void PutCString(uint64_t offset, const char *str);
			void PutPacket(uint64_t offset, const Packet &p);
			void PutData(uint64_t offset, void *data, uint64_t length);
			
			int8_t GetInt8(uint64_t offset) const;
			int16_t GetInt16(uint64_t offset) const;
			int32_t GetInt32(uint64_t offset) const;
			int64_t GetInt64(uint64_t offset) const;
			uint8_t GetUInt8(uint64_t offset) const;
			uint16_t GetUInt16(uint64_t offset) const;
			uint32_t GetUInt32(uint64_t offset) const;
			uint64_t GetUInt64(uint64_t offset) const;
			float GetFloat(uint64_t offset) const;
			double GetDouble(uint64_t offset) const;
			std::string GetString(uint64_t offset, uint64_t length) const;
			std::string GetCString(uint64_t offset) const;
			
			std::string ToString() const;
			std::string ToString(uint64_t line_length) const;
		protected:
			std::stringstream m_stream;
		};

		class StaticPacket : public Packet
		{
		public:
			StaticPacket(void *data, uint64_t size) { m_data = data; m_data_length = size; m_max_data_length = size; }
			virtual ~StaticPacket() { }
			StaticPacket(const StaticPacket &o) { m_data = o.m_data; m_data_length = o.m_data_length; }
			StaticPacket& operator=(const StaticPacket &o) { m_data = o.m_data; m_data_length = o.m_data_length; return *this; }
			StaticPacket(StaticPacket &&o) { m_data = o.m_data; m_data_length = o.m_data_length; }

			virtual const void *Data() const { return m_data; }
			virtual void *Data() { return m_data; }
			virtual uint64_t Length() const { return m_data_length; }
			virtual uint64_t Length() { return m_data_length; }
			virtual bool Clear() { return false; }
			virtual bool Resize(uint64_t new_size);
			virtual void Reserve(uint64_t new_size) { }

		protected:
			void *m_data;
			uint64_t m_data_length;
			uint64_t m_max_data_length;
		};

		class DynamicPacket : public Packet
		{
		public:
			DynamicPacket() { }
			virtual ~DynamicPacket() { }
			DynamicPacket(DynamicPacket &&o) { m_data = std::move(o.m_data); }
			DynamicPacket(const DynamicPacket &o) { m_data = o.m_data; }
			DynamicPacket& operator=(const DynamicPacket &o) { m_data = o.m_data; return *this; }

			virtual const void *Data() const { return &m_data[0]; }
			virtual void *Data() { return &m_data[0]; }
			virtual uint64_t Length() const { return m_data.size(); }
			virtual uint64_t Length() { return m_data.size(); }
			virtual bool Clear() { m_data.clear(); return true; }
			virtual bool Resize(uint64_t new_size) { m_data.resize((size_t)new_size, 0); return true; }
			virtual void Reserve(uint64_t new_size) { m_data.reserve((size_t)new_size); }
		protected:
			std::vector<char> m_data;
		};
	}
}
