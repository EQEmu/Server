#ifndef __OPENEQ_THREED_BASE__
#define __OPENEQ_THREED_BASE__

#pragma pack(1)

#include "archive.hpp"

struct Light {
  float x, y, z;
  float r, g, b;
  float rad;
} typedef Light;

struct Vertex {
  float x, y, z;
  float i, j, k;
  float u, v;

  int bone;
} typedef Vertex;


struct Polygon {
  int flags;

  int v1, v2, v3;

  int tex;
} typedef Polygon;


struct Texture {
  Archive *archive;
  char frame_count;
  char current_frame;
  uint32 *tex;
  char **filenames;

  char flags;
} typedef Texture;

class Model {
public:
  Model() {}
  ~Model() {}

  Vertex **verts;
  Polygon **polys;
  Texture **tex;

  int vert_count, poly_count, tex_count;

  char *name;
};


class Placeable {
public:
  float x, y, z;
  float rx, ry, rz;
  float scale[3];

  int model;

  char rendered;

  Vertex bounds[8];
  void CalculateBoundaries(Model *model);
};

#pragma pack()

#endif
