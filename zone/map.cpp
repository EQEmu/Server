#include "../common/global_define.h"
#include "../common/misc_functions.h"

#include "map.h"
#include "raycast_mesh.h"
#include "zone.h"

#include <algorithm>
#include <map>
#include <memory>
#include <tuple>
#include <vector>
#include <zlib.h>

uint32 InflateData(const char* buffer, uint32 len, char* out_buffer, uint32 out_len_max) {
	z_stream zstream;
	int zerror = 0;
	int i;

	zstream.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(buffer));
	zstream.avail_in = len;
	zstream.next_out = reinterpret_cast<unsigned char*>(out_buffer);;
	zstream.avail_out = out_len_max;
	zstream.zalloc = Z_NULL;
	zstream.zfree = Z_NULL;
	zstream.opaque = Z_NULL;

	i = inflateInit2(&zstream, 15);
	if (i != Z_OK) {
		return 0;
	}

	zerror = inflate(&zstream, Z_FINISH);
	if (zerror == Z_STREAM_END) {
		inflateEnd(&zstream);
		return zstream.total_out;
	}
	else {
		if (zerror == -4 && zstream.msg == 0)
		{
			return 0;
		}

		zerror = inflateEnd(&zstream);
		return 0;
	}
}

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
		safe_delete(imp);
	}
}

float Map::FindBestZ(glm::vec3 &start, glm::vec3 *result) const {
	if (!imp)
		return BEST_Z_INVALID;

	glm::vec3 tmp;
	if(!result)
		result = &tmp;

	start.z += RuleI(Map, FindBestZHeightAdjust);
	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
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

float Map::FindClosestZ(glm::vec3 &start, glm::vec3 *result) const {
	// Unlike FindBestZ, this method finds the closest Z value above or below the specified point.
	//
	if (!imp)
		return false;
	
	float ClosestZ = BEST_Z_INVALID;
	
	glm::vec3 tmp;
	if (!result)
		result = &tmp;
	
	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;
	
	// first check is below us
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit) {
		ClosestZ = result->z;
		
	}
	
	// Find nearest Z above us
	to.z = -BEST_Z_INVALID;
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance);
	if (hit) {
		if (abs(from.z - result->z) < abs(ClosestZ - from.z))
			return result->z;
	}

	return ClosestZ;
}

bool Map::LineIntersectsZone(glm::vec3 start, glm::vec3 end, float step, glm::vec3 *result) const {
	if(!imp)
		return false;
	return imp->rm->raycast((const RmReal*)&start, (const RmReal*)&end, (RmReal*)result, nullptr, nullptr);
}

bool Map::LineIntersectsZoneNoZLeaps(glm::vec3 start, glm::vec3 end, float step_mag, glm::vec3 *result) const {
	if (!imp)
		return false;
	
	float z = BEST_Z_INVALID;
	glm::vec3 step;
	glm::vec3 cur;
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
		glm::vec3 me;
		me.x = cur.x;
		me.y = cur.y;
		me.z = cur.z;
		glm::vec3 hit;

		float best_z = FindBestZ(me, &hit);
		float diff = best_z - z;
		diff = diff < 0 ? -diff : diff;

		if (z <= BEST_Z_INVALID || best_z <= BEST_Z_INVALID || diff < 12.0)
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

bool Map::CheckLoS(glm::vec3 myloc, glm::vec3 oloc) const {
	if(!imp)
		return false;

	return !imp->rm->raycast((const RmReal*)&myloc, (const RmReal*)&oloc, nullptr, nullptr, nullptr);
}

Map *Map::LoadMapFile(std::string file) {
	std::string filename = Config->MapDir;
	filename += "/";
	std::transform(file.begin(), file.end(), file.begin(), ::tolower);
	filename += file;
	filename += ".map";

	auto m = new Map();
	if (m->Load(filename)) {
		return m;
	}

	delete m;
	return nullptr;
}

bool Map::Load(std::string filename) {
	FILE *f = fopen(filename.c_str(), "rb");
	if(f) {
		uint32 version;
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
	uint32 face_count;
	uint16 node_count;
	uint32 facelist_count;
	
	if(fread(&face_count, sizeof(face_count), 1, f) != 1) {
		return false;
	}
	
	if(fread(&node_count, sizeof(node_count), 1, f) != 1) {
		return false;
	}
	
	if(fread(&facelist_count, sizeof(facelist_count), 1, f) != 1) {
		return false;
	}
	
	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;
	for(uint32 i = 0; i < face_count; ++i) {
		glm::vec3 a;
		glm::vec3 b;
		glm::vec3 c;
		float normals[4];
		if(fread(&a, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if(fread(&b, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if(fread(&c, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if(fread(normals, sizeof(normals), 1, f) != 1) {
			return false;
		}

		size_t sz = verts.size();
		verts.push_back(a);
		indices.push_back((uint32)sz);

		verts.push_back(b);
		indices.push_back((uint32)sz + 1);

		verts.push_back(c);
		indices.push_back((uint32)sz + 2);
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

struct ModelEntry
{
	struct Poly
	{
		uint32 v1, v2, v3;
		uint8 vis;
	};
	std::vector<glm::vec3> verts;
	std::vector<Poly> polys;
};

bool Map::LoadV2(FILE *f) {
	uint32 data_size;
	if (fread(&data_size, sizeof(data_size), 1, f) != 1) {
		return false;
	}

	uint32 buffer_size;
	if (fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
		return false;
	}

	std::vector<char> data;
	data.resize(data_size);
	if (fread(&data[0], data_size, 1, f) != 1) {
		return false;
	}

	std::vector<char> buffer;
	buffer.resize(buffer_size);
	uint32 v = InflateData(&data[0], data_size, &buffer[0], buffer_size);

	char *buf = &buffer[0];
	uint32 vert_count;
	uint32 ind_count;
	uint32 nc_vert_count;
	uint32 nc_ind_count;
	uint32 model_count;
	uint32 plac_count;
	uint32 plac_group_count;
	uint32 tile_count;
	uint32 quads_per_tile;
	float units_per_vertex;

	vert_count = *(uint32*)buf;
	buf += sizeof(uint32);

	ind_count = *(uint32*)buf;
	buf += sizeof(uint32);

	nc_vert_count = *(uint32*)buf;
	buf += sizeof(uint32);

	nc_ind_count = *(uint32*)buf;
	buf += sizeof(uint32);

	model_count = *(uint32*)buf;
	buf += sizeof(uint32);

	plac_count = *(uint32*)buf;
	buf += sizeof(uint32);

	plac_group_count = *(uint32*)buf;
	buf += sizeof(uint32);

	tile_count = *(uint32*)buf;
	buf += sizeof(uint32);

	quads_per_tile = *(uint32*)buf;
	buf += sizeof(uint32);

	units_per_vertex = *(float*)buf;
	buf += sizeof(float);

	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;

	for (uint32 i = 0; i < vert_count; ++i) {
		float x;
		float y;
		float z;

		x = *(float*)buf;
		buf += sizeof(float);

		y = *(float*)buf;
		buf += sizeof(float);

		z = *(float*)buf;
		buf += sizeof(float);

		glm::vec3 vert(x, y, z);
		verts.push_back(vert);
	}

	for (uint32 i = 0; i < ind_count; ++i) {
		uint32 index;
		index = *(uint32*)buf;
		buf += sizeof(uint32);

		indices.push_back(index);
	}

	for (uint32 i = 0; i < nc_vert_count; ++i) {
		buf += sizeof(float) * 3;
	}

	for (uint32 i = 0; i < nc_ind_count; ++i) {
		buf += sizeof(uint32);
	}

	std::map<std::string, std::unique_ptr<ModelEntry>> models;
	for (uint32 i = 0; i < model_count; ++i) {
		std::unique_ptr<ModelEntry> me(new ModelEntry);
		std::string name = buf;
		buf += name.length() + 1;

		uint32 vert_count = *(uint32*)buf;
		buf += sizeof(uint32);

		uint32 poly_count = *(uint32*)buf;
		buf += sizeof(uint32);

		me->verts.resize(vert_count);
		for (uint32 j = 0; j < vert_count; ++j) {
			float x = *(float*)buf;
			buf += sizeof(float);
			float y = *(float*)buf;
			buf += sizeof(float);
			float z = *(float*)buf;
			buf += sizeof(float);

			me->verts[j] = glm::vec3(x, y, z);
		}

		me->polys.resize(poly_count);
		for (uint32 j = 0; j < poly_count; ++j) {
			uint32 v1 = *(uint32*)buf;
			buf += sizeof(uint32);
			uint32 v2 = *(uint32*)buf;
			buf += sizeof(uint32);
			uint32 v3 = *(uint32*)buf;
			buf += sizeof(uint32);
			uint8 vis = *(uint8*)buf;
			buf += sizeof(uint8);

			ModelEntry::Poly p;
			p.v1 = v1;
			p.v2 = v2;
			p.v3 = v3;
			p.vis = vis;
			me->polys[j] = p;
		}

		models[name] = std::move(me);
	}

	for (uint32 i = 0; i < plac_count; ++i) {
		std::string name = buf;
		buf += name.length() + 1;

		float x = *(float*)buf;
		buf += sizeof(float);
		float y = *(float*)buf;
		buf += sizeof(float);
		float z = *(float*)buf;
		buf += sizeof(float);

		float x_rot = *(float*)buf;
		buf += sizeof(float);
		float y_rot = *(float*)buf;
		buf += sizeof(float);
		float z_rot = *(float*)buf;
		buf += sizeof(float);

		float x_scale = *(float*)buf;
		buf += sizeof(float);
		float y_scale = *(float*)buf;
		buf += sizeof(float);
		float z_scale = *(float*)buf;
		buf += sizeof(float);

		if (models.count(name) == 0)
			continue;

		auto &model = models[name];
		auto &mod_polys = model->polys;
		auto &mod_verts = model->verts;
		for (uint32 j = 0; j < mod_polys.size(); ++j) {
			auto &current_poly = mod_polys[j];
			auto v1 = mod_verts[current_poly.v1];
			auto v2 = mod_verts[current_poly.v2];
			auto v3 = mod_verts[current_poly.v3];

			RotateVertex(v1, x_rot, y_rot, z_rot);
			RotateVertex(v2, x_rot, y_rot, z_rot);
			RotateVertex(v3, x_rot, y_rot, z_rot);

			ScaleVertex(v1, x_scale, y_scale, z_scale);
			ScaleVertex(v2, x_scale, y_scale, z_scale);
			ScaleVertex(v3, x_scale, y_scale, z_scale);

			TranslateVertex(v1, x, y, z);
			TranslateVertex(v2, x, y, z);
			TranslateVertex(v3, x, y, z);

			float t = v1.x;
			v1.x = v1.y;
			v1.y = t;

			t = v2.x;
			v2.x = v2.y;
			v2.y = t;

			t = v3.x;
			v3.x = v3.y;
			v3.y = t;

			if (current_poly.vis != 0) {
				verts.push_back(v1);
				verts.push_back(v2);
				verts.push_back(v3);

				indices.push_back((uint32)verts.size() - 3);
				indices.push_back((uint32)verts.size() - 2);
				indices.push_back((uint32)verts.size() - 1);
			}
		}
	}

	for (uint32 i = 0; i < plac_group_count; ++i) {
		float x = *(float*)buf;
		buf += sizeof(float);
		float y = *(float*)buf;
		buf += sizeof(float);
		float z = *(float*)buf;
		buf += sizeof(float);

		float x_rot = *(float*)buf;
		buf += sizeof(float);
		float y_rot = *(float*)buf;
		buf += sizeof(float);
		float z_rot = *(float*)buf;
		buf += sizeof(float);

		float x_scale = *(float*)buf;
		buf += sizeof(float);
		float y_scale = *(float*)buf;
		buf += sizeof(float);
		float z_scale = *(float*)buf;
		buf += sizeof(float);

		float x_tile = *(float*)buf;
		buf += sizeof(float);
		float y_tile = *(float*)buf;
		buf += sizeof(float);
		float z_tile = *(float*)buf;
		buf += sizeof(float);

		uint32 p_count = *(uint32*)buf;
		buf += sizeof(uint32);

		for (uint32 j = 0; j < p_count; ++j) {
			std::string name = buf;
			buf += name.length() + 1;

			float p_x = *(float*)buf;
			buf += sizeof(float);
			float p_y = *(float*)buf;
			buf += sizeof(float);
			float p_z = *(float*)buf;
			buf += sizeof(float);

			float p_x_rot = *(float*)buf * 3.14159f / 180;
			buf += sizeof(float);
			float p_y_rot = *(float*)buf * 3.14159f / 180;
			buf += sizeof(float);
			float p_z_rot = *(float*)buf * 3.14159f / 180;
			buf += sizeof(float);

			float p_x_scale = *(float*)buf;
			buf += sizeof(float);
			float p_y_scale = *(float*)buf;
			buf += sizeof(float);
			float p_z_scale = *(float*)buf;
			buf += sizeof(float);

			if (models.count(name) == 0)
				continue;

			auto &model = models[name];

			for (size_t k = 0; k < model->polys.size(); ++k) {
				auto &poly = model->polys[k];
				glm::vec3 v1, v2, v3;

				v1 = model->verts[poly.v1];
				v2 = model->verts[poly.v2];
				v3 = model->verts[poly.v3];

				ScaleVertex(v1, p_x_scale, p_y_scale, p_z_scale);
				ScaleVertex(v2, p_x_scale, p_y_scale, p_z_scale);
				ScaleVertex(v3, p_x_scale, p_y_scale, p_z_scale);

				TranslateVertex(v1, p_x, p_y, p_z);
				TranslateVertex(v2, p_x, p_y, p_z);
				TranslateVertex(v3, p_x, p_y, p_z);

				RotateVertex(v1, x_rot * 3.14159f / 180.0f, 0, 0);
				RotateVertex(v2, x_rot * 3.14159f / 180.0f, 0, 0);
				RotateVertex(v3, x_rot * 3.14159f / 180.0f, 0, 0);

				RotateVertex(v1, 0, y_rot * 3.14159f / 180.0f, 0);
				RotateVertex(v2, 0, y_rot * 3.14159f / 180.0f, 0);
				RotateVertex(v3, 0, y_rot * 3.14159f / 180.0f, 0);

				glm::vec3 correction(p_x, p_y, p_z);

				RotateVertex(correction, x_rot * 3.14159f / 180.0f, 0, 0);

				TranslateVertex(v1, -correction.x, -correction.y, -correction.z);
				TranslateVertex(v2, -correction.x, -correction.y, -correction.z);
				TranslateVertex(v3, -correction.x, -correction.y, -correction.z);

				RotateVertex(v1, p_x_rot, 0, 0);
				RotateVertex(v2, p_x_rot, 0, 0);
				RotateVertex(v3, p_x_rot, 0, 0);

				RotateVertex(v1, 0, -p_y_rot, 0);
				RotateVertex(v2, 0, -p_y_rot, 0);
				RotateVertex(v3, 0, -p_y_rot, 0);

				RotateVertex(v1, 0, 0, p_z_rot);
				RotateVertex(v2, 0, 0, p_z_rot);
				RotateVertex(v3, 0, 0, p_z_rot);

				TranslateVertex(v1, correction.x, correction.y, correction.z);
				TranslateVertex(v2, correction.x, correction.y, correction.z);
				TranslateVertex(v3, correction.x, correction.y, correction.z);

				RotateVertex(v1, 0, 0, z_rot * 3.14159f / 180.0f);
				RotateVertex(v2, 0, 0, z_rot * 3.14159f / 180.0f);
				RotateVertex(v3, 0, 0, z_rot * 3.14159f / 180.0f);

				ScaleVertex(v1, x_scale, y_scale, z_scale);
				ScaleVertex(v2, x_scale, y_scale, z_scale);
				ScaleVertex(v3, x_scale, y_scale, z_scale);

				TranslateVertex(v1, x_tile, y_tile, z_tile);
				TranslateVertex(v2, x_tile, y_tile, z_tile);
				TranslateVertex(v3, x_tile, y_tile, z_tile);

				TranslateVertex(v1, x, y, z);
				TranslateVertex(v2, x, y, z);
				TranslateVertex(v3, x, y, z);

				float t = v1.x;
				v1.x = v1.y;
				v1.y = t;

				t = v2.x;
				v2.x = v2.y;
				v2.y = t;

				t = v3.x;
				v3.x = v3.y;
				v3.y = t;

				if (poly.vis != 0) {
					verts.push_back(v1);
					verts.push_back(v2);
					verts.push_back(v3);

					indices.push_back((uint32)verts.size() - 3);
					indices.push_back((uint32)verts.size() - 2);
					indices.push_back((uint32)verts.size() - 1);
				}
			}
		}
	}

	uint32 ter_quad_count = (quads_per_tile * quads_per_tile);
	uint32 ter_vert_count = ((quads_per_tile + 1) * (quads_per_tile + 1));
	std::vector<uint8> flags;
	std::vector<float> floats;
	flags.resize(ter_quad_count);
	floats.resize(ter_vert_count);
	for (uint32 i = 0; i < tile_count; ++i) {
		bool flat;
		flat = *(bool*)buf;
		buf += sizeof(bool);

		float x;
		x = *(float*)buf;
		buf += sizeof(float);

		float y;
		y = *(float*)buf;
		buf += sizeof(float);

		if (flat) {
			float z;
			z = *(float*)buf;
			buf += sizeof(float);

			float QuadVertex1X = x;
			float QuadVertex1Y = y;
			float QuadVertex1Z = z;

			float QuadVertex2X = QuadVertex1X + (quads_per_tile * units_per_vertex);
			float QuadVertex2Y = QuadVertex1Y;
			float QuadVertex2Z = QuadVertex1Z;

			float QuadVertex3X = QuadVertex2X;
			float QuadVertex3Y = QuadVertex1Y + (quads_per_tile * units_per_vertex);
			float QuadVertex3Z = QuadVertex1Z;

			float QuadVertex4X = QuadVertex1X;
			float QuadVertex4Y = QuadVertex3Y;
			float QuadVertex4Z = QuadVertex1Z;

			uint32 current_vert = (uint32)verts.size() + 3;
			verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
			verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
			verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
			verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));

			indices.push_back(current_vert);
			indices.push_back(current_vert - 2);
			indices.push_back(current_vert - 1);

			indices.push_back(current_vert);
			indices.push_back(current_vert - 3);
			indices.push_back(current_vert - 2);
		}
		else {
			//read flags
			for (uint32 j = 0; j < ter_quad_count; ++j) {
				uint8 f;
				f = *(uint8*)buf;
				buf += sizeof(uint8);

				flags[j] = f;
			}

			//read floats
			for (uint32 j = 0; j < ter_vert_count; ++j) {
				float f;
				f = *(float*)buf;
				buf += sizeof(float);

				floats[j] = f;
			}

			int row_number = -1;
			std::map<std::tuple<float, float, float>, uint32> cur_verts;
			for (uint32 quad = 0; quad < ter_quad_count; ++quad) {
				if ((quad % quads_per_tile) == 0) {
					++row_number;
				}

				if (flags[quad] & 0x01)
					continue;

				float QuadVertex1X = x + (row_number * units_per_vertex);
				float QuadVertex1Y = y + (quad % quads_per_tile) * units_per_vertex;
				float QuadVertex1Z = floats[quad + row_number];

				float QuadVertex2X = QuadVertex1X + units_per_vertex;
				float QuadVertex2Y = QuadVertex1Y;
				float QuadVertex2Z = floats[quad + row_number + quads_per_tile + 1];

				float QuadVertex3X = QuadVertex1X + units_per_vertex;
				float QuadVertex3Y = QuadVertex1Y + units_per_vertex;
				float QuadVertex3Z = floats[quad + row_number + quads_per_tile + 2];

				float QuadVertex4X = QuadVertex1X;
				float QuadVertex4Y = QuadVertex1Y + units_per_vertex;
				float QuadVertex4Z = floats[quad + row_number + 1];

				uint32 i1, i2, i3, i4;
				std::tuple<float, float, float> t = std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z);
				auto iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i1 = iter->second;
				}
				else {
					i1 = (uint32)verts.size();
					verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
					cur_verts[std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z)] = i1;
				}

				t = std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i2 = iter->second;
				}
				else {
					i2 = (uint32)verts.size();
					verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
					cur_verts[std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z)] = i2;
				}

				t = std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i3 = iter->second;
				}
				else {
					i3 = (uint32)verts.size();
					verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
					cur_verts[std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z)] = i3;
				}

				t = std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i4 = iter->second;
				}
				else {
					i4 = (uint32)verts.size();
					verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));
					cur_verts[std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z)] = i4;
				}

				indices.push_back(i4);
				indices.push_back(i2);
				indices.push_back(i3);

				indices.push_back(i4);
				indices.push_back(i1);
				indices.push_back(i2);
			}
		}
	}

	uint32 face_count = indices.size() / 3;

	if (imp) {
		imp->rm->release();
		imp->rm = nullptr;
	}
	else {
		imp = new impl;
	}

	imp->rm = createRaycastMesh((RmUint32)verts.size(), (const RmReal*)&verts[0], face_count, &indices[0]);

	if (!imp->rm) {
		delete imp;
		imp = nullptr;
		return false;
	}

	return true;
}

void Map::RotateVertex(glm::vec3 &v, float rx, float ry, float rz) {
	glm::vec3 nv = v;

	nv.y = (cos(rx) * v.y) - (sin(rx) * v.z);
	nv.z = (sin(rx) * v.y) + (cos(rx) * v.z);

	v = nv;

	nv.x = (cos(ry) * v.x) + (sin(ry) * v.z);
	nv.z = -(sin(ry) * v.x) + (cos(ry) * v.z);

	v = nv;

	nv.x = (cos(rz) * v.x) - (sin(rz) * v.y);
	nv.y = (sin(rz) * v.x) + (cos(rz) * v.y);

	v = nv;
}

void Map::ScaleVertex(glm::vec3 &v, float sx, float sy, float sz) {
	v.x = v.x * sx;
	v.y = v.y * sy;
	v.z = v.z * sz;
}

void Map::TranslateVertex(glm::vec3 &v, float tx, float ty, float tz) {
	v.x = v.x + tx;
	v.y = v.y + ty;
	v.z = v.z + tz;
}
