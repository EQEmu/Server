#ifndef __AZONE_COMMON_COMPRESSION_H
#define __AZONE_COMMON_COMPRESSION_H

#include <string>
#include <zlib.h>

void decompress(const char* in, size_t in_len, char* out, size_t out_len);

#endif
