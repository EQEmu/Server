#ifndef __OPENEQ_WLD_STRUCTS__
#define __OPENEQ_WLD_STRUCTS__

#pragma pack(1)

struct struct_wld_header {
  long magic;
  long version;
  long fragmentCount;
  long header3;
  long header4;
  long stringHashSize;
  long header6;
} typedef struct_wld_header;

struct struct_wld_basic_frag {
  long size;
  long id;
  long nameRef;
} typedef struct_wld_basic_frag;


struct struct_Data15 {
  uint32 ref, flags, fragment1;
  float trans[3], rot[3];
  float scale[3];
  uint32 fragment2, flags2;
} typedef struct_Data15;

struct struct_Data36 {
  long flags;
  long fragment1;
  long fragment2;
  long fragment3;
  long fragment4;
  float centerX;
  float centerY;
  float centerZ;
  long params2[3]; // 48
  float maxDist;
  float minX;
  float minY;
  float minZ;
  float maxX;
  float maxY;
  float maxZ; // 24
  short int vertexCount;
  short int texCoordsCount;
  short int normalsCount;
  short int colorCount;
  short int PolygonsCount;
  short int size6;
  short int PolygonTexCount;
  short int vertexTexCount;
  short int size9;
  short int scale; // 20
} typedef struct_Data36;

struct struct_Data10 {
  long flags, size1, fragment;
} typedef struct_Data10;

struct struct_Data1B {
  uint32 flags, params1;
  uint32 params3b;
  float color[3];
} typedef struct_Data1B;

struct struct_Data21 {
  float normal[3], splitdistance;
  long region, node[2];
} typedef struct_Data21;

struct struct_Data22 {
  long flags, fragment1, size1, size2, params1, size3, size4, params2, size5, size6;
} typedef struct_Data22;

struct struct_Data28 {
  uint32 flags;
  float x, y, z, rad;
} typedef struct_Data28;

typedef struct struct_Data29 {
  long region_count;
  long *region_array;
  long strlen;
  char *str;
  int region_type; // We fill this in with -1 for unknown, 1 for water, 2 for lava
} struct_Data29;


struct struct_Data30 {
  uint32 flags, params1, params2;
  float params3[2];
} typedef struct_Data30;

typedef struct BSP_Node {
  long node_number;
  float normal[3], splitdistance;
  long region;
  int special;
  long left, right;
} BSP_Node;


struct Vert {
  signed short int x, y, z;
} typedef Vert;

struct TexCoordsNew {
  float tx, tz;
} typedef TexCoordsNew;

struct TexCoordsOld {
  signed short int tx, tz;
} typedef TexCoordsOld;

struct VertexNormal {
  signed char nx, ny, nz;
} typedef VertexNormal;

struct VertexColor {
  char color[4];
} typedef VertexColor;

struct Poly {
  short int flags, v1, v2, v3;
} typedef Poly;

struct TexMap {
  uint16 polycount;
  uint16 tex;
} typedef TexMap;

#pragma pack()

#endif
