#include "serialize_buffer.h"

void SerializeBuffer::Grow(size_t new_size)
{
	assert(new_size > m_capacity);
	auto new_buffer = new unsigned char[new_size * 2];
	memset(new_buffer, 0, new_size * 2);

	if (m_buffer)
		memcpy(new_buffer, m_buffer, m_capacity);
	m_capacity = new_size * 2;
	delete[] m_buffer;
	m_buffer = new_buffer;
}

void SerializeBuffer::Reset()
{
	delete[] m_buffer;
	m_buffer = nullptr;
	m_capacity = 0;
	m_pos = 0;
}

