#ifndef __OPENEQ_OCTREE__
#define __OPENEQ_OCTREE__

#ifdef WIN32
#define Polygon Polygon_win32
#include <windows.h>
#undef Polygon
#endif

#include "3d_base.hpp"

class Octree_Node {
public:
  Octree_Node();
  Octree_Node(int poly_count, Polygon **polys, Vertex **verts, int plac_count, Placeable **plac);
  Octree_Node(int poly_count, Polygon **polys, Vertex **verts, int plac_count, Placeable **plac, float min[3], float max[3]);
  ~Octree_Node();
  void FindMinMax();
  void Split(char level);

  float min[3], max[3];
  float x, y, z, size, sizes[3];

  int poly_count;
  Vertex **verts;
  Polygon **polys;

  int plac_count;
  Placeable **plac;

  GLuint li;

  Octree_Node *nodes[8];
};

#define in_box(obj, x, y, z) \
( \
  (x) >= (obj)->min[0] && (x) <= (obj)->max[0] && \
  (y) >= (obj)->min[1] && (y) <= (obj)->max[1] &&\
  (z) >= (obj)->min[2] && (z) <= (obj)->max[2] \
)

#endif
