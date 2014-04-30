#include "../common/debug.h"
#include "../common/MiscFunctions.h"
#include "map.h"
#include "RaycastMesh.h"
#include "zone.h"
#include <stdint.h>
#include <algorithm>
#include <locale>
#include <vector>

struct Map::impl
{
	RaycastMesh *rm;
};

Map::Map() {
	imp = nullptr;
}

Map::~Map() {
	if(imp) {
		imp->rm->release();
	}
}

float Map::FindBestZ(Vertex &start, Vertex *result) const {
	if (!imp)
		return false;

	Vertex tmp;
	if(!result)
		result = &tmp;

	start.z += RuleI(Map, FindBestZHeightAdjust);
	Vertex from(start.x, start.y, start.z);
	Vertex to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;

	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if(hit) {
		return result->z;
	}
	
	// Find nearest Z above us
	
	to.z = -BEST_Z_INVALID;
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit)
	{
		return result->z;
	}
	
	return BEST_Z_INVALID;
}

bool Map::LineIntersectsZone(Vertex start, Vertex end, float step, Vertex *result) const {
	if(!imp)
		return false;
	return imp->rm->raycast((const RmReal*)&start, (const RmReal*)&end, (RmReal*)result, nullptr, nullptr);
}

bool Map::LineIntersectsZoneNoZLeaps(Vertex start, Vertex end, float step_mag, Vertex *result) const {
	if (!imp)
		return false;
	
	float z = BEST_Z_INVALID;
	Vertex step;
	Vertex cur;
	cur.x = start.x;
	cur.y = start.y;
	cur.z = start.z;

	step.x = end.x - start.x;
	step.y = end.y - start.y;
	step.z = end.z - start.z;
	float factor = step_mag / sqrt(step.x*step.x + step.y*step.y + step.z*step.z);

	step.x *= factor;
	step.y *= factor;
	step.z *= factor;

	int steps = 0;

	if (step.x > 0 && step.x < 0.001f)
	step.x = 0.001f;
	if (step.y > 0 && step.y < 0.001f)
	step.y = 0.001f;
	if (step.z > 0 && step.z < 0.001f)
	step.z = 0.001f;
	if (step.x < 0 && step.x > -0.001f)
	step.x = -0.001f;
	if (step.y < 0 && step.y > -0.001f)
	step.y = -0.001f;
	if (step.z < 0 && step.z > -0.001f)
	step.z = -0.001f;

	//while we are not past end
	//always do this once, even if start == end.
	while(cur.x != end.x || cur.y != end.y || cur.z != end.z)
	{
		steps++;
		Vertex me;
		me.x = cur.x;
		me.y = cur.y;
		me.z = cur.z;
		Vertex hit;

		float best_z = FindBestZ(me, &hit);
		float diff = best_z - z;
		diff = diff < 0 ? -diff : diff;

		if (z == -999999 || best_z == -999999 || diff < 12.0)
			z = best_z;
		else
			return true;

		//look at current location
		if(LineIntersectsZone(start, end, step_mag, result))
		{
			return true;
		}

		//move 1 step
		if (cur.x != end.x)
			cur.x += step.x;
		if (cur.y != end.y)
			cur.y += step.y;
		if (cur.z != end.z)
			cur.z += step.z;

		//watch for end conditions
		if ( (cur.x > end.x && end.x >= start.x) || (cur.x < end.x && end.x <= start.x) || (step.x == 0) ) {
			cur.x = end.x;
		}
		if ( (cur.y > end.y && end.y >= start.y) || (cur.y < end.y && end.y <= start.y) || (step.y == 0) ) {
			cur.y = end.y;
		}
		if ( (cur.z > end.z && end.z >= start.z) || (cur.z < end.z && end.z < start.z) || (step.z == 0) ) {
			cur.z = end.z;
		}
	}

	//walked entire line and didnt run into anything...
	return false;
}

bool Map::CheckLoS(Vertex myloc, Vertex oloc) const {
	if(!imp)
		return false;

	return !imp->rm->raycast((const RmReal*)&myloc, (const RmReal*)&oloc, nullptr, nullptr, nullptr);
}

Map *Map::LoadMapFile(std::string file) {
	std::string filename = MAP_DIR;
	filename += "/";
	std::transform(file.begin(), file.end(), file.begin(), ::tolower);
	filename += file;
	filename += ".map";

	Map *m = new Map();
	if (m->Load(filename)) {
		return m;
	}

	delete m;
	return nullptr;
}

bool Map::Load(std::string filename) {
	FILE *f = fopen(filename.c_str(), "rb");
	if(f) {
		uint32_t version;
		if(fread(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			return false;
		}
		
		if(version == 0x01000000) {
			bool v = LoadV1(f);
			fclose(f);
			return v;
		} else if(version == 0x02000000) {
			bool v = LoadV2(f);
			fclose(f);
			return v;
		} else {
			fclose(f);
			return false;
		}
	}
	
	return false;
}

bool Map::LoadV1(FILE *f) {
	uint32_t face_count;
	uint16_t node_count;
	uint32_t facelist_count;
	
	if(fread(&face_count, sizeof(face_count), 1, f) != 1) {
		return false;
	}
	
	if(fread(&node_count, sizeof(node_count), 1, f) != 1) {
		return false;
	}
	
	if(fread(&facelist_count, sizeof(facelist_count), 1, f) != 1) {
		return false;
	}
	
	std::vector<Vertex> verts;
	std::vector<uint32_t> indices;
	for(uint32_t i = 0; i < face_count; ++i) {
		Vertex a;
		Vertex b;
		Vertex c;
		float normals[4];
		if(fread(&a, sizeof(Vertex), 1, f) != 1) {
			return false;
		}

		if(fread(&b, sizeof(Vertex), 1, f) != 1) {
			return false;
		}

		if(fread(&c, sizeof(Vertex), 1, f) != 1) {
			return false;
		}

		if(fread(normals, sizeof(normals), 1, f) != 1) {
			return false;
		}

		size_t sz = verts.size();
		verts.push_back(a);
		indices.push_back((uint32_t)sz);

		verts.push_back(b);
		indices.push_back((uint32_t)sz + 1);

		verts.push_back(c);
		indices.push_back((uint32_t)sz + 2);
	}
	
	if(imp) {
		imp->rm->release();
		imp->rm = nullptr;
	} else {
		imp = new impl;
	}
	
	imp->rm = createRaycastMesh((RmUint32)verts.size(), (const RmReal*)&verts[0], face_count, &indices[0]);
	
	if(!imp->rm) {
		delete imp;
		imp = nullptr;
		return false;
	}
	
	return true;
}

bool Map::LoadV2(FILE *f) {
	return false;
}
