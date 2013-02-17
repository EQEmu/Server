#ifndef __OPENEQ_TER__
#define __OPENEQ_TER__

#include "global.hpp"
#include "file_loader.hpp"

#pragma pack(1)

struct ter_header {
  char magic[4];
  unsigned long version, list_len, mat_count, vert_count, tri_count;
} typedef ter_header;

struct ter_vertex {
  float x, y, z;
  float i, j, k;
  float u, v;
} typedef ter_vertex;

struct ter_vertexV3 {
  float x, y, z;
  float i, j, k;
  long unk1;
  float unk2, unk3;
  float u, v;
} typedef ter_vertexV3;

struct ter_triangle {
  long v1, v2, v3;
  long group;
  long unk;
} typedef ter_triangle;

struct ter_object {
  long index;
  long name_offset, another_name_offset;
  long property_count;
} typedef ter_object;

struct ter_property {
  long name_offset, type, value;
} typedef ter_property;

struct material {
  char *name;
  char *basetex;
  char var_count;
  char **var_names;
  char **var_vals;
} typedef material;

#pragma pack()

class TERLoader : public FileLoader {
public:
  TERLoader();
  ~TERLoader();
  virtual int Open(char *base_path, char *zone_name, Archive *archive);
  virtual int Close();
};

#endif
