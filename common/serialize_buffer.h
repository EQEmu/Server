#ifndef SERIALIZE_BUFFER_H
#define SERIALIZE_BUFFER_H

#include <cstring>
#include <cassert>
#include <cstdint>
#include <string>

class SerializeBuffer
{
public:
	SerializeBuffer() : m_buffer(nullptr), m_capacity(0), m_pos(0) {}

	explicit SerializeBuffer(size_t size) : m_capacity(size), m_pos(0)
	{
		m_buffer = new unsigned char[size];
		memset(m_buffer, 0, size);
	}

	SerializeBuffer(const SerializeBuffer& rhs)
		: m_buffer(new unsigned char[rhs.m_capacity]), m_capacity(rhs.m_capacity), m_pos(rhs.m_pos)
	{
		memcpy(m_buffer, rhs.m_buffer, rhs.m_capacity);
	}

	SerializeBuffer& operator=(const SerializeBuffer& rhs)
	{
		if (this != &rhs) {
			delete[] m_buffer;
			m_buffer = new unsigned char[rhs.m_capacity];
			m_capacity = rhs.m_capacity;
			m_pos = rhs.m_pos;
			memcpy(m_buffer, rhs.m_buffer, m_capacity);
		}
		return *this;
	}

	SerializeBuffer(SerializeBuffer&& rhs) noexcept : m_buffer(rhs.m_buffer), m_capacity(rhs.m_capacity), m_pos(rhs.m_pos)
	{
		rhs.m_buffer = nullptr;
		rhs.m_capacity = 0;
		rhs.m_pos = 0;
	}

	SerializeBuffer& operator=(SerializeBuffer&& rhs) noexcept
	{
		if (this != &rhs) {
			delete[] m_buffer;

			m_buffer = rhs.m_buffer;
			m_capacity = rhs.m_capacity;
			m_pos = rhs.m_pos;

			rhs.m_buffer = nullptr;
			rhs.m_capacity = 0;
			rhs.m_pos = 0;
		}
		return *this;
	}

	~SerializeBuffer() { delete[] m_buffer; }

	void WriteUInt8(uint8_t v)
	{
		WriteNumber(v);
	}

	void WriteUInt16(uint16_t v)
	{
		WriteNumber(v);
	}

	void WriteUInt32(uint32_t v)
	{
		WriteNumber(v);
	}

	void WriteUInt64(uint64_t v)
	{
		WriteNumber(v);
	}

	void WriteInt8(int8_t v)
	{
		WriteNumber(v);
	}

	void WriteInt16(int16_t v)
	{
		WriteNumber(v);
	}

	void WriteInt32(int32_t v)
	{
		WriteNumber(v);
	}

	void WriteInt64(int64_t v)
	{
		WriteNumber(v);
	}

	void WriteFloat(float v)
	{
		WriteNumber(v); 
	}

	void WriteDouble(double v)
	{
		WriteNumber(v);
	}

	void WriteString(const char* str)
	{
		assert(str != nullptr);
		auto len = std::char_traits<char>::length(str) + 1;
		if (m_pos + len > m_capacity)
			Grow(m_capacity + len);
		memcpy(m_buffer + m_pos, str, len);
		m_pos += len;
	}

	void WriteString(const std::string& str)
	{
		auto len = str.length() + 1;
		if (m_pos + len > m_capacity)
			Grow(m_capacity + len);
		memcpy(m_buffer + m_pos, str.c_str(), len);
		m_pos += len;
	}

	void WriteLengthString(uint32_t len, const char* str)
	{
		assert(str != nullptr);
		if (m_pos + len + sizeof(len) > m_capacity)
			Grow(m_capacity + len + sizeof(len));
		*reinterpret_cast<uint32_t*>(m_buffer + m_pos) = len; 
		m_pos += sizeof(len);
		memcpy(m_buffer + m_pos, str, len);
		m_pos += len;
	}

	void WriteLengthString(const std::string& str)
	{
		WriteLengthString(static_cast<uint32_t>(str.length()), str.c_str());
	}

	size_t size() const { return m_pos; }
	size_t length() const { return size(); }
	size_t capacity() const { return m_capacity; }
	const unsigned char* buffer() const { return m_buffer; }

	friend class BasePacket;

private:
	void Grow(size_t new_size);
	void Reset();
	unsigned char* m_buffer;
	size_t m_capacity;
	size_t m_pos;

	template <typename DATA_TYPE> void WriteNumber(DATA_TYPE v) {
		if (m_pos + sizeof(DATA_TYPE) > m_capacity)
			Grow(m_capacity + sizeof(DATA_TYPE));
		*reinterpret_cast<DATA_TYPE*>(m_buffer + m_pos) = v; 
		m_pos += sizeof(DATA_TYPE);
	}
};

#endif /* !SERIALIZE_BUFFER_H */
