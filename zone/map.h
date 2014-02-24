/*
	EQEMu:  Everquest Server Emulator

	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

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

#ifndef ZONE_MAP_H
#define ZONE_MAP_H

#include <stdio.h>
#include <string>

#define BEST_Z_INVALID -99999

class Map
{
public:
#pragma pack(1)
	struct Vertex
	{
		Vertex() : x(0.0f), y(0.0f), z(0.0f) { }
		Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }
		~Vertex() { }
		bool operator==(const Vertex &v) const 
		{
			return((v.x == x) && (v.y == y) && (v.z == z));
		}

		float x;
		float y;
		float z;
	};
#pragma pack()

	Map();
	~Map();
	
	float FindBestZ(Vertex &start, Vertex *result) const;
	bool LineIntersectsZone(Vertex start, Vertex end, float step, Vertex *result) const;
	bool LineIntersectsZoneNoZLeaps(Vertex start, Vertex end, float step_mag, Vertex *result) const;
	bool CheckLoS(Vertex myloc, Vertex oloc) const;
	bool Load(std::string filename);
	static Map *LoadMapFile(std::string file);
private:
	bool LoadV1(FILE *f);
	bool LoadV2(FILE *f);
	
	struct impl;
	impl *imp;
};

#endif
