
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "3d.hpp"
#include "zonv4.hpp"
//#define DEBUGEQG
//#define DEBUGPLAC

string GetToken(uchar *&Buffer, int &Position);

Zonv4Loader::Zonv4Loader()
{
	this->buffer = nullptr;
	this->buf_len = -1;
	this->archive = nullptr;
	this->status = 0;
}

Zonv4Loader::~Zonv4Loader()
{
	this->Close();
}

int Zonv4Loader::Open(char *base_path, char *zone_name, Archive *archive)
{

	uchar *buffer;
	int position;
	int buf_len;

	Texture **tex;
	int tex_count, tex_tmp, *tex_map;

	int i, j, k, l;
	char **model_names;

	char *filename;

	uchar *zon_tmp, *zon_orig;

	float rot_change = 180.0f / 3.14159f;

	float base[3];

	printf("Attempting to load EQG %s\n", zone_name);

	filename = new char[strlen(zone_name) + 5];

	sprintf(filename, "*.zon");
#ifdef DEBUGEQG
	printf("Looking for ZON file %s\n", filename);
	archive->GetFile("*.zon", &buffer, &buf_len);
#endif
	if(!archive->GetFile(filename, &buffer, &buf_len))
	{
		printf("Couldn't find ZON file %s\n", zone_name);

	 	return 0;
	}
	delete[] filename;

	zonv4_header *hdr = (zonv4_header *) buffer;

	zonv4_placeable *plac;

	if(hdr->magic[0] != 'E' || hdr->magic[1] != 'Q' || hdr->magic[2] != 'T' || hdr->magic[3] != 'Z')
	  return 0;
#ifdef DEBUGEQG
	printf("Found V4 EQG.\n"); fflush(stdout);

	printf("Seeking .DAT file\n");
#endif
	buffer += sizeof(zonv4_header);

	position = 0;

	char ZonName[255];

	while(position < buf_len)
	{
		string Token = GetToken(buffer, position);

		if(Token == "*NAME")
		{
			Token = GetToken(buffer, position);
#ifdef DEBUGEQG
			printf(".zon name is %s\n", Token.c_str());
#endif
			sprintf(ZonName, "%s.dat", Token.c_str());
			break;
		}
	}

	this->datloader.Open(nullptr, ZonName, archive);

	this->model_data.zone_model = datloader.model_data.zone_model;

	model_loaders = new TERLoader[this->datloader.model_data.ModelNames.size() + 1];

	this->model_data.models = new Model *[this->datloader.model_data.ModelNames.size() + 1];

	this->model_data.placeable = new Placeable *[this->datloader.model_data.PlaceableList.size()];

	for(unsigned int i = 0; i < this->datloader.model_data.ModelNames.size(); ++i)
	{
		char tmp[200];
		sprintf(tmp, "%s.mod", this->datloader.model_data.ModelNames[i].c_str());

		char *str = tmp;

		while(*str) {
			if(*str >= 'A' && *str <= 'Z')
				*str += 'a' - 'A';
			++str;
		}

		if(model_loaders[i].Open(nullptr, tmp, archive))
		{
			this->model_data.models[i] = new Model;
			this->model_data.models[i]->vert_count = model_loaders[i].model_data.zone_model->vert_count;
			this->model_data.models[i]->poly_count = model_loaders[i].model_data.zone_model->poly_count;
			this->model_data.models[i]->tex_count = model_loaders[i].model_data.zone_model->tex_count;
			this->model_data.models[i]->verts = model_loaders[i].model_data.zone_model->verts;
			this->model_data.models[i]->polys = model_loaders[i].model_data.zone_model->polys;
			this->model_data.models[i]->tex = model_loaders[i].model_data.zone_model->tex;
			this->model_data.models[i]->name = new char[100];
			strcpy(this->model_data.models[i]->name, tmp);

			this->model_data.models[i]->IncludeInMap = true;

			// Attempt to cut down on the .map file size by defaulting some objects to not be included. The user can
			// always change this in the azone.ini
			//
			// For example, taking out all the tak_braziers from elddar saves around 30MB
			//
			if(strstr(tmp, "tree") || strstr(tmp, "pine") || strstr(tmp, "palm") || strstr(tmp, "rock") ||
			   strstr(tmp, "shrub") || strstr(tmp, "fern") || strstr(tmp, "bamboo") || strstr(tmp, "coral") ||
			   strstr(tmp, "camp_bones") || strstr(tmp, "sponge") || strstr(tmp, "plant") || strstr(tmp, "shortplm") ||
			   strstr(tmp, "tak_brazier") || strstr(tmp, "tak_banner") || strstr(tmp, "fung") || strstr(tmp, "bolete") ||
			   strstr(tmp, "amanita") || strstr(tmp, "leopita"))
			{
				if(!strstr(tmp, "arch"))
				{
					this->model_data.models[i]->IncludeInMap = false;
				}
			}
		}
		else
		{
			printf("Unable to open model %s, but continuing.\n", tmp);
			this->model_data.models[i] = new Model;
			this->model_data.models[i]->IncludeInMap = false;
		}
	}

#ifdef DEBUGPLAC
	printf("Placeable list:\n");

	for(int i = 0; i < this->datloader.model_data.ObjectGroups.size(); ++i)
	{
		printf("ObjectGroup: %i\n", i);
		printf(" XYZ: %8.3f, %8.3f, %8.3f  Tile: (%8.3f, %8.3f, %8.3f)  Rots: (%8.3f, %8.3f, %8.3f) Scale: %8.3f\n",
			this->datloader.model_data.ObjectGroups[i].x,
			this->datloader.model_data.ObjectGroups[i].y,
			this->datloader.model_data.ObjectGroups[i].z,
			this->datloader.model_data.ObjectGroups[i].TileX,
			this->datloader.model_data.ObjectGroups[i].TileY,
			this->datloader.model_data.ObjectGroups[i].TileZ,
			this->datloader.model_data.ObjectGroups[i].RotX,
			this->datloader.model_data.ObjectGroups[i].RotY,
			this->datloader.model_data.ObjectGroups[i].RotZ,
			this->datloader.model_data.ObjectGroups[i].ScaleX);

		list<int>::iterator ModelIterator;

		ModelIterator = this->datloader.model_data.ObjectGroups[i].SubObjects.begin();

		while(ModelIterator != this->datloader.model_data.ObjectGroups[i].SubObjects.end())
		{
			printf("  Uses Placeable: %i\n", (*ModelIterator));
			printf("   %s\n",
				this->datloader.model_data.ModelNames[this->datloader.model_data.PlaceableList[(*ModelIterator)].model].c_str());
			printf("   Model: %3i XYZ: (%8.3f, %8.3f, %8.3f) ROT: (%8.3f, %8.3f, %8.3f) SCALE: (%8.3f, %8.3f, %8.3f)\n",
				this->datloader.model_data.PlaceableList[(*ModelIterator)].model,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].x,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].y,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].z,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].rx,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].ry,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].rz,
				this->datloader.model_data.PlaceableList[(*ModelIterator)].scale[0],
				this->datloader.model_data.PlaceableList[(*ModelIterator)].scale[1],
				this->datloader.model_data.PlaceableList[(*ModelIterator)].scale[2]);


			++ModelIterator;
		}
	}
#endif

	for(unsigned int i = 0; i < this->datloader.model_data.PlaceableList.size(); ++i)
	{
		this->model_data.placeable[i] = new Placeable;

		this->model_data.placeable[i]->model = this->datloader.model_data.PlaceableList[i].model;

		this->model_data.placeable[i]->x = this->datloader.model_data.PlaceableList[i].x;
		this->model_data.placeable[i]->y = this->datloader.model_data.PlaceableList[i].y;
		this->model_data.placeable[i]->z = this->datloader.model_data.PlaceableList[i].z;

		this->model_data.placeable[i]->rx = this->datloader.model_data.PlaceableList[i].rx;
		this->model_data.placeable[i]->ry = this->datloader.model_data.PlaceableList[i].ry;
		this->model_data.placeable[i]->rz = this->datloader.model_data.PlaceableList[i].rz;

		this->model_data.placeable[i]->scale[0] = this->datloader.model_data.PlaceableList[i].scale[0];
		this->model_data.placeable[i]->scale[1] = this->datloader.model_data.PlaceableList[i].scale[1];
		this->model_data.placeable[i]->scale[2] = this->datloader.model_data.PlaceableList[i].scale[2];
	}

	this->model_data.ObjectGroups = this->datloader.model_data.ObjectGroups;

	this->model_data.plac_count = this->datloader.model_data.PlaceableList.size();

	this->model_data.model_count = this->datloader.model_data.ModelNames.size();

	this->status = 1;

	return 1;
}


int Zonv4Loader::Close()
{
	int i;

	if(!this->status)
		return 1;

	//TODO: Free up all the memory we used

	this->datloader.Close();

	for(i = 0; i < this->model_data.model_count; ++i)
		this->model_loaders[i].Close();

	delete[] this->model_loaders;

	this->status = 0;

	return 1;
}
