#ifndef __OPENEQ_ZON__
#define __OPENEQ_ZON__

#include "ter.hpp"

/* Big thanks to jbb on the ZON stuff! */

#pragma pack(1)

struct zon_header {
  char magic[4]; // Constant at EQGZ
  long version; // Constant at 2
  long list_len; // Length of the list to follow.
  long NumberOfModels;
  long obj_count; // Placeable object count.
  long unk[2]; // Unknown.
} typedef zon_header;

struct zon_placeable {
  long id;
  long loc;
  float x, y, z;
  float rx, ry, rz;
  float scale;
} typedef zon_placeable;

struct zon_object {
   long offset;
   long id;
} typedef zon_object;

#pragma pack()

class ZonLoader : public FileLoader {
public:
  ZonLoader();
  ~ZonLoader();

  virtual int Open(char *base_path, char *zone_name, Archive *archive);
  virtual int Close();

private:
  TERLoader terloader;
  TERLoader *model_loaders;
};

#endif
