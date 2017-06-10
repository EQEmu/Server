#pragma once

#include <stdint.h>

namespace EQ
{
	int Crc32(const void *data, int size);
	int Crc32(const void *data, int size, int key);
}