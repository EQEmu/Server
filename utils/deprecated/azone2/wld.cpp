
// This source is from OpenEQ by Daeken et al. Modified a bit by Derision, some bug fixes etc.

#include "wld.hpp"
#include "file.hpp"
#include "pfs.hpp"

#include <string.h>
#include <stdlib.h>
//#define DEBUGWLD

static uchar encarr[] = {0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A};

inline void decode(uchar *str, int len) { int i; for(i = 0; i < len; ++i) str[i] ^= encarr[i % 8]; }


FRAG_CONSTRUCTOR(Data03) {
  int i, count, fnlen;
  Texture *tex;

  count = uint32(buf);
  if(!count) count = 1;

  tex = new Texture;
  tex->filenames = new char *[count];
  tex->frame_count = count;
  tex->current_frame = 0;
  tex->archive = nullptr;
  tex->flags = 0;

  for(i = 0; i < count; ++i) {
    fnlen = uint16(buf);
    tex->filenames[i] = new char[fnlen];
    memcpy(tex->filenames[i], buf, fnlen);
    decode((uchar *) tex->filenames[i], fnlen);

    // Derision: Not sure why this check is here, but need to check fnlen is >=18
    if(fnlen>=18) {
      if(tex->filenames[i][fnlen - 8] == '.')
        tex->filenames[i][fnlen - 17] = '\0';
    }

    buf += fnlen;
  }
  this->frag = (void *) tex;
}

FRAG_CONSTRUCTOR(Data04) {
  int flags, count, i;
  Texture *tex, *texref;

  flags = uint32(buf);
  count = uint32(buf);

  if(flags & (1 << 2))
    buf += 4;
  if(flags & (1 << 3))
    buf += 4;

  if(count == 0 || count == 1)
    this->frag = wld->frags[uint32(buf) - 1]->frag;
  else {
    tex = new Texture;
    tex->frame_count = count;
    tex->current_frame = 0;
    tex->archive = nullptr;
    tex->filenames = new char *[count];
    tex->flags = 0;

    for(i = 0; i < count; ++i) {
      texref = (Texture *) wld->frags[uint32(buf) - 1]->frag;
      tex->filenames[i] = texref->filenames[texref->frame_count - 1];
    }

    this->frag = tex;
  }
}

FRAG_CONSTRUCTOR(Data05) {
  this->frag = wld->frags[uint32(buf) - 1]->frag;
}

FRAG_CONSTRUCTOR(Data15) {
  struct_Data15 *hdr = (struct_Data15 *) buf;
  Placeable *plac = new Placeable;
  Placeable **pl;

  plac->x = hdr->trans[0];
  plac->y = hdr->trans[1];
  plac->z = hdr->trans[2];

  plac->rx = hdr->rot[2] / 512.f * 360.f;
  plac->ry = hdr->rot[1] / 512.f * 360.f;
  plac->rz = hdr->rot[0] / 512.f * 360.f;

  plac->scale[0] = hdr->scale[2];
  plac->scale[1] = hdr->scale[1];
  plac->scale[2] = hdr->scale[1];

  plac->model = atoi((const char*)&wld->sHash[-(int)hdr->ref]);

  pl = new Placeable *[wld->model_data.plac_count + 1];
  memcpy(pl, wld->model_data.placeable, sizeof(Placeable *) * wld->model_data.plac_count);
  if(wld->model_data.plac_count)
    delete[] wld->model_data.placeable;
  pl[wld->model_data.plac_count] = plac;
  wld->model_data.placeable = pl;
  ++wld->model_data.plac_count;
}

FRAG_CONSTRUCTOR(Data1B) {
  struct_Data1B *data = (struct_Data1B *) buf;
  Light *light = new Light;
  this->frag = light;
  light->x = light->y = light->z = light->rad = 0;
  if(data->flags & (1 << 3)) {
    light->r = data->color[0];
    light->g = data->color[1];
    light->b = data->color[2];
  }
  else
    light->r = light->g = light->b = 1.0f;
}

FRAG_CONSTRUCTOR(Data1C) {
  this->frag = wld->frags[uint32(buf) - 1]->frag;
}

FRAG_CONSTRUCTOR(Data21) {

  struct_Data21 *data;
  long count = *((long *) buf);
  long i;

  wld->tree = (BSP_Node *) malloc(count * sizeof(BSP_Node));

  // Build the BSP Tree
  //
  for(i = 0; i < count; ++i) {
    wld->tree[i].node_number=i;
    data = (struct_Data21 *) (buf + (i * sizeof(struct_Data21)) + 4);
    wld->tree[i].normal[0] = data->normal[0];
    wld->tree[i].normal[1] = data->normal[1];
    wld->tree[i].normal[2] = data->normal[2];
    wld->tree[i].splitdistance = data->splitdistance;
    wld->tree[i].region = data->region;
    wld->tree[i].left = data->node[0];
    wld->tree[i].right = data->node[1];
  }



}

FRAG_CONSTRUCTOR(Data22) {

  int pos;

  uchar *data6area ;

  struct_Data22 *data = (struct_Data22 *) buf;

  pos = sizeof(struct_Data22) + (12 * data->size1) + (8 * data->size2);




  data6area = buf + pos;

  if((data->size3!=0)||(data->size4!=0)) {
        printf("Size 3 and 4 not zero, we can't handle that yet\n");
        exit(1);
  }
  data6area = data6area + ((data->size5) * 7 * 4);

  unsigned short d6size = *((unsigned short *) data6area);
  data6area = data6area + 2; // Move past d6 size
  data6area = data6area + d6size; // Move past RLE data ? Hopefully;

  float f1, f2, f3, f4;

  f1 = *((float *) data6area);
  f2 = *((float *) (data6area+4));
  f3 = *((float *) (data6area+8));
  f4 = *((float *) (data6area+12));

  long Frag36Ref;
  if(data->flags==0x181) {
	  Frag36Ref = *((long *) (data6area+20));
  }





}

FRAG_CONSTRUCTOR(Data29) {

        long  a,flags, numregions, lenstr;

	struct_Data29 *data29 = (struct_Data29 *) malloc(sizeof(struct_Data29));
	data29->region_type = -1; // Start of by flagging type as unknown

	if(!strncmp((char *) &wld->sHash[-frag_name], "WT", 2)) data29->region_type = 1; // Water
	else if(!strncmp((char *) &wld->sHash[-frag_name], "LA", 2)) data29->region_type = 2; // Lava
	else if(!strncmp((char *) &wld->sHash[-frag_name], "DRNTP", 5)) data29->region_type = 3; // Zone Line
	else if(!strncmp((char *) &wld->sHash[-frag_name], "DRP_", 4)) data29->region_type = 4; // PVP
	else if(!strncmp((char *) &wld->sHash[-frag_name], "SL", 2)) data29->region_type = 5; // Slippery/Slime ?
	else if(!strncmp((char *) &wld->sHash[-frag_name], "DRN", 3)) data29->region_type = 6; // Ice/Ice Water
	else if(!strncmp((char *) &wld->sHash[-frag_name], "VWA", 3)) data29->region_type = 7; // VWater ?

	this->frag = (void *) data29;

        flags = *buf;
        numregions = *((long *)(buf+4));
        data29->region_count = numregions ;
        data29->region_array = (long *)malloc(numregions * sizeof(long));
        for(a=0;a<numregions;a++) {
                data29->region_array[a] = *((long *)(buf+8+(a*4)));
        }
        lenstr = *((long *)(buf+8+(numregions*4)));
        data29->strlen=lenstr;
        if(lenstr==0) return;

        data29->str = (char *)malloc(lenstr);

        char *encstr;

        encstr =  (char *)buf+8+(++a*4);

        decode((uchar *)encstr, lenstr);
	strcpy(data29->str, encstr);
//	printf("   data2 name is %s\n", data29->str);
	if(lenstr>=5)
		if(!strncmp(encstr,"WT",2)) data29->region_type = 1; // Water
		else if(!strncmp(encstr,"LA",2)) data29->region_type = 2; // Lava
		else if(!strncmp(encstr,"DRNTP",5)) data29->region_type = 3; // Zone Line ?
		else if(!strncmp(encstr,"DRP_",4)) data29->region_type = 4; //PVP
		else if(!strncmp(encstr,"SL",2)) data29->region_type = 5; // Slippery/Slime ?
		else if(!strncmp(encstr,"DRN",3)) data29->region_type = 6; // Ice/Ice Water ?
		else if(!strncmp(encstr,"VWN",3)) data29->region_type = 7; // Ice/Ice Water ?
                else data29->region_type = -2; // Flag this so that it doesn't get set to the default water or lava
                                               // later, because it is neither.
                                               //

}

FRAG_CONSTRUCTOR(Data28) {
//  struct_Data28 *data = (struct_Data28 *) buf; // obviously unused for now.
}

FRAG_CONSTRUCTOR(Data30) {
  struct_Data30 *data;
  uint32 ref;
  Texture *tex;

  data = (struct_Data30 *) buf;

  buf += sizeof(struct_Data30);

  if(!data->flags)
    buf += 8;

  ref = uint32(buf);

  if(!data->params1 || !ref) {
    tex = new Texture;
    tex->frame_count = 1;
    tex->current_frame = 0;
    tex->filenames = new char *[1];
    tex->filenames[0] = "collide.dds";
    tex->archive = nullptr;
    tex->flags = 1;
    this->frag = (void *) tex;
    return;
  }

  tex = (Texture *) wld->frags[ref - 1]->frag;

  if(data->params1 & (1 << 1) || data->params1 & (1 << 2) || data->params1 & (1 << 3) || data->params1 & (1 << 4))
    tex->flags = 1;
  else
    tex->flags = 0;

  this->frag = (void *) tex;
}

FRAG_CONSTRUCTOR(Data31) {
  TexRef *tr;
  int i, count;

  buf += 4;
  count = uint32(buf);

  tr = new TexRef;
  tr->tex_count = count;
  tr->tex = new Texture *[count];

  for(i = 0; i < count; ++i)
    tr->tex[i] = (Texture *) wld->frags[uint32(buf) - 1]->frag;

  this->frag = (void *) tr;
}

FRAG_CONSTRUCTOR(Data36) {
  struct_Data36 *header = (struct_Data36 *) buf;
  Poly *p;
  TexCoordsNew *tex_new;
  TexCoordsOld *tex_old;
  VertexNormal *vn;
  Vert *v;



  TexRef *tr;

  Model *model;
  Vertex *vert;
  Polygon *poly;

  float scale = 1.0f / (float) (1 << header->scale);

  int i, j, pc;
  TexMap *tm;

  float recip_255 = 1.0f / 256.0f, recip_127 = 1.0f / 127.0f;

  buf += sizeof(struct_Data36);

  model = new Model;
  model->vert_count = header->vertexCount;
  model->poly_count = header->PolygonsCount;
  model->verts = new Vertex *[model->vert_count];
  model->polys = new Polygon *[model->poly_count];
  tr = (TexRef *) wld->frags[header->fragment1 - 1]->frag;
  model->tex = tr->tex;    // Reference to type 0x31 Texture List fragment
  model->tex_count = tr->tex_count;
  model->name = (char *) &wld->sHash[-frag_name];

#ifdef DEBUGWLD

  printf("Frag 0x36, %4d Vertices, %4d Polys, %4d TexCoords, %4d Normals, %3d Colors, %3d Polytex, %3d VertexTex, Name %s\n",
         header->vertexCount, header->PolygonsCount, header->texCoordsCount, header->normalsCount, header->colorCount, header->PolygonTexCount, header->vertexTexCount, model->name);
#endif

  for(i = 0; i < header->vertexCount; ++i) {
    v = (Vert *) buf;

    vert = model->verts[i] = new Vertex;

    vert->x = header->centerX + v->x * scale;
    vert->y = header->centerY + v->y * scale;
    vert->z = header->centerZ + v->z * scale;
#ifdef DEBUGWLD2
    printf(" Vertex %4d: X=%4.2f, Y=%4.2f, Z=%4.2f\n", i, vert->x, vert->y, vert->z);
#endif

    buf += sizeof(Vert);
  }

  if(wld->old) {
    for(i = 0; i < header->texCoordsCount; ++i) {
      tex_old = (TexCoordsOld *) buf;
      vert = model->verts[i];

      vert->u = (float) tex_old->tx * recip_255;
      vert->v = (float) tex_old->tz * recip_255;

#ifdef DEBUGWLD2
      printf(" TexCoord %4d: U=%4.2f, V=%4.2f\n", i, vert->u, vert->v);
#endif

      buf += sizeof(TexCoordsOld);
    }
  }
  else {
    for(i = 0; i < header->texCoordsCount; ++i) {
      tex_new = (TexCoordsNew *) buf;
      vert = model->verts[i];

      vert->u = tex_new->tx;
      vert->v = tex_new->tz;

#ifdef DEBUGWLD2
      printf(" TexCoord %4d: U=%4.2f, V=%4.2f\n", i, vert->u, vert->v);
#endif
      buf += sizeof(TexCoordsNew);
    }
  }


  for(i = 0; i < header->normalsCount; ++i) {

    if(i<header->vertexCount) {
      vn = (VertexNormal *) buf;
      vert = model->verts[i];

      vert->i = (float) vn->nx * recip_127;
      vert->j = (float) vn->ny * recip_127;
      vert->k = (float) vn->nz * recip_127;
    }

    buf += sizeof(VertexNormal);
  }

  buf += 4 * header->colorCount;

  for(i = 0; i < header->PolygonsCount; ++i) {
    p = (Poly *) buf;

    poly = model->polys[i] = new Polygon;

    poly->flags = p->flags;
    // Derision: 16/06/08
    // Previously, this was assigning v1=v1, v2=v3, v3=v2. This is different to how it was done
    // in the WLD loader in the stock azone, and has the effect of screwing up the normals,
    // Changed to keep things consistent with prior versions.
    poly->v1 = p->v1;
    poly->v2 = p->v2;
    poly->v3 = p->v3;
#ifdef DEBUGWLD2
    printf("Frag36: Polygon: %5d Vertices: %5d, %5d, %5d\n", i, poly->v1, poly->v2, poly->v3);
#endif
    buf += sizeof(Poly);
  }

  buf += 4 * header->size6;

  pc = 0;

  // For each polygontex entry ...
  for(i = 0; i < header->PolygonTexCount; ++i) {
    tm = (TexMap *) buf;

	for(j = 0; j < tm->polycount; ++j) {
	  if(pc>=(model->poly_count)) {
		    break; // Derision: Crashing here in acrylia because of this. TODO: Find root cause, for now, this lets zone load
	  }
	  // there are tm->polycount consecutive polys that use this tex
          model->polys[pc++]->tex = tm->tex;
	}

    buf += 4;
  }


#ifdef DEBUGWLD
  fflush(stdout);
#endif

  this->frag = (void *) model;
}

WLDLoader::WLDLoader() {
}

WLDLoader::~WLDLoader() {
  this->Close();
}

int WLDLoader::Open(char *base_path, char *zone_name, Archive *archive) {
  uchar *buffer;
  int buf_len;
  int i, j, vc, pc, bc, mlen, *pmap;
  Zone_Model *zm;
  Model *m;

  Archive *obj_archive;

  char *filename, *model_name;

  struct_wld_header *header;
  struct_wld_basic_frag *frag;

#ifdef DEBUGWLD
  printf("Zone: %s\n", zone_name);
#endif
  this->model_data.zone_model = nullptr;

  this->model_data.plac_count = 0;
  this->model_data.placeable = 0;

  this->obj_loader = this->plac_loader = nullptr;

  this->clear_plac = 0;

  filename = new char[strlen(zone_name) + 5];
  sprintf(filename, "%s.wld", zone_name);

  if(!GetFile(&buffer, &buf_len, nullptr, filename, archive)) {
    delete[] filename;
    return 0;
  }
  delete[] filename;

  header = (struct_wld_header *) buffer;

  if(header->magic != 0x54503d02)
    return 0;

  buffer += sizeof(struct_wld_header);

  this->sHash = buffer;
  decode(this->sHash, header->stringHashSize);
  buffer += header->stringHashSize;

  if(header->version == 0x00015500)
    this->old = 1;
  else
    this->old = 0;

  this->fragcount = header->fragmentCount;
  this->frags = new Fragment *[header->fragmentCount];


  for(i = 0; i < this->fragcount; ++i) {
    frag = (struct_wld_basic_frag *) buffer;

    buffer += sizeof(struct_wld_basic_frag);

    switch(frag->id) {
    case 0x03: FRAGMENT(Data03); break;
    case 0x04: FRAGMENT(Data04); break;
    case 0x05: FRAGMENT(Data05); break;
    case 0x15: FRAGMENT(Data15); break;
    case 0x1B: FRAGMENT(Data1B); break;
    case 0x1C: FRAGMENT(Data1C); break;
    case 0x21: FRAGMENT(Data21); break;
    case 0x22: FRAGMENT(Data22); break;
    case 0x29: FRAGMENT(Data29); break;
    case 0x28: FRAGMENT(Data28); break;
    case 0x30: FRAGMENT(Data30); break;
    case 0x31: FRAGMENT(Data31); break;
    case 0x36: FRAGMENT(Data36); break;

    default: this->frags[i] = new Fragment; break;
    }

    this->frags[i]->type = frag->id;
    this->frags[i]->name = frag->nameRef;

    buffer += frag->size - 4;
  }




  if(!strcmp(&zone_name[strlen(zone_name) - 4], "_obj")) {
    this->model_data.plac_count = 0;
    this->model_data.model_count = 0;
    // for an obj_s3d file, find out how many placeabale objects there are (each 0x36 Frag is a placeable object in an obj_s3d file
    for(i = 0; i < this->fragcount; ++i) {
      if(this->frags[i]->type == 0x36)
        ++this->model_data.model_count;
    }
    // allocate space for this many models
    this->model_data.models = new Model *[this->model_data.model_count];
    this->model_data.model_count = 0;
    for(i = 0; i < this->fragcount; ++i) {
      if(this->frags[i]->type == 0x36)
        this->model_data.models[this->model_data.model_count++] = (Model *) this->frags[i]->frag;
    }
  }
  else if(!strcmp(&zone_name[strlen(zone_name) - 4], "_chr")) {
  }
  else if(!strcmp(zone_name, "objects")) {
    this->clear_plac = 1;
  }
  else {
    // We are procesing the zone mesh S3D
    zm = this->model_data.zone_model = new Zone_Model;
    zm->vert_count = 0;
    zm->poly_count = 0;
    for(i = 0; i < this->fragcount; ++i) {
      if(this->frags[i]->type != 0x36)
        continue;
      m = (Model *) this->frags[i]->frag;
      zm->vert_count += m->vert_count;
      zm->poly_count += m->poly_count;
      zm->tex_count = m->tex_count; // The texcount and tex fields are the same for every 0x36 frag in the zone mesh S3D
      zm->tex = m->tex;
    }



    zm->verts = new Vertex *[zm->vert_count];
    zm->polys = new Polygon *[zm->poly_count];

    vc = pc = 0;

    for(i = 0; i < this->fragcount; ++i) {
      if(this->frags[i]->type != 0x36)
        continue;
      m = (Model *) this->frags[i]->frag;
      bc = vc;
      // Copy the vertexes from the 0x36 frag into our zone model
      for(j = 0; j < m->vert_count; ++j)
        zm->verts[vc++] = m->verts[j];
      // Adjust the polygon vertices indices to match the indices in our zone model vertex array
      for(j = 0; j < m->poly_count; ++j) {
        m->polys[j]->v1 += bc;
        m->polys[j]->v2 += bc;
        m->polys[j]->v3 += bc;
#ifdef DEBUGWLD
	printf("Polygon: %5d  Vertices %5d, %5d, %5d\n", j,
	        m->polys[j]->v1, m->polys[j]->v2, m->polys[j]->v3);
#endif

#ifdef DEBUGWLD
        printf("Zone Frag36 No. %5d: Poly %5d V3 = %4.3f, %4.3f, %4.3f\n", i, j, zm->verts[m->polys[j]->v3]->x, zm->verts[m->polys[j]->v3]->y, zm->verts[m->polys[j]->v3]->z);
	fflush(stdout);
#endif
        zm->polys[pc++] = m->polys[j];
      }
    }

    this->model_data.plac_count = 0;
    this->model_data.model_count = 0;

    filename = new char[strlen(zone_name) + 10];
    sprintf(filename, "%s_obj.s3d",zone_name);

    obj_archive = new PFSLoader;
    if(!obj_archive->Open(fopen(filename, "rb"))) {
//    	printf("_obj.s3d file not found.\n");
	return 1;
    }

    delete[] filename;

    filename = new char[strlen(zone_name) + 5];
    sprintf(filename, "%s_obj", zone_name);

    this->obj_loader = new WLDLoader();
    this->obj_loader->Open(nullptr, filename, obj_archive);

    delete[] filename;

    this->model_data.model_count = this->obj_loader->model_data.model_count;
    this->model_data.models = this->obj_loader->model_data.models;

    this->plac_loader = new WLDLoader();
    this->plac_loader->Open(nullptr, "objects", archive);

    if(this->model_data.plac_count) {
      delete this->model_data.placeable[0];
      delete[] this->model_data.placeable;
    }

    this->model_data.plac_count = this->plac_loader->model_data.plac_count;
    this->model_data.placeable = this->plac_loader->model_data.placeable;


    for(i = 0; i < this->model_data.plac_count; ++i) {
      mlen = strlen((char *) this->model_data.placeable[i]->model) - 8;
      model_name = new char[mlen + 12];
      memcpy(model_name, (char *) this->model_data.placeable[i]->model, mlen);
      model_name[mlen] = 0;
      sprintf(model_name, "%sDMSPRITEDEF", model_name);
      this->model_data.placeable[i]->model = -1;

      for(j = 0; j < this->model_data.model_count; ++j) {
        if(!strcmp(this->model_data.models[j]->name, model_name)) {
          this->model_data.placeable[i]->model = j;
//	    printf("Placeable object %d name is %s\n", j, model_name);
          break;
        }
      }


      delete[] model_name;
    }

    for(i = 0; i < this->model_data.model_count; ++i) {
      for(j = 0; j < this->model_data.models[i]->tex_count; ++j)
        this->model_data.models[i]->tex[j]->archive = obj_archive;
    }

    for(i = 0; i < this->model_data.model_count; ++i) {
      pmap = new int[this->model_data.models[i]->poly_count];
      memset(pmap, 0, sizeof(int) * this->model_data.models[i]->poly_count);
      for(j = 0; j < this->model_data.models[i]->tex_count; ++j) {
        filename = this->model_data.models[i]->tex[j]->filenames[0];
        for(bc = 0; bc < this->model_data.zone_model->tex_count; ++bc) {
          if(!strcmp(filename, this->model_data.zone_model->tex[bc]->filenames[0])) {
            for(pc = 0; pc < this->model_data.models[i]->poly_count; ++pc) {
              if(!pmap[pc] && this->model_data.models[i]->polys[pc]->tex == j) {
                this->model_data.models[i]->polys[pc]->tex = bc;
                pmap[pc] = 1;
              }
            }
            break;
          }
        }
      }
      delete[] pmap;
    }
  }

  return 1;
}

int WLDLoader::Close() {
  int i, j;
  Zone_Model *zm = this->model_data.zone_model;
  Model *m;

  return -1;

  if(zm) {
    for(j = 0; j < zm->tex_count; ++j)
      delete zm->tex[j];
    delete[] zm->tex;

    delete[] zm->verts;
    delete[] zm->polys;
  }

  for(i = 0; i < this->fragcount; ++i) {
    if(this->frags[i]->type == 0x36) {
      m = (Model *) this->frags[i]->frag;

      for(j = 0; j < m->vert_count; ++j)
        delete m->verts[j];
      for(j = 0; j < m->poly_count; ++j)
        delete m->polys[j];

      if(!zm) {
        for(j = 0; j < m->tex_count; ++j)
          delete m->tex[j];
        delete[] m->tex;
      }

      delete[] m->verts;
      delete[] m->polys;
      delete m;
    }
    else
      delete this->frags[i];
  }
  delete[] this->frags;

  if(this->clear_plac) {
    // for(i = 0; i < this->model_data.plac_count; ++i)
    //  delete[] this->model_data.placeable[i];
  }

  delete[] this->model_data.placeable;

  delete this->obj_loader;
  delete this->plac_loader;

  return 1;
}

