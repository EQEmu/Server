#ifndef __OPENEQ_DAT__
#define __OPENEQ_DAT__

#include "global.hpp"
#include "file_loader.hpp"

#pragma pack(1)

struct dat_header {
  unsigned long version, list_len, mat_count, vert_count, tri_count;
} typedef dat_header;

struct dat_vertex {
  float x, y, z;
  float i, j, k;
  float u, v;
} typedef dat_vertex;

struct dat_vertexV3 {
  float x, y, z;
  float i, j, k;
  long unk1;
  float unk2, unk3;
  float u, v;
} typedef dat_vertexV3;

struct dat_triangle {
  long v1, v2, v3;
  long group;
  long unk;
} typedef dat_triangle;

struct dat_object {
  long index;
  long name_offset, another_name_offset;
  long property_count;
} typedef dat_object;

struct dat_property {
  long name_offset, type, value;
} typedef dat_property;

struct dat_material {
  char *name;
  char *basetex;
  char var_count;
  char **var_names;
  char **var_vals;
} typedef dat_material;

#pragma pack()

class DATLoader : public FileLoader {
public:
  DATLoader();
  ~DATLoader();
  virtual int Open(char *base_path, char *zone_name, Archive *archive);
  virtual int Close();
};

#endif
