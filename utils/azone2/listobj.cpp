/*

  List Placeable Objects in an S3D or EQG.
  By Derision, based on OpenEQ File Loaders by Daeken et al.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef          short SHORT;
typedef unsigned long  DWORD;

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "wld.hpp"

#include "archive.hpp"
#include "pfs.hpp"

#include "file_loader.hpp"
#include "zon.hpp"
#include "ter.hpp"
#include "zonv4.hpp"



bool ProcessZoneFile(const char *shortname);
void ListPlaceable(FileLoader *fileloader, char *ZoneFileName);
void ListPlaceableV4(FileLoader *fileloader, char *ZoneFileName);
enum EQFileType { S3D, EQG, UNKNOWN };


int main(int argc, char *argv[]) {

	printf("LISTOBJ: List Placeable Objects in .S3D or .EQG zone files.\n");

	if(argc != 2) {
		printf("Usage: %s (zone short name)\n", argv[0]);
		return(1);
	}

	return(ProcessZoneFile(argv[1]));
}

bool ProcessZoneFile(const char *shortname) {

	char bufs[96];
  	Archive *archive;
  	FileLoader *fileloader;
  	Zone_Model *zm;
	FILE *fff;
	EQFileType FileType = UNKNOWN;

	sprintf(bufs, "%s.s3d", shortname);

	archive = new PFSLoader();
	fff = fopen(bufs, "rb");
	if(fff != nullptr)
		FileType = S3D;
	else {
		sprintf(bufs, "%s.eqg", shortname);
		fff = fopen(bufs, "rb");
		if(fff != nullptr)
			FileType = EQG;
	}

	if(FileType == UNKNOWN) {
		printf("Unable to locate %s.s3d or %s.eqg\n", shortname, shortname);
		return(false);
	}

  	if(archive->Open(fff) == 0) {
		printf("Unable to open container file '%s'\n", bufs);
		return(false);
	}

	bool V4Zone = false;

	switch(FileType) {
		case S3D:
  			fileloader = new WLDLoader();
  			if(fileloader->Open(nullptr, (char *) shortname, archive) == 0) {
	  			printf("Error reading WLD from %s\n", bufs);
	  			return(false);
  			}
			break;

		case EQG:
			fileloader = new ZonLoader();
			if(fileloader->Open(nullptr, (char *) shortname, archive) == 0) {
				delete fileloader;
				fileloader = new Zonv4Loader();
				if(fileloader->Open(nullptr, (char *) shortname, archive) == 0) {
					printf("Error reading ZON/TER from %s\n", bufs);
					return(false);
				}
				V4Zone = true;
	        	}
			break;
		case UNKNOWN:
			break;
	}


	zm = fileloader->model_data.zone_model;

 	if(!V4Zone)
		ListPlaceable(fileloader, bufs);
	else
		ListPlaceableV4(fileloader, bufs);

	return(true);
}

void ListPlaceable(FileLoader *fileloader, char *ZoneFileName) {

	for(int i = 0; i < fileloader->model_data.plac_count; ++i) {
		if(fileloader->model_data.placeable[i]->model==-1) continue;
		if(fileloader->model_data.models[fileloader->model_data.placeable[i]->model] == nullptr) continue;
		printf("Placeable Object %4d @ (%9.2f, %9.2f, %9.2f uses model %4d %s\n",i,
	       	 	fileloader->model_data.placeable[i]->y,
	        	fileloader->model_data.placeable[i]->x,
	        	fileloader->model_data.placeable[i]->z,
			fileloader->model_data.placeable[i]->model,
			fileloader->model_data.models[fileloader->model_data.placeable[i]->model]->name);
	}
}


void ListPlaceableV4(FileLoader *fileloader, char *ZoneFileName)
{

	float XOffset, YOffset, ZOffset;
	float RotX, RotY, RotZ;
	float XScale, YScale, ZScale;


	vector<ObjectGroupEntry>::iterator Iterator;

	int OGNum = 0;

	Iterator = fileloader->model_data.ObjectGroups.begin();

	while(Iterator != fileloader->model_data.ObjectGroups.end())
	{
		printf("ObjectGroup Number: %i\n", OGNum++);

		printf("ObjectGroup Coordinates: %8.3f, %8.3f, %8.3f\n",
			(*Iterator).x, (*Iterator).y, (*Iterator).z);

		printf("Tile: %8.3f, %8.3f, %8.3f\n",
			(*Iterator).TileX, (*Iterator).TileY, (*Iterator).TileZ);

		printf("ObjectGroup Rotations  : %8.3f, %8.3f, %8.3f\n",
			(*Iterator).RotX, (*Iterator).RotY, (*Iterator).RotZ);

		printf("ObjectGroup Scale      : %8.3f, %8.3f, %8.3f\n",
			(*Iterator).ScaleX, (*Iterator).ScaleY, (*Iterator).ScaleZ);

		list<int>::iterator ModelIterator;

		ModelIterator = (*Iterator).SubObjects.begin();

		while(ModelIterator != (*Iterator).SubObjects.end())
		{
			int SubModel = (*ModelIterator);

			Model *model = fileloader->model_data.models[fileloader->model_data.placeable[SubModel]->model];

			printf("  SubModel: %i [Model: %i, %s] ", SubModel, fileloader->model_data.placeable[SubModel]->model, model->name );
			printf("Default to include in map: %s\n", model->IncludeInMap ? "YES" : "NO");

			XOffset = fileloader->model_data.placeable[SubModel]->x;
			YOffset = fileloader->model_data.placeable[SubModel]->y;
			ZOffset = fileloader->model_data.placeable[SubModel]->z;

			printf("   Translations: %8.3f, %8.3f, %8.3f\n", XOffset, YOffset, ZOffset);
			printf("   Rotations   : %8.3f, %8.3f, %8.3f\n", fileloader->model_data.placeable[SubModel]->rx,
				fileloader->model_data.placeable[SubModel]->ry, fileloader->model_data.placeable[SubModel]->rz);
			printf("   Scale       : %8.3f, %8.3f, %8.3f\n", fileloader->model_data.placeable[SubModel]->scale[0],
				fileloader->model_data.placeable[SubModel]->scale[1], fileloader->model_data.placeable[SubModel]->scale[2]);


			RotX = fileloader->model_data.placeable[SubModel]->rx * 3.14159 / 180;  // Convert from degrees to radians
			RotY = fileloader->model_data.placeable[SubModel]->ry * 3.14159 / 180;
			RotZ = fileloader->model_data.placeable[SubModel]->rz * 3.14159 / 180;

			XScale = fileloader->model_data.placeable[SubModel]->scale[0];
			YScale = fileloader->model_data.placeable[SubModel]->scale[1];
			ZScale = fileloader->model_data.placeable[SubModel]->scale[2];

			++ModelIterator;
		}
		printf("\n");
		Iterator++;
	}


}
