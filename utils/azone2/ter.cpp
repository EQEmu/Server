
// This source is from OpenEQ by Daeken et al. Modified a bit by Derision, some bug fixes etc.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "file.hpp"
#include "ter.hpp"
//#define DEBUGTER

TERLoader::TERLoader() {
  this->buffer = nullptr;
  this->buf_len = -1;
  this->archive = nullptr;
  this->status = 0;
}

TERLoader::~TERLoader() {
  this->Close();
}

int TERLoader::Open(char *base_path, char *zone_name, Archive *archive) {

#ifdef DEBUGTER
  printf("TERLoader::Open %s, [%s]\n", base_path, zone_name);
  fflush(stdout);
#endif
  ter_header *thdr;
  ter_vertex *tver;
  ter_vertexV3 *tverV3;
  ter_triangle *ttri;
  uchar *ter_orig, *ter_tmp, *StartOfNameList;
  int model = 0;
  unsigned int i, j, mat_count = 0;

  Zone_Model *zm;

  material *mlist;

  uchar *buffer;
  int buf_len, bone_count;

  char *filename;

  if(zone_name[strlen(zone_name) - 4] == '.')
    filename = zone_name;
  else {
    filename = new char[strlen(zone_name) + 5];
    sprintf(filename, "%s.ter", zone_name);
  }

  if(!GetFile(&buffer, &buf_len, base_path, filename, archive)) {
    if(filename != zone_name)
      delete[] filename;
    return 0;
  }
  if(filename != zone_name)
    delete[] filename;

  thdr = (ter_header *) buffer;

  mlist = new material[thdr->mat_count];

  ter_orig = buffer;

  if(thdr->magic[0] != 'E' || thdr->magic[1] != 'Q' || thdr->magic[2] != 'G')
    return 0;

  if(thdr->magic[3] == 'M') {
    model = 1;
    buffer += sizeof(ter_header);
    bone_count = *((uint32 *) buffer);
    buffer += sizeof(uint32);
  }
  else if(thdr->magic[3] == 'T')
    buffer += sizeof(ter_header);
  else
    return 0;

  ter_tmp = buffer + thdr->list_len;

  if(sizeof(ter_header) + thdr->list_len + (thdr->vert_count * sizeof(ter_vertex)) + (thdr->tri_count * sizeof(ter_triangle)) > (unsigned int)buf_len)
    return 0;

  for(i = 0; i < thdr->mat_count; ++i) {
    mlist[i].name = nullptr;
    mlist[i].basetex = nullptr;
  }

  StartOfNameList = buffer;
  buffer = buffer + thdr->list_len;
#ifdef DEBUGTER
  printf("Offset is %X\n", buffer - ter_orig);
#endif
  for(j=0; j< thdr->mat_count; j++) {
     struct ter_object *tobj = (struct ter_object *)buffer;
     buffer += sizeof(struct ter_object);
     for (unsigned int i=0; i<(unsigned int)tobj->property_count; i++) {
        struct ter_property *tprop = (struct ter_property *)buffer;
     	buffer += sizeof(struct ter_property);
     }

  }

  this->model_data.zone_model = new Zone_Model;
  zm = this->model_data.zone_model;

  zm->vert_count = thdr->vert_count;
  zm->poly_count = thdr->tri_count;

  zm->verts = new Vertex *[zm->vert_count];
  zm->polys = new Polygon *[zm->poly_count];

  this->model_data.plac_count = 0;
  this->model_data.model_count = 0;

  buffer = ter_orig + thdr->list_len + sizeof(ter_header);
  if(thdr->magic[3] == 'M') buffer = buffer + 4;
#ifdef DEBUGTER
  printf("Starting offset is %8X\n", buffer-ter_orig);
#endif
  for(unsigned int b=0; b<thdr->mat_count; b++) {
 	unsigned long property_count = *((unsigned long *)(buffer+12));
#ifdef DEBUGTER
	printf("Property count is %d\n", property_count); fflush(stdout);
#endif
	buffer += 16;
	for (unsigned int a=0; a<property_count; a++)
		buffer += 12;
  }
#ifdef DEBUGTER
  printf("Offset is %8X\n", buffer - ter_orig);
#endif


  for(i = 0; i < (unsigned int)zm->vert_count; ++i) {
    if(thdr->version<3)  {
    	tver = (ter_vertex *) buffer;
        zm->verts[i] = new Vertex;
        zm->verts[i]->x = tver->x;
        zm->verts[i]->y = tver->y;
        zm->verts[i]->z = tver->z;
        zm->verts[i]->u = tver->u;
        zm->verts[i]->v = tver->v;
        buffer += sizeof(ter_vertex);
    }
    else {
    	tverV3 = (ter_vertexV3 *) buffer;
        zm->verts[i] = new Vertex;
        zm->verts[i]->x = tverV3->x;
        zm->verts[i]->y = tverV3->y;
        zm->verts[i]->z = tverV3->z;
        zm->verts[i]->u = tverV3->u;
        zm->verts[i]->v = tverV3->v;
        buffer += sizeof(ter_vertexV3);
    }


  }

  j = 0;
  for(i = 0; i < (unsigned int)zm->poly_count; ++i) {
    ttri = (ter_triangle *) buffer;
    if(ttri->group == -1) {
      buffer += sizeof(ter_triangle);
      continue;
    }
    zm->polys[j] = new Polygon;
/*
    zm->polys[j]->v1 = ttri->v1;
    zm->polys[j]->v2 = ttri->v2;
    zm->polys[j]->v3 = ttri->v3;
*/

// Change the order of the vertices to keep the normals consistent with map files generated for S3Ds in prior versions of azone.

#ifndef KEEP_OLD_EQG_VERTEX_ORDER
    zm->polys[j]->v1 = ttri->v3;
    zm->polys[j]->v2 = ttri->v2;
    zm->polys[j]->v3 = ttri->v1;
#else
	zm->polys[j]->v1 = ttri->v1;
    zm->polys[j]->v2 = ttri->v2;
    zm->polys[j]->v3 = ttri->v3;
#endif
    zm->polys[j]->flags = ttri->unk;


    if(ttri->group == -1) {
#ifdef DEBUGTER
  printf("TERLoader:: zm->poly[%d]->tex = mat_count%d\n", j, thdr->mat_count);
  fflush(stdout);
#endif
      zm->polys[j]->tex = thdr->mat_count;
    }
    else {
#ifdef DEBUGTER
  printf("TERLoader:: zm->poly[%d]->tex = ttri->group %d, Unk=%d\n", j, ttri->group, ttri->unk);
  fflush(stdout);
#endif
      zm->polys[j]->tex = ttri->group;
    }

    ++j;
    buffer += sizeof(ter_triangle);
  }

  zm->poly_count = j;

  zm->tex_count = 0;

  delete[] mlist;

  delete [] ter_orig;

  this->status = 1;
  return 1;
}

int TERLoader::Close() {
  Zone_Model *zm = this->model_data.zone_model;
  int i;

  //return 1;

  if(!this->status)
    return 1;

  for(i = 0; i < zm->vert_count; ++i)
    delete zm->verts[i];
  for(i = 0; i < zm->poly_count; ++i)
    delete zm->polys[i];

  delete[] zm->verts;
  delete[] zm->polys;

  delete this->model_data.zone_model;

  status = 0;

  return 1;
}
