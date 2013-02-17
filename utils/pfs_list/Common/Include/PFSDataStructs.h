#ifndef __AZONE_COMMON_PFSDATASTRUCTS_H
#define __AZONE_COMMON_PFSDATASTRUCTS_H

#include <stdint.h>

#pragma pack(1)

struct PFSHeader {
  uint32_t offset;
  char magic[4];
  uint32_t unknown;
};

struct PFSDirectoryHeader {
  uint32_t count;
};

struct PFSDirectory {
  uint32_t crc, offset, size;
};

struct PFSDataBlock {
  uint32_t deflate_length, inflate_length;
};

struct PFSFilenameHeader {
  uint32_t filename_count;
};

struct PFSFilenameEntry {
  uint32_t filename_length;
};

#pragma pack()

#endif
