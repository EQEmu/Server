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
			Packet() = default;

			virtual const void *Data() const = 0;
			virtual void *Data() = 0;
			virtual size_t Length() const = 0;
			virtual size_t Length() = 0;
			virtual bool Clear() = 0;
			virtual bool Resize(size_t new_size) = 0;
			virtual void Reserve(size_t new_size) = 0;

			std::string ToString() const;
			std::string ToString(size_t line_length) const;

			void SetWritePos(size_t offset) { _wpos = offset; }
			void SetReadPos(size_t offset) { _rpos = offset; }

			//Position Independent Output Interface
			void PutInt8(size_t offset, int8_t value) {
				if (Length() < offset + sizeof(int8_t)) {
					if (!Resize(offset + sizeof(int8_t))) {
						throw std::out_of_range("Packet::PutInt8(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<int8_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(int8_t);
			}

			void PutInt16(size_t offset, int16_t value) {
				if (Length() < offset + sizeof(int16_t)) {
					if (!Resize(offset + sizeof(int16_t))) {
						throw std::out_of_range("Packet::PutInt16(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<int16_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(int16_t);
			}

			void PutInt32(size_t offset, int32_t value) {
				if (Length() < offset + sizeof(int32_t)) {
					if (!Resize(offset + sizeof(int32_t))) {
						throw std::out_of_range("Packet::PutInt32(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<int32_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(int32_t);
			}

			void PutInt64(size_t offset, int64_t value) {
				if (Length() < offset + sizeof(int64_t)) {
					if (!Resize(offset + sizeof(int64_t))) {
						throw std::out_of_range("Packet::PutInt64(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<int64_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(int64_t);
			}

			void PutUInt8(size_t offset, uint8_t value) {
				if (Length() < offset + sizeof(uint8_t)) {
					if (!Resize(offset + sizeof(uint8_t))) {
						throw std::out_of_range("Packet::PutUInt8(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<uint8_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(uint8_t);
			}

			void PutUInt16(size_t offset, uint16_t value) {
				if (Length() < offset + sizeof(uint16_t)) {
					if (!Resize(offset + sizeof(uint16_t))) {
						throw std::out_of_range("Packet::PutUInt16(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<uint16_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(uint16_t);
			}

			void PutUInt32(size_t offset, uint32_t value) {
				if (Length() < offset + sizeof(uint32_t)) {
					if (!Resize(offset + sizeof(uint32_t))) {
						throw std::out_of_range("Packet::PutUInt32(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<uint32_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(uint32_t);
			}

			void PutUInt64(size_t offset, uint64_t value) {
				if (Length() < offset + sizeof(uint64_t)) {
					if (!Resize(offset + sizeof(uint64_t))) {
						throw std::out_of_range("Packet::PutUInt64(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<uint64_t*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(uint64_t);
			}

			void PutFloat(size_t offset, float value) {
				if (Length() < offset + sizeof(float)) {
					if (!Resize(offset + sizeof(float))) {
						throw std::out_of_range("Packet::PutFloat(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<float*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(float);
			}

			void PutDouble(size_t offset, double value) {
				if (Length() < offset + sizeof(double)) {
					if (!Resize(offset + sizeof(double))) {
						throw std::out_of_range("Packet::PutDouble(), could not resize packet and would of written past the end.");
					}
				}

				*reinterpret_cast<double*>(static_cast<char*>(Data()) + offset) = value;
				_wpos = offset + sizeof(double);
			}

			void PutString(size_t offset, const std::string &str) {
				if (Length() < offset + str.length()) {
					if (!Resize(offset + str.length())) {
						throw std::out_of_range("Packet::PutString(), could not resize packet and would of written past the end.");
					}
				}

				std::memcpy((static_cast<char*>(Data()) + offset), str.c_str(), str.length());
				_wpos = offset + str.length();
			}

			void PutCString(size_t offset, const char *str) {
				size_t sz = strlen(str);
				if (Length() < offset + sz + sizeof(int8_t)) {
					if (!Resize(offset + sz + sizeof(int8_t))) {
						throw std::out_of_range("Packet::PutCString(), could not resize packet and would of written past the end.");
					}
				}

				std::memcpy((static_cast<char*>(Data()) + offset), str, sz);
				*(static_cast<char*>(Data()) + offset + sz) = 0;
				_wpos = offset + sz + sizeof(int8_t);
			}

			void PutPacket(size_t offset, const Packet &p) {
				if (p.Length() == 0) {
					return;
				}

				if (Length() < offset + p.Length()) {
					if (!Resize(offset + p.Length())) {
						throw std::out_of_range("Packet::PutPacket(), could not resize packet and would of written past the end.");
					}
				}

				std::memcpy((static_cast<char*>(Data()) + offset), p.Data(), p.Length());
				_wpos = offset + p.Length();
			}

			void PutData(size_t offset, void *data, size_t length) {
				if (length == 0) {
					return;
				}

				if (Length() < offset + length) {
					if (!Resize(offset + length)) {
						throw std::out_of_range("Packet::PutData(), could not resize packet and would of written past the end.");
					}
				}

				std::memcpy((static_cast<char*>(Data()) + offset), data, length);
				_wpos = offset + length;
			}

			//Position Dependent Output Interface
			void PutInt8(int8_t value) {
				PutInt8(_wpos, value);
			}

			void PutInt16(int16_t value) {
				PutInt16(_wpos, value);
			}

			void PutInt32(int32_t value) {
				PutInt32(_wpos, value);
			}

			void PutInt64(int64_t value) {
				PutInt64(_wpos, value);
			}

			void PutUInt8(uint8_t value) {
				PutUInt8(_wpos, value);
			}

			void PutUInt16(uint16_t value) {
				PutUInt16(_wpos, value);
			}

			void PutUInt32(uint32_t value) {
				PutUInt32(_wpos, value);
			}

			void PutUInt64(uint64_t value) {
				PutUInt64(_wpos, value);
			}

			void PutFloat(float value) {
				PutFloat(_wpos, value);
			}

			void PutDouble(double value) {
				PutDouble(_wpos, value);
			}

			void PutString(const std::string &str) {
				PutString(_wpos, str);
			}

			void PutCString(const char *str) {
				PutCString(_wpos, str);
			}

			void PutPacket(const Packet &p) {
				PutPacket(_wpos, p);
			}

			void PutData(void *data, size_t length) {
				PutData(_wpos, data, length);
			}

			//Position Independent Input Interface
			int8_t GetInt8(size_t offset) const {
				if (Length() < offset + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const int8_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			int16_t GetInt16(size_t offset) const {
				if (Length() < offset + sizeof(int16_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const int16_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			int32_t GetInt32(size_t offset) const {
				if (Length() < offset + sizeof(int32_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const int32_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			int64_t GetInt64(size_t offset) const {
				if (Length() < offset + sizeof(int64_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const int64_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			uint8_t GetUInt8(size_t offset) const {
				if (Length() < offset + sizeof(uint8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const uint8_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			uint16_t GetUInt16(size_t offset) const {
				if (Length() < offset + sizeof(uint16_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const uint16_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			uint32_t GetUInt32(size_t offset) const {
				if (Length() < offset + sizeof(uint32_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const uint32_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			uint64_t GetUInt64(size_t offset) const {
				if (Length() < offset + sizeof(uint64_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const uint64_t*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			float GetFloat(size_t offset) const {
				if (Length() < offset + sizeof(float)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const float*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			double GetDouble(size_t offset) const {
				if (Length() < offset + sizeof(double)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const double*>(reinterpret_cast<const int8_t*>(Data()) + offset);
			}

			std::string GetString(size_t offset, size_t length) const {
				if (Length() < offset + length) {
					throw std::out_of_range("Packet read out of range.");
				}

				return std::string(reinterpret_cast<const int8_t*>(Data()) + offset, reinterpret_cast<const int8_t*>(Data()) + offset + length);
			}

			std::string GetCString(size_t offset) const {
				if (Length() < offset) {
					throw std::out_of_range("Packet read out of range.");
				}

				auto sz = strlen(static_cast<const char*>(Data()));

				if (Length() < offset + sz + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				const char *str = reinterpret_cast<const char*>(reinterpret_cast<const int8_t*>(Data()) + offset);
				return std::string(str);
			}

			//Position Dependent Input Interface
			int8_t GetInt8() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(int8_t);
				return *reinterpret_cast<int8_t*>(static_cast<char*>(Data()) + offset);
			}

			int16_t GetInt16() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(int16_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(int16_t);
				return *reinterpret_cast<int16_t*>(static_cast<char*>(Data()) + offset);
			}

			int32_t GetInt32() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(int32_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(int32_t);
				return *reinterpret_cast<int32_t*>(static_cast<char*>(Data()) + offset);
			}

			int64_t GetInt64() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(int64_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(int64_t);
				return *reinterpret_cast<int64_t*>(static_cast<char*>(Data()) + offset);
			}

			uint8_t GetUInt8() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(uint8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(uint8_t);
				return *reinterpret_cast<uint8_t*>(static_cast<char*>(Data()) + offset);
			}

			uint16_t GetUInt16() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(uint16_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(uint16_t);
				return *reinterpret_cast<uint16_t*>(static_cast<char*>(Data()) + offset);
			}

			uint32_t GetUInt32() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(uint32_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(uint32_t);
				return *reinterpret_cast<uint32_t*>(static_cast<char*>(Data()) + offset);
			}

			uint64_t GetUInt64() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(uint64_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(uint64_t);
				return *reinterpret_cast<uint64_t*>(static_cast<char*>(Data()) + offset);
			}

			float GetFloat() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(float)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(float);
				return *reinterpret_cast<float*>(static_cast<char*>(Data()) + offset);
			}

			double GetDouble() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(double)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sizeof(double);
				return *reinterpret_cast<double*>(static_cast<char*>(Data()) + offset);
			}

			std::string GetString(size_t length) {
				auto offset = _rpos;

				if (Length() < offset + length) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + length;
				return std::string(static_cast<char*>(Data()) + offset, static_cast<char*>(Data()) + offset + length);
			}

			std::string GetCString() {
				auto offset = _rpos;

				if (Length() < offset + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				auto sz = strlen(static_cast<const char*>(Data()));

				if (Length() < offset + sz + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				_rpos = offset + sz + sizeof(int8_t);
				char *str = (static_cast<char*>(Data()) + offset);
				return std::string(str);
			}

			int8_t& operator[](size_t offset) {
				if (Length() < offset + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<int8_t*>(static_cast<char*>(Data()) + offset);
			}

			const int8_t& operator[](size_t offset) const {
				if (Length() < offset + sizeof(int8_t)) {
					throw std::out_of_range("Packet read out of range.");
				}

				return *reinterpret_cast<const int8_t*>(static_cast<const char*>(Data()) + offset);
			}

			//Stream Output Interface
			Packet &operator<<(int8_t v) {
				PutInt8(v);
				return *this;
			}

			Packet &operator<<(int16_t v) {
				PutInt16(v);
				return *this;
			}

			Packet &operator<<(int32_t v) {
				PutInt32(v);
				return *this;
			}

			Packet &operator<<(int64_t v) {
				PutInt64(v);
				return *this;
			}

			Packet &operator<<(uint8_t v) {
				PutUInt8(v);
				return *this;
			}

			Packet &operator<<(uint16_t v) {
				PutUInt16(v);
				return *this;
			}

			Packet &operator<<(uint32_t v) {
				PutUInt32(v);
				return *this;
			}

			Packet &operator<<(uint64_t v) {
				PutUInt64(v);
				return *this;
			}

			Packet &operator<<(float v) {
				PutFloat(v);
				return *this;
			}

			Packet &operator<<(double v) {
				PutDouble(v);
				return *this;
			}

			Packet &operator<<(const std::string &v) {
				PutString(v.data());
				return *this;
			}

			Packet &operator<<(const char *v) {
				PutCString(v);
				return *this;
			}

			Packet &operator<<(const Packet &v) {
				PutPacket(v);
				return *this;
			}

			//Stream Input Interface
			Packet &operator>>(int8_t &v) {
				v = GetInt8();
				return *this;
			}

			Packet &operator>>(int16_t &v) {
				v = GetInt16();
				return *this;
			}

			Packet &operator>>(int32_t &v) {
				v = GetInt32();
				return *this;
			}

			Packet &operator>>(int64_t &v) {
				v = GetInt64();
				return *this;
			}

			Packet &operator>>(uint8_t &v) {
				v = GetUInt8();
				return *this;
			}

			Packet &operator>>(uint16_t &v) {
				v = GetUInt16();
				return *this;
			}

			Packet &operator>>(uint32_t &v) {
				v = GetUInt32();
				return *this;
			}

			Packet &operator>>(uint64_t &v) {
				v = GetUInt64();
				return *this;
			}

			Packet &operator>>(float &v) {
				v = GetFloat();
				return *this;
			}

			Packet &operator>>(double &v) {
				v = GetDouble();
				return *this;
			}

			Packet &operator>>(std::string &v) {
				v = GetCString();
				return *this;
			}

			// Seriliazation
			template<typename T>
			T GetSerialize(size_t offset) const
			{
				T ret;
				Util::MemoryStreamReader reader(((char*)Data() + offset), Length());
				cereal::BinaryInputArchive input(reader);
				input(ret);
				return ret;
			}

			template<typename T>
			void PutSerialize(size_t offset, const T &value) {
				std::stringstream stream;

				cereal::BinaryOutputArchive output(stream);
				output(value);

				auto str = stream.str();
				if (Length() < offset + str.length()) {
					if (!Resize(offset + str.length())) {
						throw std::out_of_range("Packet::PutSerialize(), could not resize packet and would of written past the end.");
					}
				}

				memcpy((char*)Data() + offset, &str[0], str.length());
			}
		protected:
			size_t _rpos{ 0 };
			size_t _wpos{ 0 };
		};

		class StaticPacket : public Packet
		{
		public:
			StaticPacket(void *data, size_t len) : _data(data), _len(len), _max_len(len) {
			}

			StaticPacket(const StaticPacket &o) : _data(o._data), _len(o._len), _max_len(o._max_len) {
				_rpos = o._rpos;
				_wpos = o._wpos;
			}

			StaticPacket& operator=(const StaticPacket& o) {
				_data = o._data;
				_len = o._len;
				_max_len = o._max_len;
				_rpos = o._rpos;
				_wpos = o._wpos;
				return *this;
			}

			StaticPacket(StaticPacket &&o) noexcept : _data(o._data), _len(o._len), _max_len(o._max_len) {
				_rpos = o._rpos;
				_wpos = o._wpos;
			}

			const void *Data() const override { return _data; }
			void *Data() override { return _data; }
			size_t Length() const override { return _len; }
			size_t Length() override { return _len; }
			bool Clear() override { return false; }
			bool Resize(size_t new_size) override;
			void Reserve(size_t new_size) override { }

		protected:
			void *_data;
			size_t _len;
			size_t _max_len;
		};

		class DynamicPacket : public Packet
		{
		public:
			const static size_t DefaultSize = 4096;

			DynamicPacket() {
				_data.reserve(DefaultSize);
			}

			DynamicPacket(size_t size) {
				_data.reserve(size);
			}

			DynamicPacket(const DynamicPacket& o) : _data(o._data) {
				_rpos = o._rpos;
				_wpos = o._wpos;
			}

			DynamicPacket& operator=(const DynamicPacket& o) {
				_data = o._data;
				_rpos = o._rpos;
				_wpos = o._wpos;
			}

			DynamicPacket(DynamicPacket &&o) noexcept : _data(o.MoveData()) {
				_rpos = o._rpos;
				_wpos = o._wpos;
			}

			std::vector<int8_t>&& MoveData() {
				_rpos = 0;
				_wpos = 0;
				return std::move(_data);
			}

			const void *Data() const override { return _data.data(); }
			void *Data() override { return _data.data(); }
			size_t Length() const override { return _data.size(); }
			size_t Length() override { return _data.size(); }
			bool Clear() override { _data.clear(); return true; }
			bool Resize(size_t new_size) override { _data.resize(new_size); return true; }
			void Reserve(size_t new_size) override { _data.reserve(new_size); }

		protected:
			std::vector<int8_t> _data;
		};
	}  // namespace Net
}  // namespace EQ
