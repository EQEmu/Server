
// This source is from OpenEQ by Daeken et al. Modified a bit by Derision, some bug fixes etc.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "3d.hpp"
#include "zon.hpp"
//#define DEBUGEQG
//#define DEBUGPLAC

ZonLoader::ZonLoader() {
  this->buffer = nullptr;
  this->buf_len = -1;
  this->archive = nullptr;
  this->status = 0;
}

ZonLoader::~ZonLoader() {
  this->Close();
}

int ZonLoader::Open(char *base_path, char *zone_name, Archive *archive) {
  uchar *buffer, *orig_buffer;
  int buf_len;

  Texture **tex;
  int tex_count, tex_tmp, *tex_map;

  int i, j, k, l;
  char **model_names;

  char *filename;

  uchar *zon_tmp, *zon_orig;

  float rot_change = 180.0f / 3.14159f;
  // float rot_change = 1.0f;
  float base[3];

#ifdef DEBUGEQG
  printf("Attempting to load EQG %s\n", zone_name);
#endif
  filename = new char[strlen(zone_name) + 5];
  //sprintf(filename, "%s.zon", zone_name);
  // Derision: .zon name is not always the same as the EQG base name. Added wildcard ability to the PFS GetFile
  // routine.
  sprintf(filename, "*.zon");
#ifdef DEBUGEQG
  printf("Looking for ZON file %s\n", filename);
  archive->GetFile("*.zon", &buffer, &buf_len);
#endif
  if(!archive->GetFile(filename, &buffer, &buf_len)) {
#ifdef DEBUGEQG
  printf("Couldn't find ZON file\n");
  printf("%s\n", zone_name);
#endif
    // Look in the filesystem for the .ZON file.
    char *FSZonName = new char[strlen(zone_name) + 6];
    sprintf(FSZonName, "%s.zon", zone_name);
    // Hack incoming for westkorlach zones because the zon filename for e.g. westkorlacha is westkorlachA
    if(!strncmp(FSZonName, "westkorlach", 11))
    	FSZonName[11] = toupper(FSZonName[11]);
    printf(".ZON file not found inside EQG. Looking for %s in filesystem. ", FSZonName);
    FILE *FSZon = fopen(FSZonName, "rb");

    delete [] FSZonName;

    if(FSZon) {
    	printf("Found.\n");
	fseek(FSZon, 0, SEEK_END);
	long FSZonSize = ftell(FSZon);
	fseek(FSZon, 0, SEEK_SET);
//	printf("File size is %d\n", FSZonSize);
	buffer = new uchar[FSZonSize];
	fread(buffer, FSZonSize, 1, FSZon);
//	printf("Read file\n");
   }
   else {
 	printf("\n.ZON file not present inside EQG file. Did not find it in the filesystem either.\n");
   	return 0;
   }
  }
  delete[] filename;

  orig_buffer = buffer;

  zon_header *hdr = (zon_header *) buffer;
  zon_placeable *plac;

  if(hdr->magic[0] != 'E' || hdr->magic[1] != 'Q' || hdr->magic[2] != 'G' || hdr->magic[3] != 'Z')
    return 0;

  buffer += sizeof(zon_header);

  // The original OpenEQ code I found assumed the first filename was the .TER file. This
  // is not always the case.
  //

#ifdef DEBUGEQG
  printf("Seeking .TER file\n");
#endif



  uchar *StartOfModelNames = buffer;
  while(strcmp((char *)(buffer+strlen((char *)buffer))-3, "TER"))
  	buffer += strlen((char *)buffer) + 1;

#ifdef DEBUGEQG
  printf("Found .TER file %s\n", (char *)buffer);
#endif
  this->terloader.Open(nullptr, (char *) buffer, archive);
  buffer = StartOfModelNames;
  this->model_data.zone_model = terloader.model_data.zone_model;

  // Derision:
  // After the ZON header, there is a list of strings like this (using guildhall.EQG as an example)
  // TER_GuildHall.TER
  // TER_GuildHall
  // OBJ_sun_frame.MOD
  // OBJ_sun_frame
  // OBJ_chandelier.MOD
  // OBJ_chandelier
  // OBJ_key_door_open_.MOD
  // OBJ_key_door_open_01
  // OBJ_key_door_open_02
  // OBJ_key_door_.MOD
  // OBJ_key_door_01
  // OBJ_key_door_02
  // OBJ_key_door_03
  //
  // This format appears to be Mesh (model) filename, followed by a list of objects that use that model.
  // So we have the zone model (ground mesh), i.e. the .TER file, which is used by object 'TER_Guildhall',
  // then a list of placeable models. It would appear that one .MOD file can be used by multiple objects,
  // i.e. the key_door_open and key_door models in the example above.
  //
  // After this list, there is a list of hdr->NumberOfModels longs, one for each Model. Each long is an offset into
  // the list above, pointing to the .MOD file for that model.

  zon_orig = buffer;
  zon_tmp = buffer + hdr->list_len;

  // Create an array of Model filenames and populate it using the offsets
  //
  buffer = zon_orig + hdr->list_len;
  model_names = new char *[hdr->NumberOfModels];
  for(int ModelNumber=0; ModelNumber< hdr->NumberOfModels; ModelNumber++) {
 	long offset = *((long *)(buffer+ModelNumber*4));
	model_names[ModelNumber] = (char *)(zon_orig + offset);
	//
	// Derision: 23/06/08
	// dranikcatacombsa.eqg has a couple of model names with a left parenthesis where there should
	// be an underscore. E.g. OBP_DZ_Lbanner0)_00.MOD instead of OBP_DZ_Lbanner0__00.MOD
	// This is the only zone I have seen this in, but is the reason for the follow code to replace
	// the parenthesis with an underscore.
	//
	for(unsigned int i=0; i<strlen(model_names[ModelNumber]); i++)
		if(model_names[ModelNumber][i] == ')') {
			model_names[ModelNumber][i] = '_';
			printf("***** Replacing parenthesis with underscore in model name %s\n", model_names[ModelNumber]);
		}
//	printf("Model name %d is %s\n", ModelNumber, model_names[ModelNumber]);
  }
  this->model_data.model_count = hdr->NumberOfModels;
#if defined(DEBUGEQG) || defined(DEBUGPLAC)
  printf("model_data.model_count is %d\n", this->model_data.model_count);
  fflush(stdout);
#endif
  this->model_data.models = new Model *[this->model_data.model_count];



  // Skip over the
  buffer = zon_orig + hdr->list_len + hdr->NumberOfModels * 4;

  this->model_data.plac_count = hdr->obj_count - 1;
/*
  if(hdr->version > 1) {
  	// Don't understand the complete format of version 2 zons
	this->model_data.plac_count = 0;
  }
*/

#ifdef DEBUGPLAC
  printf(" Placeable count is %d\n", this->model_data.plac_count);
#endif
   this->model_data.placeable = new Placeable *[this->model_data.plac_count];

  plac = (zon_placeable *) buffer;
  base[0] = plac->x;
  base[1] = plac->y;
  base[2] = plac->z;

//  printf("(%f %f %f) (%f %f %f)\n", plac->x, plac->y, plac->z, plac->rz, plac->ry, plac->rx);

  buffer += sizeof(zon_placeable);

  if(hdr->version > 1) {
  	long UnknownSize = *((long *)(buffer));
//	printf("Unknown Size is %d\n", UnknownSize);
	buffer = buffer + 4 + (UnknownSize * 4);
  }


#ifdef DEBUGEQG
  printf("TER.CPP:107 model_data.plac_count is %d\n", this->model_data.plac_count);
#endif


  for(i = 0; i < this->model_data.plac_count; ++i) {
    plac = (zon_placeable *) buffer;

    zon_tmp = zon_orig + plac->loc;

    this->model_data.placeable[i] = new Placeable;
    this->model_data.placeable[i]->model = plac->id ;
    this->model_data.placeable[i]->x = plac->x;
    this->model_data.placeable[i]->y = plac->y;
    this->model_data.placeable[i]->z = plac->z;
    this->model_data.placeable[i]->rx = plac->rz * rot_change;
    this->model_data.placeable[i]->ry = plac->ry * rot_change;
    this->model_data.placeable[i]->rz = plac->rx * rot_change;

    this->model_data.placeable[i]->scale[0] = plac->scale;
    this->model_data.placeable[i]->scale[1] = plac->scale;
    this->model_data.placeable[i]->scale[2] = plac->scale;
#ifdef DEBUG_MODE
    printf("%s (%f %f %f) (%f %f %f) %f\n",
           zon_tmp,
           this->model_data.placeable[i]->x,
           this->model_data.placeable[i]->y,
           this->model_data.placeable[i]->z,
           this->model_data.placeable[i]->rx,
           this->model_data.placeable[i]->ry,
           this->model_data.placeable[i]->rz,
           this->model_data.placeable[i]->scale);
#endif
    if(this->model_data.placeable[i]->model == -1) {
    	printf("Unable to find model for offset %5ld\n", plac->loc);
    }

#ifdef DEBUGPLAC
    printf("  Placeable object %i Model no. %d at (%4.2f, %4.2f, %4.2f)\n", i, this->model_data.placeable[i]->model, plac->x,plac->y, plac->z);
#endif
    buffer += sizeof(zon_placeable);
    if(hdr->version > 1) {
  	long UnknownSize = *((long *)(buffer));
	buffer = buffer + 4 + (UnknownSize * 4);
    }


  }

  model_loaders = new TERLoader[this->model_data.model_count];

  tex_count = this->model_data.zone_model->tex_count;
#ifdef DEBUGEQG
  printf("zon.cpp line 154 zm tex count is %d\n", this->model_data.zone_model->tex_count);
#endif

  for(j = 0; j < this->model_data.model_count; ++j) {

    // We have already loaded the zone mesh, so skip the .TER file when processing models.
    //
    if(!(strcmp(model_names[j]+strlen(model_names[j])-3, "ter"))) {
//    	printf("Skipping .TER file at model number %d\n", j);
		this->model_data.models[j] = nullptr;
		continue;
    }
//    printf("Attempting to open MOD file %s\n", model_names[j]); fflush(stdout);
    if(model_loaders[j].Open(nullptr, model_names[j], archive)) {

        this->model_data.models[j] = new Model;
        this->model_data.models[j]->vert_count = model_loaders[j].model_data.zone_model->vert_count;
        this->model_data.models[j]->poly_count = model_loaders[j].model_data.zone_model->poly_count;
        this->model_data.models[j]->tex_count = model_loaders[j].model_data.zone_model->tex_count;
        this->model_data.models[j]->verts = model_loaders[j].model_data.zone_model->verts;
        this->model_data.models[j]->polys = model_loaders[j].model_data.zone_model->polys;
        this->model_data.models[j]->tex = model_loaders[j].model_data.zone_model->tex;
	this->model_data.models[j]->name = new char[strlen(model_names[j])+1];
	strcpy(this->model_data.models[j]->name, model_names[j]);

        tex_tmp = 1;
	//    I think this is looking to see if the placeable model textures already exist in the zone model.
	//
        for(i = 0; i < this->model_data.models[j]->tex_count; ++i) {
	  tex_tmp = 1;  // Derision
          for(k = 0; k < this->model_data.zone_model->tex_count; ++k) {
//	  printf("    Checking zm tex filename %s againt model filename %s\n", this->model_data.zone_model->tex[k]->filenames[0],
//	                                                                       this->model_data.models[j]->tex[i]->filenames[0]);
            if((!this->model_data.zone_model->tex[k]->filenames[0] && !this->model_data.models[j]->tex[i]->filenames[0]) ||
               (this->model_data.zone_model->tex[k]->filenames[0] &&
                this->model_data.models[j]->tex[i]->filenames[0] &&
                !strcmp(this->model_data.zone_model->tex[k]->filenames[0], this->model_data.models[j]->tex[i]->filenames[0]))) {
              tex_tmp = 0;
              break;
            }
          }
          if(tex_tmp)
            ++tex_count;
          else {
            for(k = 0; k < j; ++k) {
	      if(!this->model_data.models[k]) {
#ifdef DEBUGEQG
                  printf("nullptr  this->model_data.models[k])\n");
#endif
	          continue;
	      }
              for(l = 0; l < this->model_data.models[k]->tex_count; ++l) {
                if(this->model_data.models[k]->tex[l]->filenames[0] == this->model_data.models[j]->tex[i]->filenames[0] ||
                   (this->model_data.models[k]->tex[l]->filenames[0] &&
                    this->model_data.models[j]->tex[i]->filenames[0] &&
                    !strcmp(this->model_data.models[k]->tex[l]->filenames[0], this->model_data.models[j]->tex[i]->filenames[0]))) {
                  tex_tmp = 0;
                  break;
                }
              }
            }
            if(tex_tmp)
              ++tex_count;
          }
        }
    }
	else this->model_data.models[j] = nullptr;
  }

//  printf("Tex count is %i %X\n", tex_count, tex_count);
  // Allocate a new Texture array
  tex_count = 0;
  tex_tmp = 1;
  // Set the first tex_count textures in the new Texture array to the textures from the zone model
    // For i in each model
  for(i = 0; i < this->model_data.model_count; ++i) {
    if(!this->model_data.models[i]) {
#ifdef DEBUGEQG
        printf("nullptr  this->model_data.models[i])\n");
#endif
    	continue;
    }
    // For j = each texture in this model
    for(j = 0; j < this->model_data.models[i]->tex_count; ++j) {
      tex_tmp = 1;  // Derision
      // for k = each texture in the zone model
      for(k = 0; k < tex_count; ++k) {
        // if the texture filenames are at the same address or, the texture filenames are the same, we set tex_tmp to 0
        if(tex[k]->filenames[0] == this->model_data.models[i]->tex[j]->filenames[0] ||
           (tex[k]->filenames[0] &&
            this->model_data.models[i]->tex[j]->filenames[0] &&
            !strcmp(tex[k]->filenames[0], this->model_data.models[i]->tex[j]->filenames[0]))) {
          tex_tmp = 0;
          break;
        }
      }
      // if tex_tmp is 0, i.e. this texture is not in the zone model, we add this texture to our new Texture array, at the end
      if(tex_tmp) {
        tex[tex_count] = this->model_data.models[i]->tex[j];
        ++tex_count;
      }
    }
  }
  for(i = 0; i < this->model_data.model_count; ++i) {
    if(!this->model_data.models[i]) {
#ifdef DEBUGEQG
        printf(">nullptr  this->model_data.models[i])\n");
#endif
        continue;
    }
    tex_map = new int[this->model_data.models[i]->tex_count];

    for(j = 0; j < this->model_data.models[i]->tex_count; ++j) {
      tex_map[j] = 0;
      for(k = 0; k < tex_count; ++k) {
        if(tex[k]->filenames[0] == this->model_data.models[i]->tex[j]->filenames[0] ||
           (tex[k]->filenames[0] &&
            this->model_data.models[i]->tex[j]->filenames[0] &&
            !strcmp(tex[k]->filenames[0], this->model_data.models[i]->tex[j]->filenames[0]))) {
          tex_map[j] = k;
          break;
        }
      }
    }

    delete[] tex_map;
  }

  delete [] model_names;

  this->model_data.zone_model->tex = 0;
  this->model_data.zone_model->tex_count = 0;
  delete [] orig_buffer;

  this->status = 1;
  return 1;
}


int ZonLoader::Close() {

	int i;

	if(!this->status)
		return 1;

	this->terloader.Close();

	for(i = 0; i < this->model_data.model_count; ++i)
		this->model_loaders[i].Close();

	for(i = 0; i < this->model_data.model_count; ++i)
	{
		if(this->model_data.models[i])
		{
			delete [] this->model_data.models[i]->name;
			delete this->model_data.models[i];
		}
	}

	for(i = 0; i < this->model_data.plac_count; ++i)
		delete this->model_data.placeable[i];

	delete [] this->model_data.placeable;

	delete [] this->model_data.models;

	delete[] this->model_loaders;

	status = 0;

  return 1;
}
