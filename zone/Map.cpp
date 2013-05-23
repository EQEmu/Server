/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/debug.h"
#include "../common/MiscFunctions.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#ifndef WIN32
//comment this out if your worried about zone boot times and your not using valgrind
#define SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
#endif

#include "zone_profile.h"
#include "map.h"
#include "zone.h"
#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

extern Zone* zone;

//Do we believe the normals from the map file?
//you want this enabled if it dosent break things.
//#define TRUST_MAPFILE_NORMALS

//#define OPTIMIZE_QT_LOOKUPS
#define EPS 0.002f	//acceptable error

//#define DEBUG_SEEK 1
//#define DEBUG_BEST_Z 1

/*
 of note:
 it is possible to get a null node in a valid region if it
 does not have any triangles in it.
 this will dictate bahaviour on getting a null node
 TODO: listen to the above
 */

//quick functions to clean up vertex code.
#define Vmin3(o, a, b, c) ((a.o<b.o)? (a.o<c.o?a.o:c.o) : (b.o<c.o?b.o:c.o))
#define Vmax3(o, a, b, c) ((a.o>b.o)? (a.o>c.o?a.o:c.o) : (b.o>c.o?b.o:c.o))
#define ABS(x) ((x)<0?-(x):(x))

Map* Map::LoadMapfile(const char* in_zonename, const char *directory) {
	FILE *fp;
	char zBuf[64];
	char cWork[256];
	Map* ret = 0;

	//have to convert to lower because the short names im getting
	//are not all lower anymore, copy since strlwr edits the str.
	strn0cpy(zBuf, in_zonename, 64);

	if(directory == nullptr)
		directory = MAP_DIR;
	snprintf(cWork, 250, "%s/%s.map", directory, strlwr(zBuf));

	if ((fp = fopen( cWork, "rb" ))) {
		ret = new Map();
		if(ret != nullptr) {
			ret->loadMap(fp);
			printf("Map %s loaded.\n", cWork);
		} else {
			printf("Map %s loading failed.\n", cWork);
		}
		fclose(fp);
	}
	else {
		printf("Map %s not found.\n", cWork);
	}
	return ret;
}

Map::Map() {
	_minz = FLT_MAX;
	_minx = FLT_MAX;
	_miny = FLT_MAX;
	_maxz = FLT_MIN;
	_maxx = FLT_MIN;
	_maxy = FLT_MIN;

	m_Faces = 0;
	m_Nodes = 0;
	m_FaceLists = 0;
	mFinalFaces = nullptr;
	mNodes = nullptr;
	mFaceLists = nullptr;
}

bool Map::loadMap(FILE *fp) {
#ifndef INVERSEXY
#warning Map files do not work without inverted XY
	return(false);
#endif

	mapHeader head;
	if(fread(&head, sizeof(head), 1, fp) != 1) {
		//map read error.
		return(false);
	}
	if(head.version != MAP_VERSION) {
		//invalid version... if there really are multiple versions,
		//a conversion routine could be possible.
		printf("Invalid map version 0x%lx\n", (unsigned long)head.version);
		return(false);
	}

	printf("Map header: %lu faces, %u nodes, %lu facelists\n", (unsigned long)head.face_count, (unsigned int)head.node_count, (unsigned long)head.facelist_count);

	m_Faces = head.face_count;
	m_Nodes = head.node_count;
	m_FaceLists = head.facelist_count;

	/*	fread(&m_Vertex, 4, 1, fp);
	fread(&m_Faces , 4, 1, fp);*/
//	mFinalVertex = new VERTEX[m_Vertex];
	mFinalFaces = new FACE	[m_Faces];
	mNodes = new NODE[m_Nodes];
	mFaceLists = new uint32[m_FaceLists];

//	fread(mFinalVertex, m_Vertex, sizeof(VERTEX), fp);

	//this was changed to this loop from the single read because valgrind was
	//hanging on this read otherwise... I dont pretend to understand it.
#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	uint32 r;
	for(r = 0; r < m_Faces; r++) {
		if(fread(mFinalFaces+r, sizeof(FACE), 1, fp) != 1) {
			printf("Unable to read %lu faces from map file, got %lu.\n", (unsigned long)m_Faces, (unsigned long)r);
			return(false);
		}
	}
#else
	uint32 count;
	if((count = static_cast<int>(fread(mFinalFaces, sizeof(FACE), m_Faces , fp))) != m_Faces) {
		printf("Unable to read %lu face bytes from map file, got %lu.\n", (unsigned long)m_Faces, (unsigned long)count);
		return(false);
	}
#endif

#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	for(r = 0; r < m_Nodes; r++) {
		if(fread(mNodes+r, sizeof(NODE), 1, fp) != 1) {
			printf("Unable to read %lu nodes from map file, got %lu.\n", (unsigned long)m_Nodes, (unsigned long)r);
			return(false);
		}
	}
#else
	if(fread(mNodes, sizeof(NODE), m_Nodes, fp) != m_Nodes) {
		printf("Unable to read %lu nodes from map file.\n", (unsigned long)m_Nodes);
		return(false);
	}
#endif

#ifdef SLOW_AND_CRAPPY_MAKES_VALGRIND_HAPPY
	for(r = 0; r < m_FaceLists; r++) {
		if(fread(mFaceLists+r, sizeof(uint32), 1, fp) != 1) {
			printf("Unable to read %lu face lists from map file, got %lu.\n", (unsigned long)m_FaceLists, (unsigned long)r);
			return(false);
		}
	}
#else
	if(fread(mFaceLists, sizeof(uint32), m_FaceLists, fp) != m_FaceLists) {
		printf("Unable to read %lu face lists from map file.\n", (unsigned long)m_FaceLists);
		return(false);
	}
#endif


/*	mRoot = new NODE();
	RecLoadNode(mRoot, fp );*/

	uint32 i;
	float v;
	for(i = 0; i < m_Faces; i++) {
		v = Vmax3(x, mFinalFaces[i].a, mFinalFaces[i].b, mFinalFaces[i].c);
		if(v > _maxx)
			_maxx = v;
		v = Vmin3(x, mFinalFaces[i].a, mFinalFaces[i].b, mFinalFaces[i].c);
		if(v < _minx)
			_minx = v;
		v = Vmax3(y, mFinalFaces[i].a, mFinalFaces[i].b, mFinalFaces[i].c);
		if(v > _maxy)
			_maxy = v;
		v = Vmin3(y, mFinalFaces[i].a, mFinalFaces[i].b, mFinalFaces[i].c);
		if(v < _miny)
			_miny = v;
		v = Vmax3(z, mFinalFaces[i].a, mFinalFaces[i].b, mFinalFaces[i].c);
		if(v > _maxz)
			_maxz = v;
		v = Vmin3(z, mFinalFaces[i].a, mFinalFaces[i].b, mFinalFaces[i].c);
		if(v < _minz)
			_minz = v;
	}
	printf("Loaded map: %lu vertices, %lu faces\n", (unsigned long)m_Faces*3, (unsigned long)m_Faces);
	printf("Map BB: (%.2f -> %.2f, %.2f -> %.2f, %.2f -> %.2f)\n", _minx, _maxx, _miny, _maxy, _minz, _maxz);
	return(true);
}

Map::~Map() {
//	safe_delete_array(mFinalVertex);
	safe_delete_array(mFinalFaces);
	safe_delete_array(mNodes);
	safe_delete_array(mFaceLists);
//	RecFreeNode( mRoot );
}


NodeRef Map::SeekNode( NodeRef node_r, float x, float y ) const {
	if(node_r == NODE_NONE || node_r >= m_Nodes) {
		return(NODE_NONE);
	}
	PNODE _node = &mNodes[node_r];
#ifdef DEBUG_SEEK
printf("Seeking node for %u:(%.2f, %.2f) with root 0x%x.\n", node_r, x, y, _node);

printf("	Current Box: (%.2f -> %.2f, %.2f -> %.2f)\n", _node->minx, _node->maxx, _node->miny, _node->maxy);
#endif
	if( x>= _node->minx && x<= _node->maxx && y>= _node->miny && y<= _node->maxy ) {
		if( _node->flags & nodeFinal ) {
#ifdef DEBUG_SEEK
printf("Seeking node for %u:(%.2f, %.2f) with root 0x%x.\n", node_r, x, y, _node);
printf("	Final Node: (%.2f -> %.2f, %.2f -> %.2f)\n", _node->minx, _node->maxx, _node->miny, _node->maxy);
fflush(stdout);
printf("	Final node found with %d faces.\n", _node->faces.count);
/*printf("	Faces:\n");
unsigned long *cfl = mFaceLists + _node->faces.offset;
unsigned long m;
for(m = 0; m < _node->faces.count; m++) {
	FACE *c = &mFinalFaces[ *cfl ];
	printf("	%lu (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f)\n",
				*cfl, c->a.x, c->a.y, c->a.z,
				c->b.x, c->b.y, c->b.z,
				c->c.x, c->c.y, c->c.z);
	cfl++;
}*/
#endif
			return node_r;
		}
#ifdef DEBUG_SEEK
printf("	Kids: %u, %u, %u, %u\n", _node->nodes[0], _node->nodes[1], _node->nodes[2], _node->nodes[3]);

printf("	Contained In Box: (%.2f -> %.2f, %.2f -> %.2f)\n", _node->minx, _node->maxx, _node->miny, _node->maxy);

/*printf("	Node found has children.\n");
if(_node->node1 != nullptr) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n",
		_node->node1->minx, _node->node1->maxx, _node->node1->miny, _node->node1->maxy);
}
if(_node->node2 != nullptr) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n",
		_node->node2->minx, _node->node2->maxx, _node->node2->miny, _node->node2->maxy);
}
if(_node->node3 != nullptr) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n",
		_node->node3->minx, _node->node3->maxx, _node->node3->miny, _node->node3->maxy);
}
if(_node->node4 != nullptr) {
	printf("\tNode: (%.2f -> %.2f, %.2f -> %.2f)\n",
		_node->node4->minx, _node->node4->maxx, _node->node4->miny, _node->node4->maxy);
}*/
#endif
		//NOTE: could precalc these and store them in node headers

		NodeRef tmp = NODE_NONE;
#ifdef OPTIMIZE_QT_LOOKUPS
		float midx = _node->minx + (_node->maxx - _node->minx) * 0.5;
		float midy = _node->miny + (_node->maxy - _node->miny) * 0.5;
		//follow ordering rules from map.h...
		if(x < midx) {
			if(y < midy) { //quad 3
				if(_node->nodes[2] != NODE_NONE && _node->nodes[2] != node_r)
					tmp = SeekNode( _node->nodes[2], x, y );
			} else {	//quad 2
				if(_node->nodes[2] != NODE_NONE && _node->nodes[1] != node_r)
					tmp = SeekNode( _node->nodes[1], x, y );
			}
		} else {
			if(y < midy) { //quad 4
				if(_node->nodes[2] != NODE_NONE && _node->nodes[3] != node_r)
					tmp = SeekNode( _node->nodes[3], x, y );
			} else {	//quad 1
				if(_node->nodes[2] != NODE_NONE && _node->nodes[0] != node_r)
					tmp = SeekNode( _node->nodes[0], x, y );
			}
		}
		if( tmp != NODE_NONE ) return tmp;
#else
		if(_node->nodes[0] == node_r) return(NODE_NONE);	//prevent infinite recursion
		tmp = SeekNode( _node->nodes[0], x, y );
		if( tmp != NODE_NONE ) return tmp;
		if(_node->nodes[1] == node_r) return(NODE_NONE);	//prevent infinite recursion
		tmp = SeekNode( _node->nodes[1], x, y );
		if( tmp != NODE_NONE ) return tmp;
		if(_node->nodes[2] == node_r) return(NODE_NONE);	//prevent infinite recursion
		tmp = SeekNode( _node->nodes[2], x, y );
		if( tmp != NODE_NONE ) return tmp;
		if(_node->nodes[3] == node_r) return(NODE_NONE);	//prevent infinite recursion
		tmp = SeekNode( _node->nodes[3], x, y );
		if( tmp != NODE_NONE ) return tmp;
#endif

	}
#ifdef DEBUG_SEEK
printf(" No node found.\n");
#endif
	return(NODE_NONE);
}

// maybe precalc edges.
int* Map::SeekFace( NodeRef node_r, float x, float y ) {
	if( node_r == NODE_NONE || node_r >= m_Nodes) {
		return(nullptr);
	}
	const PNODE _node = &mNodes[node_r];
	if(!(_node->flags & nodeFinal)) {
		return(nullptr); //not a final node... could find the proper node...
	}


//printf("Seeking face for (%.2f, %.2f) with root 0x%x.\n", x, y, _node);
	float	dx,dy;
	float	nx,ny;
	int		*face = mCandFaces;
	unsigned long i;
	for( i=0;i<_node->faces.count;i++ ) {
		const FACE &cf = mFinalFaces[ mFaceLists[_node->faces.offset + i] ];
		const VERTEX &v1 = cf.a;
		const VERTEX &v2 = cf.b;
		const VERTEX &v3 = cf.c;

		dx = v2.x - v1.x; dy = v2.y - v1.y;
		nx = x - v1.x; ny = y - v1.y;
		if( dx*ny - dy*nx >0.0f ) continue;

		dx = v3.x - v2.x; dy = v3.y - v2.y;
		nx = x - v2.x; ny = y - v2.y;
		if( dx*ny - dy*nx >0.0f ) continue;

		dx = v1.x - v3.x; dy = v1.y - v3.y;
		nx = x - v3.x; ny = y - v3.y;
		if( dx*ny - dy*nx >0.0f ) continue;

		*face++ = mFaceLists[_node->faces.offset + i];
	}
	*face = -1;
	return mCandFaces;
}

// can be op?
float Map::GetFaceHeight( int _idx, float x, float y ) const {
	const PFACE	pface = &mFinalFaces[ _idx ];
	return ( pface->nd - x * pface->nx - y * pface->ny ) / pface->nz;
}

//FatherNitwit's LOS code...
//Algorithm stolen from internet
//p1=start of segment
//p2=end of segment

bool Map::LineIntersectsZone(VERTEX start, VERTEX end, float step_mag, VERTEX *result, FACE **on) const
{
	_ZP(Map_LineIntersectsZone);
	// Cast a ray from start to end, checking for collisions in each node between the two points.
	//
	float stepx, stepy, stepz, curx, cury, curz;

	curx = start.x;
	cury = start.y;
	curz = start.z;

	VERTEX cur = start;

	stepx = end.x - start.x;
	stepy = end.y - start.y;
	stepz = end.z - start.z;

	if((stepx == 0) && (stepy == 0) && (stepz == 0))
		return false;

	float factor = sqrt(stepx*stepx + stepy*stepy + stepz*stepz);

	stepx = (stepx/factor)*step_mag;
	stepy = (stepy/factor)*step_mag;
	stepz = (stepz/factor)*step_mag;

	NodeRef cnode, lnode, finalnode;
	lnode = NODE_NONE; //last node visited

	cnode = SeekNode(GetRoot(), start.x, start.y);
	finalnode = SeekNode(GetRoot(), end.x, end.y);
	if(cnode == finalnode)
		return LineIntersectsNode(cnode, start, end, result, on);

	do {

		stepx = (float)end.x - curx;
		stepy = (float)end.y - cury;
		stepz = (float)end.z - curz;

		factor = sqrt(stepx*stepx + stepy*stepy + stepz*stepz);

		stepx = (stepx/factor)*step_mag;
		stepy = (stepy/factor)*step_mag;
		stepz = (stepz/factor)*step_mag;

			cnode = SeekNode(GetRoot(), curx, cury);
		if(cnode != lnode)
		{
				lnode = cnode;

			if(cnode == NODE_NONE)
				return false;

			if(LineIntersectsNode(cnode, start, end, result, on))
					return(true);

			if(cnode == finalnode)
				return false;
		}
		curx += stepx;
		cury += stepy;
		curz += stepz;

		cur.x = curx;
		cur.y = cury;
		cur.z = curz;

		if(ABS(curx - end.x) < step_mag) cur.x = end.x;
		if(ABS(cury - end.y) < step_mag) cur.y = end.y;
		if(ABS(curz - end.z) < step_mag) cur.z = end.z;

	} while(cur.x != end.x || cur.y != end.y || cur.z != end.z);

	return false;
}

bool Map::LocWithinNode( NodeRef node_r, float x, float y ) const {
	if( node_r == NODE_NONE || node_r >= m_Nodes) {
		return(false);
	}
	const PNODE _node = &mNodes[node_r];
	//this function exists so nobody outside of MAP needs to know
	//how the NODE sturcture works
	return( x>= _node->minx && x<= _node->maxx && y>= _node->miny && y<= _node->maxy );
}

bool Map::LineIntersectsNode( NodeRef node_r, VERTEX p1, VERTEX p2, VERTEX *result, FACE **on) const {
	_ZP(Map_LineIntersectsNode);
	if( node_r == NODE_NONE || node_r >= m_Nodes) {
		return(true); //can see through empty nodes, just allow LOS on error...
	}
	const PNODE _node = &mNodes[node_r];
	if(!(_node->flags & nodeFinal)) {
		return(true); //not a final node... not sure best action
	}

	unsigned long i;

	PFACE cur;
	const uint32 *cfl = mFaceLists + _node->faces.offset;

	for(i = 0; i < _node->faces.count; i++) {
		if(*cfl > m_Faces)
			continue;	//watch for invalid lists, they seem to happen
		cur = &mFinalFaces[ *cfl ];
		if(LineIntersectsFace(cur,p1, p2, result)) {
			if(on != nullptr)
				*on = cur;
			return(true);
		}
		cfl++;
	}

//printf("Checked %ld faces and found none in the way.\n", i);

	return(false);
}


float Map::FindBestZ( NodeRef node_r, VERTEX p1, VERTEX *result, FACE **on) const {
	_ZP(Map_FindBestZ);

	p1.z += RuleI(Map, FindBestZHeightAdjust);

	if(RuleB(Map, UseClosestZ))
		return FindClosestZ(p1);

	if(node_r == GetRoot()) {
		node_r = SeekNode(node_r, p1.x, p1.y);
	}
	if( node_r == NODE_NONE || node_r >= m_Nodes) {
		return(BEST_Z_INVALID);
	}
	const PNODE _node = &mNodes[node_r];
	if(!(_node->flags & nodeFinal)) {
		return(BEST_Z_INVALID); //not a final node... could find the proper node...
	}

	VERTEX tmp_result;	//dummy placeholder if they do not ask for a result.
	if(result == nullptr)
		result = &tmp_result;

	VERTEX p2(p1);
	p2.z = BEST_Z_INVALID;

	float best_z = BEST_Z_INVALID;
	int zAttempt;

	unsigned long i;

	PFACE cur;

	// If we don't find a bestZ on the first attempt, we try again from a position CurrentZ + 10 higher
	// This is in case the pathing between waypoints temporarily sends the NPC below ground level.
	//
	for(zAttempt=1; zAttempt<=2; zAttempt++) {

		const uint32 *cfl = mFaceLists + _node->faces.offset;

#ifdef DEBUG_BEST_Z
printf("Start finding best Z...\n");
#endif
		for(i = 0; i < _node->faces.count; i++) {
			if(*cfl > m_Faces)
				continue; //watch for invalid lists, they seem to happen, e.g. in eastwastes.map

			cur = &mFinalFaces[ *cfl ];
//printf("Intersecting with face %lu\n", *cfl);
			if(LineIntersectsFace(cur, p1, p2, result)) {
#ifdef DEBUG_BEST_Z
					printf("  %lu (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f) (%.2f, %.2f, %.2f)\n",
					*cfl, cur->a.x, cur->a.y, cur->a.z,
					cur->b.x, cur->b.y, cur->b.z,
					cur->c.x, cur->c.y, cur->c.z);
					printf("Found a z: %.2f\n", result->z);
#endif
				if (result->z > best_z) {
					if(on != nullptr)
						*on = cur;
					best_z = result->z;
				}
			}
			cfl++;
		}

		if(best_z != BEST_Z_INVALID) return best_z;

		p1.z = p1.z + 10 ; // If we can't find a best Z, the NPC is probably just under the world. Try again from 10 units higher up.
	}

#ifdef DEBUG_BEST_Z
fflush(stdout);
printf("Best Z found: %.2f\n", best_z);
#endif
	return best_z;
}


bool Map::LineIntersectsFace( PFACE cface, VERTEX p1, VERTEX p2, VERTEX *result) const {
	if( cface == nullptr ) {
		return(false); //cant intersect a face we dont have... i guess
	}

	const VERTEX &pa = cface->a;
	const VERTEX &pb = cface->b;
	const VERTEX &pc = cface->c;

	//quick bounding box checks
	float tbb;

	tbb = Vmin3(x, pa, pb, pc);
	if(p1.x < tbb && p2.x < tbb)
		return(false);
	tbb = Vmin3(y, pa, pb, pc);
	if(p1.y < tbb && p2.y < tbb)
		return(false);
	tbb = Vmin3(z, pa, pb, pc);
	if(p1.z < tbb && p2.z < tbb)
		return(false);

	tbb = Vmax3(x, pa, pb, pc);
	if(p1.x > tbb && p2.x > tbb)
		return(false);
	tbb = Vmax3(y, pa, pb, pc);
	if(p1.y > tbb && p2.y > tbb)
		return(false);
	tbb = Vmax3(z, pa, pb, pc);
	if(p1.z > tbb && p2.z > tbb)
		return(false);

	//begin attempt 2
//#define RTOD 57.2957795	//radians to degrees constant.

	float d;
	float denom,mu;
	VERTEX n, intersect;

//	FACE *thisface = &mFinalFaces[ _node->pfaces[ i ] ];

	VERTEX *p = &intersect;
	if(result != nullptr)
		p = result;

	// Calculate the parameters for the plane
	//recalculate from points
#ifndef TRUST_MAPFILE_NORMALS
	n.x = (pb.y - pa.y)*(pc.z - pa.z) - (pb.z - pa.z)*(pc.y - pa.y);
	n.y = (pb.z - pa.z)*(pc.x - pa.x) - (pb.x - pa.x)*(pc.z - pa.z);
	n.z = (pb.x - pa.x)*(pc.y - pa.y) - (pb.y - pa.y)*(pc.x - pa.x);
	Normalize(&n);
	d = - n.x * pa.x - n.y * pa.y - n.z * pa.z;
#else
	//use precaled data from .map file
	n.x = cface->nx;
	n.y = cface->ny;
	n.z = cface->nz;
	d = cface->nd;
#endif

	//try inverting the normals...
	n.x = -n.x;
	n.y = -n.y;
	n.z = -n.z;
	d = - n.x * pa.x - n.y * pa.y - n.z * pa.z;	//recalc


	// Calculate the position on the line that intersects the plane
	denom = n.x * (p2.x - p1.x) + n.y * (p2.y - p1.y) + n.z * (p2.z - p1.z);
	if (ABS(denom) < EPS) // Line and plane don't intersect
		return(false);
	mu = - (d + n.x * p1.x + n.y * p1.y + n.z * p1.z) / denom;
	if (mu < 0 || mu > 1) // Intersection not along line segment
		return(false);
	p->x = p1.x + mu * (p2.x - p1.x);
	p->y = p1.y + mu * (p2.y - p1.y);
	p->z = p1.z + mu * (p2.z - p1.z);


/*	//old method, slow as hell due to acos(), but it works well

	float a1,a2,a3;
	float total;
	VERTEX pa1,pa2,pa3;

	pa1.x = pa.x - p->x;
	pa1.y = pa.y - p->y;
	pa1.z = pa.z - p->z;
	Normalize(&pa1);
	pa2.x = pb.x - p->x;
	pa2.y = pb.y - p->y;
	pa2.z = pb.z - p->z;
	Normalize(&pa2);
	pa3.x = pc.x - p->x;
	pa3.y = pc.y - p->y;
	pa3.z = pc.z - p->z;
	Normalize(&pa3);
	a1 = pa1.x*pa2.x + pa1.y*pa2.y + pa1.z*pa2.z;
	a2 = pa2.x*pa3.x + pa2.y*pa3.y + pa2.z*pa3.z;
	a3 = pa3.x*pa1.x + pa3.y*pa1.y + pa3.z*pa1.z;

//holy hell these 3 acos are slow, we need to rewrite this...
//	total = (acos(a1) + acos(a2) + acos(a3));
//	if (ABS(total - 2*M_PI) > EPS)
	total = (acos(a1) + acos(a2) + acos(a3)) * 57.2957795;
	if (ABS(total - 360) > EPS)
		return(false);

	return(true);
*/

/*
	//yet another failed method, project triangle and point into
	//2 space based on largest component of the normal
	//and check the triangle there.
	float tx, ty, tz;
	if(n.x < 0)
		tx = -n.x;
	else
		tx = n.x;
	if(n.y < 0)
		ty = -n.y;
	else
		ty = n.y;
	if(n.z < 0)
		tz = -n.z;
	else
		tz = n.z;

	VERTEX pa2, pb2, pc2;
	if(tx < ty) {
		//keep x
		if(tz < ty) {
			//keep z, drop y
			pa2.x = pa.x; pa2.y = pa.z;
			pb2.x = pb.x; pb2.y = pb.z;
			pc2.x = pc.x; pc2.y = pc.z;
		} else {
			//keep y, drop z...
			pa2.x = pa.x; pa2.y = pa.y;
			pb2.x = pb.x; pb2.y = pb.y;
			pc2.x = pc.x; pc2.y = pc.y;
		}
	} else {
		//keep y
		if(tz < tx) {
			//keep z, drop x
			pa2.x = pa.x; pa2.y = pa.z;
			pb2.x = pb.x; pb2.y = pb.z;
			pc2.x = pc.x; pc2.y = pc.z;
		} else {
			//keep y, drop z...
			pa2.x = pa.x; pa2.y = pa.y;
			pb2.x = pb.x; pb2.y = pb.y;
			pc2.x = pc.x; pc2.y = pc.y;
		}
	}

	// Determine whether or not the intersection point is bounded by pa,pb,pc
#define Sign(p1, p2, p3) \
	((p1->x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1->y - p3.y))
	bool b1, b2, b3;

	b1 = Sign(p, pa2, pb2) < 0.0f;
	b2 = Sign(p, pb2, pc2) < 0.0f;
	b3 = Sign(p, pc2, pa2) < 0.0f;

	return ((b1 == b2) && (b2 == b3));
*/

/*	//not working well, seems to block LOS a lot

	//a new check based on barycentric coordinates, stolen from
	//http://www.flipcode.com/cgi-bin/fcmsg.cgi?thread_show=7766
	float xcp, ycp, xab, yab, xac, yac;
	float divb;
	VERTEX barycoords;

	xcp = p->x - pc.x;
	ycp = p->z - pc.z;
	if( xcp == 0.f && ycp == 0.f )
		return(true);

	xab = pb.x - pa.x;
	yab = pb.z - pa.z;
	divb = xab * ycp - yab * xcp;
	if( divb == 0.f )
		return(false);

	xac = pc.x - pa.x;
	yac = pc.z - pa.z;

	barycoords.y = ( -yac * xcp + xac * ycp ) / divb;
	if( barycoords.y < -EPS || barycoords.y > (1+EPS) )
		return(false); // small error tolerance
	if( barycoords.y < 0.f )
		barycoords.y = 0.f;
	if( barycoords.y > 1.f )
		barycoords.y = 1.f;

//	barycoords.x = 1.f - barycoords.y;

	if( xcp != 0.f ) {
		float div = -xac + barycoords.y * xab;
		if( div == 0.f )
			return(false); // flat triangle
		barycoords.z = 1.f - xcp / div ;
	} else {
		float div = -yac + barycoords.y * yab;
		if( div == 0.f )
			return(false); // flat triangle
		barycoords.z = 1.f - ycp / div ;
	}

	if( barycoords.z < -EPS || barycoords.z > (1+EPS) )
		return(false);
//	if( barycoords.z < 0.f )
//		barycoords.z = 0.f;
//	if( barycoords.z > 1.f )
//		barycoords.z = 1.f;

//	barycoords.x *= 1.f - barycoords.z;
//	barycoords.y *= 1.f - barycoords.z;

	return(true);
*/

/*
	Yet another method adapted from this code:
	Vec3 pa1 = pa - p;
	Vec3 pa2 = pb - p;
	float d = pa1.cross(pa2).dot(n);
	if (d < 0) return false;
	Vec3 pa3 = pb - p;
	d = pa2.cross(pa3).dot(n);
	if (d < 0) return false;
	d = pa3.cross(pa1).dot(n);
	if (d < 0) return false;
	return true;
*/

	//in practice, this seems to actually take longer
	//than the arc cosine method above...
	n.x = -n.x;
	n.y = -n.y;
	n.z = -n.z;
	VERTEX pa1,pa2,pa3, tmp;
	float t;

	//pa1 = pa - p
	pa1.x = pa.x - p->x;
	pa1.y = pa.y - p->y;
	pa1.z = pa.z - p->z;

	//pa2 = pb - p
	pa2.x = pb.x - p->x;
	pa2.y = pb.y - p->y;
	pa2.z = pb.z - p->z;

	//tmp = pa1 cross pa2
	tmp.x = pa1.y * pa2.z - pa1.z * pa2.y;
	tmp.y = pa1.z * pa2.x - pa1.x * pa2.z;
	tmp.z = pa1.x * pa2.y - pa1.y * pa2.x;

	//t = tmp dot n
	t = tmp.x * n.x + tmp.y * n.y + tmp.z * n.z;
	if(t < 0)
		return(false);
//printf("t = %f\n", t);

	//pa3 = pb - p
	pa3.x = pc.x - p->x;
	pa3.y = pc.y - p->y;
	pa3.z = pc.z - p->z;

	//tmp = pa2 cross pa3
	tmp.x = pa2.y * pa3.z - pa2.z * pa3.y;
	tmp.y = pa2.z * pa3.x - pa2.x * pa3.z;
	tmp.z = pa2.x * pa3.y - pa2.y * pa3.x;

	//t = tmp dot n
	t = tmp.x * n.x + tmp.y * n.y + tmp.z * n.z;
	if(t < 0)
		return(false);
//printf("t = %f\n", t);

	//tmp = pa3 cross pa1
	tmp.x = pa3.y * pa1.z - pa3.z * pa1.y;
	tmp.y = pa3.z * pa1.x - pa3.x * pa1.z;
	tmp.z = pa3.x * pa1.y - pa3.y * pa1.x;

	//t = tmp dot n
	t = tmp.x * n.x + tmp.y * n.y + tmp.z * n.z;
	if(t < 0)
		return(false);
//printf("t = %f\n", t);

	return(true);
}

void Map::Normalize(VERTEX *p) {
	float len = sqrtf(p->x*p->x + p->y*p->y + p->z*p->z);
	p->x /= len;
	p->y /= len;
	p->z /= len;
}

bool Map::LineIntersectsZoneNoZLeaps(VERTEX start, VERTEX end, float step_mag, VERTEX *result, FACE **on) {
	float z = -999999;
	VERTEX step;
	VERTEX cur;
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

	NodeRef cnode, lnode;
	lnode = 0;
	//while we are not past end
	//always do this once, even if start == end.
	while(cur.x != end.x || cur.y != end.y || cur.z != end.z)
	{
		steps++;
		cnode = SeekNode(GetRoot(), cur.x, cur.y);
		if (cnode == NODE_NONE)
		{
			return(true);
		}
		VERTEX me;
		me.x = cur.x;
		me.y = cur.y;
		me.z = cur.z;
		VERTEX hit;
		FACE *onhit;
		float best_z = zone->zonemap->FindBestZ(cnode, me, &hit, &onhit);
		float diff = ABS(best_z-z);
//		diff *= sign(diff);
		if (z == -999999 || best_z == -999999 || diff < 12.0)
			z = best_z;
		else
			return(true);
		//look at current location
		if(cnode != NODE_NONE && cnode != lnode) {
			if(LineIntersectsNode(cnode, start, end, result, on))
			{
				return(true);
			}
			lnode = cnode;
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
	return(false);
}

float Map::FindClosestZ(VERTEX p ) const
{
	// Unlike FindBestZ, this method finds the closest Z value above or below the specified point.
	//
	std::list<float> ZSet;

	NodeRef NodeR = SeekNode(MAP_ROOT_NODE, p.x, p.y);

	if( NodeR == NODE_NONE || NodeR >= m_Nodes)
		return 0;

	PNODE CurrentNode = &mNodes[NodeR];

	if(!(CurrentNode->flags & nodeFinal))
		return 0;

	VERTEX p1(p), p2(p), Result;

	p1.z = 999999;

	p2.z = BEST_Z_INVALID;

	const uint32 *CurrentFaceList = mFaceLists + CurrentNode->faces.offset;

	for(unsigned long i = 0; i < CurrentNode->faces.count; ++i)
	{
		if(*CurrentFaceList > m_Faces)
			continue;

		PFACE CurrentFace = &mFinalFaces[ *CurrentFaceList ];

		if(CurrentFace->nz > 0 && LineIntersectsFace(CurrentFace, p1, p2, &Result))
			ZSet.push_back(Result.z);

		CurrentFaceList++;

	}
	if(ZSet.size() == 0)
		return 0;

	if(ZSet.size() == 1)
		return ZSet.front();

	float ClosestZ = -999999;

	for(std::list<float>::iterator Iterator = ZSet.begin(); Iterator != ZSet.end(); ++Iterator)
	{
		if(ABS(p.z - (*Iterator)) < ABS(p.z - ClosestZ))
				ClosestZ = (*Iterator);
	}

	return ClosestZ;
}

