#ifndef __OPENEQ_THREED__
#define __OPENEQ_THREED__

#include "3d_base.hpp"

class Octree_Node;

class Bone {
public:
  char *name;

  int bone1, bone2;

  float x, y, z;
  float e[4];
  float scale[3];

  int vert_count;
  int *verts;
};

class Zone_Model {
public:
  Zone_Model() {}
  ~Zone_Model() {}

  Vertex **verts;
  Polygon **polys;
  Octree_Node *octree;
  Texture **tex;

  int vert_count, poly_count, tex_count;
};

class MobModel {
public:
  MobModel() {}
  ~MobModel() {}

  Vertex **verts;
  Polygon **polys;
  Texture **tex;

  int vert_count, poly_count, tex_count;

  Bone *bones;

  int bone_count;
};

class Content_3D {
public:
  Content_3D() { this->mob_model_count = 0; this->mob_count = 0; }
  ~Content_3D() {}

  Model *GetModel(char *model_name);

  void GenerateOctree();

  void MergeTextures();
  void MergeTexturesLazy();

  Zone_Model *zone_model;

  Model **models;
  Placeable **placeable;

  int model_count, plac_count; // Count of placeable object models and count of instances

  MobModel **mob_models;
  Placeable **mob_locs;
  int mob_model_count, mob_count;
};

#endif
