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

struct ter_triangle {
  long v1, v2, v3;
  long group;
  long unk;
} typedef ter_triangle;

struct ter_vertex_v3 {
  float x, y, z;
  float i, j, k;
  float unk1, unk2, unk3;
  float u, v;
} typedef ter_vertex_v3;

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
