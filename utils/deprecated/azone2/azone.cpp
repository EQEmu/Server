/*

	Father Nitwit's Zone to map conversion program.
	Copyright (C) 2004 Father Nitwit (eqemu@8ass.com)

	This thing uses code from freaku, so whatever license that comes under
	is relavent, if you care.

	the rest of it is GPL, even though I hate the GPL.


	Derision: 20/06/2008
	Replaced S3D and EQG fileloaders with later OpenEQ based versions
	Added placeable object support.
	Altered order of vertices in polygons from EQG files so that map
	files produced from S3D and EQG zones are consistent in the vertex
	order and therefore surface normals.

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
#include <stdlib.h>
//#include "EQWldData.h"
#include "types.h"
#include "azone.h"
#include "wld.hpp"

#include "archive.hpp"
#include "pfs.hpp"

#include "file_loader.hpp"
#include "zon.hpp"
#include "ter.hpp"
#include "zonv4.hpp"

//TODO: I am trimming faces for which all vertices are above MAX_Z
//	but I am not taking out the vertices to go with them.


//this un-commented works with my map.cpp code correctly.
//with both of my inverts there commented.
#define INVERSEXY 1
//#define DEBUG
#include <vector>
#include <map>
using namespace std;

//#define SPLIT_DEBUG



#ifndef MAX_QUADRENT_FACES
#ifndef MIN_QUADRENT_SIZE
#error Umm... your asking for trouble by turning off both stopping criteria
#endif
#endif

int main(int argc, char *argv[]) {

	printf("AZONE2: EQEmu .MAP file generator with placeable object support.\n");

	if(argc != 2) {
		printf("Usage: %s (zone short name)\n", argv[0]);
		return(1);
	}

	char bufm[250];

	sprintf(bufm, "%s.map", argv[1]);

	QTBuilder QT;

	if(!QT.build(argv[1]))
		return(1);

	if(!QT.writeMap(bufm))
		return(1);

	return(0);
}

QTBuilder::QTBuilder() {
	_root = nullptr;

	faceCount = 0;
	faceBlock = nullptr;

#ifdef COUNT_MACTHES
	gEasyMatches = 0;
	gEasyExcludes = 0;
	gHardMatches = 0;
	gHardExcludes = 0;
#endif
}

QTBuilder::~QTBuilder() {
	if(_root != nullptr)
		delete _root;
	_root = nullptr;
	if(faceBlock != nullptr)
		delete [] faceBlock;
	faceBlock = nullptr;
}

bool QTBuilder::build(const char *shortname) {


	char bufs[96];
  	Archive *archive;
  	FileLoader *fileloader;
  	Zone_Model *zm;
	FILE *fff;
	EQFileType FileType = UNKNOWN;

	bool V4Zone = false;

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
			// Just here to stop the compiler warning
			break;
	}

	zm = fileloader->model_data.zone_model;

	long i;
	VERTEX v1, v2, v3;
	for(i = 0; i < zm->poly_count; ++i) {
#ifdef INVERSEXY
		v1.y = zm->verts[zm->polys[i]->v1]->x;
		v1.x = zm->verts[zm->polys[i]->v1]->y;
#else
		v1.x = zm->verts[zm->polys[i]->v1]->x;
		v1.y = zm->verts[zm->polys[i]->v1]->y;
#endif
		v1.z = zm->verts[zm->polys[i]->v1]->z;
#ifdef INVERSEXY
		v2.y = zm->verts[zm->polys[i]->v2]->x;
		v2.x = zm->verts[zm->polys[i]->v2]->y;
#else
		v2.x = zm->verts[zm->polys[i]->v2]->x;
		v2.y = zm->verts[zm->polys[i]->v2]->y;
#endif
		v2.z = zm->verts[zm->polys[i]->v2]->z;
#ifdef INVERSEXY
		v3.y = zm->verts[zm->polys[i]->v3]->x;
		v3.x = zm->verts[zm->polys[i]->v3]->y;
#else
		v3.x = zm->verts[zm->polys[i]->v3]->x;
		v3.y = zm->verts[zm->polys[i]->v3]->y;
#endif
		v3.z = zm->verts[zm->polys[i]->v3]->z;

		AddFace(v1, v2, v3);
	}

	printf("There are %lu vertices and %lu faces.\n", _FaceList.size()*3, _FaceList.size());

	if(fileloader->model_data.plac_count)
	{
		if(V4Zone)
		{
			vector<ObjectGroupEntry>::iterator Iterator;

			Iterator = fileloader->model_data.ObjectGroups.begin();
			AddPlaceableV4(fileloader, bufs, false);
		}
		else
			AddPlaceable(fileloader, bufs, false);
	}
	else
		printf("No placeable objects (or perhaps %s_obj.s3d not present).\n", shortname);

	printf("After processing placeable objects, there are %lu vertices and %lu faces.\n", _FaceList.size()*3, _FaceList.size());

	unsigned long r;

	faceCount = _FaceList.size();



	faceBlock = new FACE[faceCount];
	//im not going to assume I know vectors are stored in contiguous blocks
	for(r = 0; r < faceCount; r++) {
		faceBlock[r] = _FaceList[r];
	}

	//build quad tree... prolly much slower than it needs to be.
	float minx, miny, maxx, maxy;
	minx = 1e12;
	miny = 1e12;
	maxx = -1e12;
	maxy = -1e12;

	//find our limits.
	for(r = 0; r < faceCount; r++) {
		//a bit of lazyness going on here...
		{
		VERTEX &v = faceBlock[r].a;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
		{
		VERTEX &v = faceBlock[r].b;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
		{
		VERTEX &v = faceBlock[r].c;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
	}

	printf("Bounding box: %.2f < x < %.2f, %.2f < y < %.2f\n", minx, maxx, miny, maxy);

	printf("Building quadtree.\n");

	_root = new QTNode(this, minx, maxx, miny, maxy);
	if(_root == nullptr) {
		printf("Unable to allocate new QTNode.\n");
		return(false);
	}

	//build our initial set of faces... all of them:
	FACE *faceptr = faceBlock;
	_root->faces.resize(faceCount);
	for(r = 0; r < faceCount; r++) {
		_root->faces[r].face = faceptr;
		_root->faces[r].index = r;
		faceptr++;
	}

	_root->divideYourself(0);

	printf("Done building quad tree...\n");

#ifdef COUNT_MACTHES
	printf("Match counters: %lu easy in, %lu easy out, %lu hard in, %lu hard out.\n", gEasyMatches, gEasyExcludes, gHardMatches, gHardExcludes);
#endif

	fileloader->Close();

	delete fileloader;

	archive->Close();

	delete archive;

	return(true);
}



bool QTBuilder::writeMap(const char *file) {
	if(_root == nullptr)
		return(false);

	printf("Writing map file.\n");

	FILE *out = fopen(file, "wb");
	if(out == nullptr) {
		printf("Unable to open output file '%s'.\n", file);
		return(1);
	}

	mapHeader head;
	head.version = MAP_VERSION;
	head.face_count = faceCount;
	head.node_count = _root->countNodes();
	head.facelist_count = _root->countFacelists();

	if(fwrite(&head, sizeof(head), 1, out) != 1) {
		printf("Error writing map file header.\n");
		fclose(out);
		return(1);

	}

	printf("Map header: Version: 0x%08lX. %lu faces, %u nodes, %lu facelists\n", head.version, head.face_count, head.node_count, head.facelist_count);



	//write faceBlock
	if(fwrite(faceBlock, sizeof(FACE), faceCount, out) != faceCount) {
		printf("Error writing map file faces.\n");
		fclose(out);
		return(1);
	}

	//make our node blocks to write out...
	nodeHeader *nodes = new nodeHeader[head.node_count];
	unsigned long *facelist = new unsigned long[head.facelist_count];
	if(nodes == nullptr || facelist == nullptr) {
		printf("Error allocating temporary memory for output.\n");
		fclose(out);
		return(1);  //no memory
	}

	unsigned long hindex = 0;
	unsigned long findex = 0;
	_root->fillBlocks(nodes, facelist, hindex, findex);

	if(fwrite(nodes, sizeof(nodeHeader), head.node_count, out) != head.node_count) {
		printf("Error writing map file nodes.\n");
		fclose(out);
		return(1);
	}
	if(fwrite(facelist, sizeof(unsigned long), head.facelist_count, out) != head.facelist_count) {
		printf("Error writing map file face list.\n");
		fclose(out);
		return(1);
	}


	long MapFileSize = ftell(out);
	fclose(out);
	delete[] nodes;
	delete[] facelist;

	printf("Done writing map (%3.2fMB).\n", (float)MapFileSize/1048576);

	return(0);
}


QTNode::QTNode(QTBuilder *b, float Tminx, float Tmaxx, float Tminy, float Tmaxy) {
	node1 = nullptr;
	node2 = nullptr;
	node3 = nullptr;
	node4 = nullptr;
	minx = Tminx;
	maxx = Tmaxx;
	miny = Tminy;
	maxy = Tmaxy;
	final = false;
	buildVertexes();

	builder = b;
}

QTNode::~QTNode() {
	clearNodes();
}

void QTNode::clearNodes() {
	if(node1 != nullptr)
		delete node1;
	if(node2 != nullptr)
		delete node2;
	if(node3 != nullptr)
		delete node3;
	if(node4 != nullptr)
		delete node4;
	node1 = nullptr;
	node2 = nullptr;
	node3 = nullptr;
	node4 = nullptr;
}

//assumes that both supplied arrays are big enough per countNodes/Facelists
void QTNode::fillBlocks(nodeHeader *heads, unsigned long *flist, unsigned long &hindex, unsigned long &findex) {
	nodeHeader *head = &heads[hindex];
	hindex++;

	head->minx = minx;
	head->maxx = maxx;
	head->miny = miny;
	head->maxy = maxy;
	head->flags = 0;
	if(final) {
		head->flags |= nodeFinal;
		head->faces.count = faces.size();
		head->faces.offset = findex;
		unsigned long r;
		for(r = 0; r < head->faces.count; r++) {
			flist[findex] = faces[r].index;
			findex++;
		}
//		findex += head->faces.count;
	} else {
		//branch node.
		head->flags = 0;

		if(node1 != nullptr) {
			head->nodes[0] = hindex;
			node1->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[0] = NODE_NONE;
		}
		if(node2 != nullptr) {
			head->nodes[1] = hindex;
			node2->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[1] = NODE_NONE;
		}
		if(node3 != nullptr) {
			head->nodes[2] = hindex;
			node3->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[2] = NODE_NONE;
		}
		if(node4 != nullptr) {
			head->nodes[3] = hindex;
			node4->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[3] = NODE_NONE;
		}
	}
}

unsigned long QTNode::countNodes() const {
	unsigned long c = 1;
	if(node1 != nullptr)
		c += node1->countNodes();
	if(node2 != nullptr)
		c += node2->countNodes();
	if(node3 != nullptr)
		c += node3->countNodes();
	if(node4 != nullptr)
		c += node4->countNodes();
	return(c);
}

unsigned long QTNode::countFacelists() const {
	unsigned long c = final? faces.size() : 0;
	if(node1 != nullptr)
		c += node1->countFacelists();
	if(node2 != nullptr)
		c += node2->countFacelists();
	if(node3 != nullptr)
		c += node3->countFacelists();
	if(node4 != nullptr)
		c += node4->countFacelists();
	return(c);
}

/*
Map Format:

1x mapHeader (head)
head.face_count x FACE
head.node_count x nodeHeader
head.facelist_count x unsigned long (indexes into face array)


*/

void QTNode::divideYourself(int depth) {
//	printf("Dividing in box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n",
//		minx, maxx, miny, maxy, depth, faces.size());

	unsigned long cc;
	cc = faces.size();
#ifdef MAX_QUADRENT_FACES
	if(cc <= MAX_QUADRENT_FACES) {
#ifdef SPLIT_DEBUG
printf("Stopping (facecount) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n",
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif

#ifdef MIN_QUADRENT_SIZE
	if((maxx - minx) < MIN_QUADRENT_SIZE || (maxy - miny) < MIN_QUADRENT_SIZE) {
#ifdef SPLIT_DEBUG
printf("Stopping on box (size) (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n",
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif

	doSplit();

	//get counts on our split
	float c1, c2, c3, c4;
	c1 = node1? node1->faces.size() : 0;
	c2 = node2? node2->faces.size() : 0;
	c3 = node3? node3->faces.size() : 0;
	c4 = node4? node4->faces.size() : 0;

#ifdef MIN_QUADRENT_GAIN
	int miss = 0;
	float gain1 = 1.0 - c1 / cc;
	float gain2 = 1.0 - c2 / cc;
	float gain3 = 1.0 - c3 / cc;
	float gain4 = 1.0 - c4 / cc;

	//see how many missed the gain mark
	if(gain1 < MIN_QUADRENT_GAIN)
		miss++;
	if(gain2 < MIN_QUADRENT_GAIN)
		miss++;
	if(gain3 < MIN_QUADRENT_GAIN)
		miss++;
	if(gain4 < MIN_QUADRENT_GAIN)
		miss++;

	if(miss > MAX_QUADRENT_MISSES) {
#ifdef SPLIT_DEBUG
printf("Stopping (gain) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n",
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif


	//if all faces pass through all quadrents, then we are done
	//partially obsoleted by gain test.
	if(c1 == c2 && c1 == c3 && c1 == c4) {
#ifdef SPLIT_DEBUG
printf("Stopping (empty) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n",
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}

	//there are prolly some more intelligent stopping criteria...

	depth++;

	if(node1 != nullptr)
		node1->divideYourself(depth);
	if(node2 != nullptr)
		node2->divideYourself(depth);
	if(node3 != nullptr)
		node3->divideYourself(depth);
	if(node4 != nullptr)
		node4->divideYourself(depth);


}

void QTNode::buildVertexes() {

	v[0].x = v[1].x = v[2].x = v[3].x = minx;
	v[4].x = v[5].x = v[6].x = v[7].x = maxx;

	v[0].y = v[1].y = v[4].y = v[5].y = miny;
	v[2].y = v[3].y = v[6].y = v[7].y = maxy;

	v[0].z = v[3].z = v[4].z = v[7].z = -999999;
	v[1].z = v[2].z = v[5].z = v[6].z = 9999999;
}


static const GVector gNormals[6] = {
	GVector(-1.0, 0.0, 0.0),
	GVector(0.0, 1.0, 0.0),
	GVector(1.0, 0.0, 0.0),
	GVector(0.0, -1.0, 0.0),
	GVector(0.0, 0.0, 1.0),
	GVector(0.0, 0.0, -1.0),
};

static const unsigned short gIntFaces[6][4] =
{
	{0, 1, 2, 3},
	{3, 2, 6, 7},
	{7, 6, 5, 4},
	{4, 5, 1, 0},
	{5, 6, 2, 1},
	{7, 4, 0, 3}
};


//stolen in haste from my fear pathing program, and untested in here...
bool edges_cross(GPoint *pt1, GPoint *pt2, const VERTEX *pt3, const VERTEX *pt4) {
//I love macros
#define IntersectDenom(p1, p2, p3, p4) \
((p4->y - p3->y)*(p2->x - p1->x) - (p4->x - p3->x)*(p2->y - p1->y))
#define IntersectNumerX(p1, p2, p3, p4) \
((p4->x - p3->x)*(p1->y - p3->y) - (p4->y - p3->y)*(p1->x - p3->x))
#define IntersectNumerY(p1, p2, p3, p4) \
((p2->x - p1->x)*(p1->y - p3->y) - (p2->y - p1->y)*(p1->x - p3->x))

#define IntersectX(p1, p2, p3, p4, denom) \
(p1->x + IntersectNumerX(p1, p2, p3, p4)*(p2->x - p1->x)/denom)
#define IntersectY(p1, p2, p3, p4, denom) \
(p1->y + IntersectNumerX(p1, p2, p3, p4)*(p2->y - p1->y)/denom)

#define CheckEqualXY(p1, p2) \
 (p1->x == p2->x && p1->y == p2->y)

#define CoordOnLine(p1, p2, coord, dim) \
 (p1->dim > p2->dim? (coord >= p2->dim && coord <= p1->dim) : (coord >= p1->dim && coord <= p2->dim))

#define IntersectZfromX(p1, p2, inter) \
 (p2->x > p1->x? \
 (p1->z + ((inter - p1->x)/(p2->x - p1->x) * (p2->z - p1->z))) \
 :(p2->z + ((inter - p2->x)/(p1->x - p2->x) * (p1->z - p2->z))))

	float denom = IntersectDenom(pt1, pt2, pt3, pt4);
	if(denom != 0) {

		//the lines intersect, check segments now
		float xinter = IntersectX(pt1, pt2, pt3, pt4, denom);
		float yinter = IntersectY(pt1, pt2, pt3, pt4, denom);

		//now see if this point is on both segments
		if(		CoordOnLine(pt1, pt2, xinter, x)
			&&	CoordOnLine(pt1, pt2, yinter, y)
			&&	CoordOnLine(pt3, pt4, xinter, x)
			&&	CoordOnLine(pt3, pt4, yinter, y) ){
//			printf("Line (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", pt1->x, pt1->y, pt1->z, pt2->x, pt2->y, pt2->z, pt1->Dist2(pt2));
//			printf("Hits (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", pt3->x, pt3->y, pt3->z, pt4->x, pt4->y, pt4->z, pt3->Dist2(pt4));
//			printf("At (%.3f, %.3f), which IS on both segments.\n", xinter, yinter);

			return(true);
		}
//		printf("At (%.3f, %.3f), which is not on both segments.\n", xinter, yinter);
	}
	return(false);
}


#define MAX(x,y) ((x)<y?(y):(x))
#define MIN(x,y) ((x)<y?(x):(y))

bool PointInTriangle(VERTEX *polygon, float px, float py)

/*
  This code is from http://local.wasp.uwa.edu.au/~pbourke/geometry/
  Copyright Paul Bourke
  */

{
  int counter = 0;
  int i;
  double xinters;
  VERTEX p1,p2;

  p1 = polygon[0];
  for (i=1;i<=3;i++) {
    p2 = polygon[i % 3];
    if (py > MIN(p1.y,p2.y)) {
      if (py <= MAX(p1.y,p2.y)) {
        if (px <= MAX(p1.x,p2.x)) {
          if (p1.y != p2.y) {
            xinters = (py-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
            if (p1.x == p2.x || px <= xinters)
              counter++;
          }
        }
      }
    }
    p1 = p2;
  }

  if (counter % 2 == 0)
    return(false);
  else
    return(true);
}

//quick function which got too messy in the loop below.
bool QTBuilder::FaceInNode(const QTNode *q, const FACE *f) {
	const VERTEX *v1 = &f->a;
	const VERTEX *v2 = &f->b;
	const VERTEX *v3 = &f->c;

#ifdef COUNT_MACTHES
	gEasyMatches++;
#endif

	//Easy matches, points are within the quadrant.
	if( ! ( v1->x <= q->minx || v1->x > q->maxx
			|| v1->y <= q->miny || v1->y > q->maxy ) ) {
		return(true);
	}
	if( ! ( v2->x <= q->minx || v2->x > q->maxx
			|| v2->y <= q->miny || v2->y > q->maxy ) ) {
		return(true);
	}
	if( ! ( v3->x <= q->minx || v3->x > q->maxx
			|| v3->y <= q->miny || v3->y > q->maxy ) ) {
		return(true);
	}

#ifdef COUNT_MACTHES
	gEasyMatches--;
	gEasyExcludes++;
#endif

	//make sure it is even possible to insersect:
	if( v1->x < q->minx && v2->x < q->minx && v3->x < q->minx )
		return(false);
	if( v1->x > q->maxx && v2->x > q->maxx && v3->x > q->maxx )
		return(false);
	if( v1->y < q->miny && v2->y < q->miny && v3->y < q->miny )
		return(false);
	if( v1->y > q->maxy && v2->y > q->maxy && v3->y > q->maxy )
		return(false);


#ifdef COUNT_MACTHES
	gEasyExcludes--;
#endif

#ifdef COUNT_MACTHES
	gHardMatches++;
#endif
//	return(true);

	//harder: no points are in the cube

	//4 points of this node
	GPoint	pt1(q->minx, q->miny, 0),
			pt2(q->minx, q->maxy, 0),
			pt3(q->maxx, q->miny, 0),
			pt4(q->maxx, q->maxy, 0);

	/*
	//box lines:
	pt1, pt2
	pt3, pt4
	pt1, pt3
	pt2, pt4

	//tri lines
	v1, v2
	v1, v3
	v2, v3
	*/

#define CheckIntersect(p1, p2, p3, p4) \
(((p4->y - p3->y)*(p2.x - p1.x) - (p4->x - p3->x)*(p2.y - p1.y)) != 0)

	int finaltest =
		   (edges_cross(&pt1, &pt2, v1, v2)
		|| edges_cross(&pt1, &pt2, v1, v3)
		|| edges_cross(&pt1, &pt2, v2, v3)
		|| edges_cross(&pt3, &pt4, v1, v2)
		|| edges_cross(&pt3, &pt4, v1, v3)
		|| edges_cross(&pt3, &pt4, v2, v3)
		|| edges_cross(&pt1, &pt3, v1, v2)
		|| edges_cross(&pt1, &pt3, v1, v3)
		|| edges_cross(&pt1, &pt3, v2, v3)
		|| edges_cross(&pt2, &pt4, v1, v2)
		|| edges_cross(&pt2, &pt4, v1, v3)
		|| edges_cross(&pt2, &pt4, v2, v3));

	if(finaltest) return finaltest;

	VERTEX Triangle[3];
	Triangle[0]=*v1; Triangle[1]=*v2; Triangle[2] = *v3;
	finaltest = PointInTriangle(Triangle, q->minx, q->miny) ||
	            PointInTriangle(Triangle, q->minx, q->maxy) ||
		    PointInTriangle(Triangle, q->maxx, q->maxy) ||
		    PointInTriangle(Triangle, q->maxx, q->miny);

	return finaltest;
}



void QTNode::doSplit() {


	//find midpoints...
	float midx = minx + (maxx - minx) / 2.0;
	float midy = miny + (maxy - miny) / 2.0;

	//ordering following definitions in map.h
	node1 = new QTNode(builder, midx, maxx, midy, maxy);
	node2 = new QTNode(builder, minx, midx, midy, maxy);
	node3 = new QTNode(builder, minx, midx, miny, midy);
	node4 = new QTNode(builder, midx, maxx, miny, midy);
	if(node1 == nullptr || node2 == nullptr || node3 == nullptr || node4 == nullptr) {
		printf("Error: unable to allocate new QTNode, giving up.\n");
		return;
	}

	unsigned long r,l;
	l = faces.size();
	for(r = 0; r < l; r++) {
		FaceRecord &cur = faces[r];
		if(builder->FaceInNode(node1, cur.face))
			node1->faces.push_back(cur);
		if(builder->FaceInNode(node2, cur.face))
			node2->faces.push_back(cur);
		if(builder->FaceInNode(node3, cur.face))
			node3->faces.push_back(cur);
		if(builder->FaceInNode(node4, cur.face))
			node4->faces.push_back(cur);
	}

	//clean up empty sets.
	if(node1->faces.size() == 0) {
		delete node1;
		node1 = nullptr;
	}
	if(node2->faces.size() == 0) {
		delete node2;
		node2 = nullptr;
	}
	if(node3->faces.size() == 0) {
		delete node3;
		node3 = nullptr;
	}
	if(node4->faces.size() == 0) {
		delete node4;
		node4 = nullptr;
	}

}

void QTBuilder::AddFace(VERTEX &v1, VERTEX &v2, VERTEX &v3) {
	FACE f;

#ifdef MAX_Z
	if(v1.z > MAX_Z && v2.z > MAX_Z && v3.z > MAX_Z)
		return;
#endif


	//this still might not work
	f.nx = (v2.y - v1.y)*(v3.z - v1.z) - (v2.z - v1.z)*(v3.y - v1.y);
	f.ny = (v2.z - v1.z)*(v3.x - v1.x) - (v2.x - v1.x)*(v3.z - v1.z);
	f.nz = (v2.x - v1.x)*(v3.y - v1.y) - (v2.y - v1.y)*(v3.x - v1.x);
	NormalizeN(&f);
	f.nd = - f.nx * v1.x - f.ny * v1.y - f.nz * v1.z;

	f.a = v1;
	f.b = v2;
	f.c = v3;

	_FaceList.push_back(f);
}


GPoint::GPoint() {
	x = 0;
	y = 0;
	z = 0;
}

GPoint::GPoint(VERTEX &v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

GPoint::GPoint(float nx, float ny, float nz) {
	x = nx;
	y = ny;
	z = nz;
}

//dot of x,y,z
float GPoint::dot3(const GPoint &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z));
}

//cross product
GPoint GPoint::cross(const GPoint &them) const {
	return(GPoint(y * them.z - z * them.y,
					 z * them.x - x * them.z,
					 x * them.y - y * them.x));
}

GPoint operator-(const GPoint &v1, const GPoint &v2) {
	return(GPoint(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
}


GVector::GVector() : GPoint() {
	W = 0;
}

GVector::GVector(const GPoint &them) : GPoint(them) {
	W = 1.0f;
}

GVector::GVector(float x, float y, float z, float w) : GPoint(x, y, z) {
	W = w;
}

//dot product of x,y,z,w
float GVector::dot4(const GVector &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z) + (W * them.W));
}

//dot product of x,y,z+w
float GVector::dot4(const GPoint &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z) + W);
}

float GVector::length() {
	return(sqrt((x * x) + (y * y) + (z * z)));
}

void GVector::normalize() {
	float len = length();	//stupid square roots take forever
	x /= len;
	y /= len;
	z /= len;
}

//stolen from: http://gamecode.tripod.com/tut/tut04.htm
int QTBuilder::ClipPolygon(POLYGON *poly, GVector *plane) {
   /* Plan: cycle through the vertices, considering pairs of them.
      If both vertices are visible, add them to the new array.
      If both vertices are invisible, add neither to the new array.
      If one vertex is visible and the other is not, move the one
      that's not, and then add both vertices to the new array.
    */
   float dist1, dist2; // distances of points to plane
   float distratio; // fraction of distance between two points
     // new vertices might be created. Don't change
     // polygon's vertices because we might still need
     // them. Instead, use tempvtx array and update vertices
     // array at the end. Create tempvtx array once only.
   int i, ii, j=0;

   /* Check if plane is a valid plane */
   if (!(plane->x || plane->y || plane->z)) return -1;
   // if not valid plane, don't change polygon and return an error;

   /* The vertices should, as for all functions, be arranged in cyclic
   order. That is, if a line was drawn from each vertex to the next
   it would form the correct outline of the polygon in 3D space.
   This routine might create new vertices because of the clipping,
   but the cyclic order will be preserved.
   */

   for (i=0; i<poly->count; i++)
   {
      ii = (i+1)%poly->count;
      dist1 = plane->x * poly->c[i].x + plane->y * poly->c[i].y +
      plane->z * poly->c[i].z + plane->W;
      dist2 = plane->x * poly->c[ii].x + plane->y * poly->c[ii].y +
      plane->z * poly->c[ii].z + plane->W;
      if (dist1<0 && dist2<0) // line unclipped and invisible
         continue;
      if (dist1>0 && dist2>0) // line unclipped and visible
         tempvtx[j++]=poly->c[i];
      else // line partially visible
      if (dist1>0) // first vertex is visible
      {
         distratio = dist1/(dist1-dist2);
         tempvtx[j] = poly->c[i];
         j++; // Copied 1st vertex
         tempvtx[j].x = poly->c[i].x +
            (poly->c[ii].x - poly->c[i].x) * distratio;
         tempvtx[j].y = poly->c[i].y +
            (poly->c[ii].y - poly->c[i].y) * distratio;
         tempvtx[j].z = poly->c[i].z +
            (poly->c[ii].z - poly->c[i].z) * distratio;
         j++; // Copied second vertex
      }
      else // second vertex is visible
      {
         distratio = dist2/(dist2-dist1);
         tempvtx[j].x = poly->c[ii].x +
            (poly->c[i].x - poly->c[ii].x) * distratio;
         tempvtx[j].y = poly->c[ii].y +
            (poly->c[i].y - poly->c[ii].y) * distratio;
         tempvtx[j].z = poly->c[ii].z +
            (poly->c[i].z - poly->c[ii].z) * distratio;
         j++; // Copy only first vertex. 2nd vertex will be copied
              // in next iteration of loop
      }
   }

   for (i=0; i<j; i++)
      poly->c[i] = tempvtx[i]; // Update the vertices in polygon
   poly->count = j;            // Update the vertex count
   return j;
}


void QTBuilder::NormalizeN(FACE *p) {
	float len = sqrt(p->nx*p->nx + p->ny*p->ny + p->nz*p->nz);
	p->nx /= len;
	p->ny /= len;
	p->nz /= len;
}

void QTBuilder::AddPlaceable(FileLoader *fileloader, char *ZoneFileName, bool ListPlaceable) {
	Polygon *poly;
	Vertex *verts[3];
	float XOffset, YOffset, ZOffset;
	float RotX, RotY, RotZ;
	float XScale, YScale, ZScale;
	VERTEX v1, v2, v3, tmpv;


	// Ghetto ini file parser to see which models to include
	// The format of each line in azone.ini is:
	//
	// shortname.[eqg|s3d],model number, model numner, ...
	// E.g.
	// tox.s3d,1,17,34
	// anguish.eqg,25,69,70
	//

	const int IniBufferSize = 255;
	enum ReadingState { ReadingZoneName, ReadingModelNumbers };
	ReadingState State = ReadingZoneName;
	bool INIEntryFound = false;
	int INIModelCount = 0;
	char IniBuffer[IniBufferSize], ch;
	vector<int> ModelNumbers;
	int StrIndex = 0;
	int ModelNumber;

	FILE *IniFile = fopen("azone.ini", "rb");

	if(!IniFile) {
		printf("azone.ini not found in current directory. Not processing placeable models.\n");
		return;
	}
	printf("Processing azone.ini for placeable models.\n");

	while(!feof(IniFile)) {
		ch = fgetc(IniFile);
		if((ch=='#')&&(StrIndex==0)) { // Discard comment lines beginning with a hash
			while((ch!=EOF)&&(ch!='\n'))
	       	        	ch = fgetc(IniFile);

                	continue;
        	}
		if((ch=='\n') && (State==ReadingZoneName)) {
			StrIndex = 0;
			continue;
		}
    	   	if(ch=='\r') continue;
		if((ch==EOF)||(ch=='\n')) {
			IniBuffer[StrIndex] = '\0';
			if(State == ReadingModelNumbers) {
				ModelNumber = atoi(IniBuffer);
				if((ModelNumber >= 0) && (ModelNumber < fileloader->model_data.model_count))
				{
					fileloader->model_data.models[ModelNumber]->IncludeInMap = true;
					INIModelCount++;
				}
				else
					printf("ERROR: Specified model %s invalid, must be in range 0 to %i\n", IniBuffer, fileloader->model_data.model_count - 1);
			}
			break;
		}
		if(ch==',') {
			IniBuffer[StrIndex]='\0';
			StrIndex = 0;
			if(State == ReadingZoneName)  {
				if(strcmp(ZoneFileName, IniBuffer)) {
					StrIndex = 0;
					// Not our zone, skip to next line
					while((ch!=EOF)&&(ch!='\n'))
	       	        			ch = fgetc(IniFile);
					continue;
				}
				else {
					State = ReadingModelNumbers;
					INIEntryFound = true;
				}
			}
			else  {
				ModelNumber = atoi(IniBuffer);
				if((ModelNumber >= 0) && (ModelNumber < fileloader->model_data.model_count))
				{
					fileloader->model_data.models[ModelNumber]->IncludeInMap = true;
					INIModelCount++;
				}
				else
					printf("ERROR: Specified model %s invalid, must be in range 0 to %i\n", IniBuffer, fileloader->model_data.model_count - 1);
			}
			continue;
		}
		IniBuffer[StrIndex++] = tolower(ch);
	}
	fclose(IniFile);

	if(INIEntryFound) {
		printf("azone.ini entry found for this zone. ");
		if(INIModelCount > 0)
			printf("Including %d models.\n", INIModelCount);
		else
			printf("No valid model numbers specified.\n");
	}
	else {
		printf("No azone.ini entry found for zone %s\n", ZoneFileName);
	}



	for(int i = 0; i < fileloader->model_data.plac_count; ++i) {
		if(fileloader->model_data.placeable[i]->model==-1) continue;
		// The model pointer should only really be nullptr for the zone model, as we process that separately.
		if(fileloader->model_data.models[fileloader->model_data.placeable[i]->model] == nullptr) continue;
		if(ListPlaceable)
			printf("Placeable Object %4d @ (%9.2f, %9.2f, %9.2f uses model %4d %s\n",i,
		       	 	fileloader->model_data.placeable[i]->y,
		        	fileloader->model_data.placeable[i]->x,
		        	fileloader->model_data.placeable[i]->z,
				fileloader->model_data.placeable[i]->model,
				fileloader->model_data.models[fileloader->model_data.placeable[i]->model]->name);


		if(!fileloader->model_data.models[fileloader->model_data.placeable[i]->model]->IncludeInMap)
			continue;
		printf("Including Placeable Object %4d using model %4d (%s).\n", i,
			fileloader->model_data.placeable[i]->model,
			fileloader->model_data.models[fileloader->model_data.placeable[i]->model]->name);

		if(fileloader->model_data.placeable[i]->model>fileloader->model_data.model_count) continue;

		XOffset = fileloader->model_data.placeable[i]->x;
		YOffset = fileloader->model_data.placeable[i]->y;
		ZOffset = fileloader->model_data.placeable[i]->z;

		RotX = fileloader->model_data.placeable[i]->rx * 3.14159 / 180;  // Convert from degrees to radians
		RotY = fileloader->model_data.placeable[i]->ry * 3.14159 / 180;
		RotZ = fileloader->model_data.placeable[i]->rz * 3.14159 / 180;

		XScale = fileloader->model_data.placeable[i]->scale[0];
		YScale = fileloader->model_data.placeable[i]->scale[1];
		ZScale = fileloader->model_data.placeable[i]->scale[2];


		Model *model = fileloader->model_data.models[fileloader->model_data.placeable[i]->model];


		for(int j = 0; j < model->poly_count; ++j) {

			poly = model->polys[j];

			verts[0] = model->verts[poly->v1];
			verts[1] = model->verts[poly->v2];
			verts[2] = model->verts[poly->v3];

			v1.x = verts[0]->x; v1.y = verts[0]->y; v1.z = verts[0]->z;
			v2.x = verts[1]->x; v2.y = verts[1]->y; v2.z = verts[1]->z;
			v3.x = verts[2]->x; v3.y = verts[2]->y; v3.z = verts[2]->z;


			RotateVertex(v1, RotX, RotY, RotZ);
			RotateVertex(v2, RotX, RotY, RotZ);
			RotateVertex(v3, RotX, RotY, RotZ);

			ScaleVertex(v1, XScale, YScale, ZScale);
			ScaleVertex(v2, XScale, YScale, ZScale);
			ScaleVertex(v3, XScale, YScale, ZScale);

			TranslateVertex(v1, XOffset, YOffset, ZOffset);
			TranslateVertex(v2, XOffset, YOffset, ZOffset);
			TranslateVertex(v3, XOffset, YOffset, ZOffset);


			// Swap X & Y
			//
			tmpv = v1; v1.x = tmpv.y; v1.y = tmpv.x;
			tmpv = v2; v2.x = tmpv.y; v2.y = tmpv.x;
			tmpv = v3; v3.x = tmpv.y; v3.y = tmpv.x;

			AddFace(v1, v2, v3);
		}
	}
}


void QTBuilder::AddPlaceableV4(FileLoader *fileloader, char *ZoneFileName, bool ListPlaceable) {
	Polygon *poly;
	Vertex *verts[3];
	float XOffset, YOffset, ZOffset;
	float RotX, RotY, RotZ;
	float XScale, YScale, ZScale;
	VERTEX v1, v2, v3, tmpv;

	//return;

	printf("EQG V4 Placeable Zone Support\n");
	printf("ObjectGroupCount = %lu\n", fileloader->model_data.ObjectGroups.size());

	vector<ObjectGroupEntry>::iterator Iterator;

	int OGNum = 0;

	bool BailOut = false;

	// Ghetto ini file parser to see which models to include
	// The format of each line in azone.ini is:
	//
	// shortname.[eqg|s3d],model number, model numner, ...
	// E.g.
	// tox.s3d,1,17,34
	// anguish.eqg,25,69,70
	//

	const int IniBufferSize = 255;
	enum ReadingState { ReadingZoneName, ReadingModelNumbers };
	ReadingState State = ReadingZoneName;
	bool INIEntryFound = false;
	int INIModelCount = 0;
	char IniBuffer[IniBufferSize], ch;
	vector<int> ModelNumbers;
	int StrIndex = 0;
	int ModelNumber;

	FILE *IniFile = fopen("azone.ini", "rb");

	if(!IniFile)
		printf("azone.ini not found in current directory. Including default models.\n");
	else
	{
		printf("Processing azone.ini for placeable models.\n");

		while(!feof(IniFile)) {
			ch = fgetc(IniFile);
			if((ch=='#')&&(StrIndex==0)) { // Discard comment lines beginning with a hash
				while((ch!=EOF)&&(ch!='\n'))
		       	        	ch = fgetc(IniFile);

       		         	continue;
       		 	}
			if((ch=='\n') && (State==ReadingZoneName)) {
				StrIndex = 0;
				continue;
			}
    		   	if(ch=='\r') continue;
			if((ch==EOF)||(ch=='\n')) {
				IniBuffer[StrIndex] = '\0';
				if(State == ReadingModelNumbers) {
					bool Exclude = false;
					bool Group = false;
					if(IniBuffer[0]=='-')
					{
						Exclude = true;
						strcpy(IniBuffer, IniBuffer+1);
					}
					if(IniBuffer[0]=='g')
					{
						Group = true;
						strcpy(IniBuffer, IniBuffer+1);
					}
					ModelNumber = atoi(IniBuffer);
					if(!Group)
					{
						if((ModelNumber >= 0) && (ModelNumber < fileloader->model_data.model_count))
							fileloader->model_data.models[ModelNumber]->IncludeInMap = Exclude ? false : true;
						else
							printf("ERROR: Specified model %s invalid, must be in range 0 to %i\n", IniBuffer, fileloader->model_data.model_count - 1);
					}
					else
					{
						if((ModelNumber >= 0) && ((unsigned int)ModelNumber < fileloader->model_data.ObjectGroups.size()))
							fileloader->model_data.ObjectGroups[ModelNumber].IncludeInMap = Exclude ? false : true;

					}
				}
				break;
			}
			if(ch==',') {
				IniBuffer[StrIndex]='\0';
				StrIndex = 0;
				if(State == ReadingZoneName)  {
					if(strcmp(ZoneFileName, IniBuffer)) {
						StrIndex = 0;
						// Not our zone, skip to next line
						while((ch!=EOF)&&(ch!='\n'))
		       	        			ch = fgetc(IniFile);
						continue;
					}
					else {
						State = ReadingModelNumbers;
						INIEntryFound = true;
					}
				}
				else  {
					bool Exclude = false;
					bool Group = false;
					if(IniBuffer[0]=='-')
					{
						Exclude = true;
						strcpy(IniBuffer, IniBuffer+1);
					}
					if(IniBuffer[0]=='g')
					{
						Group = true;
						strcpy(IniBuffer, IniBuffer+1);
					}
					ModelNumber = atoi(IniBuffer);
					if(!Group)
					{
						if((ModelNumber >= 0) && (ModelNumber < fileloader->model_data.model_count))
							fileloader->model_data.models[ModelNumber]->IncludeInMap = Exclude ? false : true;
						else
							printf("ERROR: Specified model %s invalid, must be in range 0 to %i\n", IniBuffer, fileloader->model_data.model_count - 1);
					}
					else
					{
						if((ModelNumber >= 0) && ((unsigned int)ModelNumber < fileloader->model_data.ObjectGroups.size()))
							fileloader->model_data.ObjectGroups[ModelNumber].IncludeInMap = Exclude ? false : true;

					}
				}
				continue;
			}
			IniBuffer[StrIndex++] = tolower(ch);
		}
		fclose(IniFile);

		if(INIEntryFound)
			printf("azone.ini entry found for this zone. ");
		else
			printf("No azone.ini entry found for zone %s\n", ZoneFileName);
	}

	Iterator = fileloader->model_data.ObjectGroups.begin();

	while(Iterator != fileloader->model_data.ObjectGroups.end())
	{
		if(!(*Iterator).IncludeInMap)
		{
			++OGNum;
			++Iterator;
			continue;
		}


#ifdef DEBUG
		printf("ObjectGroup Number: %i\n", OGNum++);

		printf("ObjectGroup Coordinates: %8.3f, %8.3f, %8.3f\n",
			(*Iterator).x, (*Iterator).y, (*Iterator).z);

		printf("Tile: %8.3f, %8.3f, %8.3f\n",
			(*Iterator).TileX, (*Iterator).TileY, (*Iterator).TileZ);

		printf("ObjectGroup Rotations  : %8.3f, %8.3f, %8.3f\n",
			(*Iterator).RotX, (*Iterator).RotY, (*Iterator).RotZ);

		printf("ObjectGroup Scale      : %8.3f, %8.3f, %8.3f\n",
			(*Iterator).ScaleX, (*Iterator).ScaleY, (*Iterator).ScaleZ);
#endif

		list<int>::iterator ModelIterator;

		ModelIterator = (*Iterator).SubObjects.begin();

		while(ModelIterator != (*Iterator).SubObjects.end())
		{
			int SubModel = (*ModelIterator);

#ifdef DEBUG
			printf("  SubModel: %i\n", (*ModelIterator));
#endif

			XOffset = fileloader->model_data.placeable[SubModel]->x;
			YOffset = fileloader->model_data.placeable[SubModel]->y;
			ZOffset = fileloader->model_data.placeable[SubModel]->z;
#ifdef DEBUG
			printf("   Translations: %8.3f, %8.3f, %8.3f\n", XOffset, YOffset, ZOffset);
			printf("   Rotations   : %8.3f, %8.3f, %8.3f\n", fileloader->model_data.placeable[SubModel]->rx,
				fileloader->model_data.placeable[SubModel]->ry, fileloader->model_data.placeable[SubModel]->rz);
			printf("   Scale       : %8.3f, %8.3f, %8.3f\n", fileloader->model_data.placeable[SubModel]->scale[0],
				fileloader->model_data.placeable[SubModel]->scale[1], fileloader->model_data.placeable[SubModel]->scale[2]);
#endif

			RotX = fileloader->model_data.placeable[SubModel]->rx * 3.14159 / 180;  // Convert from degrees to radians
			RotY = fileloader->model_data.placeable[SubModel]->ry * 3.14159 / 180;
			RotZ = fileloader->model_data.placeable[SubModel]->rz * 3.14159 / 180;

			XScale = fileloader->model_data.placeable[SubModel]->scale[0];
			YScale = fileloader->model_data.placeable[SubModel]->scale[1];
			ZScale = fileloader->model_data.placeable[SubModel]->scale[2];

			Model *model = fileloader->model_data.models[fileloader->model_data.placeable[SubModel]->model];

#ifdef DEBUG
			printf("Model %i, name is %s\n", fileloader->model_data.placeable[SubModel]->model, model->name);
#endif
			if(!model->IncludeInMap)
			{
				++ModelIterator;
#ifdef DEBUG
				printf("Not including in .map\n");
#endif
				continue;
			}
			for(int j = 0; j < model->poly_count; ++j) {

				poly = model->polys[j];

				verts[0] = model->verts[poly->v1];
				verts[1] = model->verts[poly->v2];
				verts[2] = model->verts[poly->v3];

				v1.x = verts[0]->x; v1.y = verts[0]->y; v1.z = verts[0]->z;
				v2.x = verts[1]->x; v2.y = verts[1]->y; v2.z = verts[1]->z;
				v3.x = verts[2]->x; v3.y = verts[2]->y; v3.z = verts[2]->z;

				// Scale by the values specified for the individual object.
				//
				ScaleVertex(v1, XScale, YScale, ZScale);
				ScaleVertex(v2, XScale, YScale, ZScale);
				ScaleVertex(v3, XScale, YScale, ZScale);
				// Translate by the values specified for the individual object
				//
				TranslateVertex(v1, XOffset, YOffset, ZOffset);
				TranslateVertex(v2, XOffset, YOffset, ZOffset);
				TranslateVertex(v3, XOffset, YOffset, ZOffset);
				// Rotate by the values specified for the group
				// TODO: The X/Y rotations can be combined
				//
				RotateVertex(v1, (*Iterator).RotX * 3.14159 / 180.0f, 0, 0);
				RotateVertex(v2, (*Iterator).RotX * 3.14159 / 180.0f, 0, 0);
				RotateVertex(v3, (*Iterator).RotX * 3.14159 / 180.0f, 0, 0);

				RotateVertex(v1, 0, (*Iterator).RotY * 3.14159 / 180.0f, 0);
				RotateVertex(v2, 0, (*Iterator).RotY * 3.14159 / 180.0f, 0);
				RotateVertex(v3, 0, (*Iterator).RotY * 3.14159 / 180.0f, 0);

				// To make things align properly, we need to translate the object back to the origin
				// before applying the model Z rotation. This is what the Correction VERTEX is for.
				//
				VERTEX Correction(XOffset, YOffset, ZOffset);

				RotateVertex(Correction, (*Iterator).RotX * 3.14159 / 180.0f, 0, 0);

				TranslateVertex(v1, -Correction.x, -Correction.y, -Correction.z);
				TranslateVertex(v2, -Correction.x, -Correction.y, -Correction.z);
				TranslateVertex(v3, -Correction.x, -Correction.y, -Correction.z);
				// Rotate by model Z rotation
				//
				//
				RotateVertex(v1, RotX, 0, 0);
				RotateVertex(v2, RotX, 0, 0);
				RotateVertex(v3, RotX, 0, 0);

				// Don't know why the Y rotation needs to be negative
				//
				RotateVertex(v1, 0, -RotY, 0);
				RotateVertex(v2, 0, -RotY, 0);
				RotateVertex(v3, 0, -RotY, 0);

				RotateVertex(v1, 0, 0, RotZ);
				RotateVertex(v2, 0, 0, RotZ);
				RotateVertex(v3, 0, 0, RotZ);
				// Now we have applied the individual model rotations, translate back to where we were.
				//
				TranslateVertex(v1, Correction.x, Correction.y, Correction.z);
				TranslateVertex(v2, Correction.x, Correction.y, Correction.z);
				TranslateVertex(v3, Correction.x, Correction.y, Correction.z);
				// Rotate by the Z rotation value specified for the object group
				//
				RotateVertex(v1, 0, 0, (*Iterator).RotZ * 3.14159 / 180.0f);
				RotateVertex(v2, 0, 0, (*Iterator).RotZ * 3.14159 / 180.0f);
				RotateVertex(v3, 0, 0, (*Iterator).RotZ * 3.14159 / 180.0f);
				// Scale by the object group values
				//
				ScaleVertex(v1, (*Iterator).ScaleX, (*Iterator).ScaleY, (*Iterator).ScaleZ);
				ScaleVertex(v2, (*Iterator).ScaleX, (*Iterator).ScaleY, (*Iterator).ScaleZ);
				ScaleVertex(v3, (*Iterator).ScaleX, (*Iterator).ScaleY, (*Iterator).ScaleZ);
				// Translate to the relevant tile starting co-ordinates
				//
				TranslateVertex(v1, (*Iterator).TileX, (*Iterator).TileY, (*Iterator).TileZ); // Y+14, Z + 68
				TranslateVertex(v2, (*Iterator).TileX, (*Iterator).TileY, (*Iterator).TileZ);
				TranslateVertex(v3, (*Iterator).TileX, (*Iterator).TileY, (*Iterator).TileZ);
				// Translate to the position within the tile for this object group
				//
				TranslateVertex(v1, (*Iterator).x, (*Iterator).y, (*Iterator).z);
				TranslateVertex(v2, (*Iterator).x, (*Iterator).y, (*Iterator).z);
				TranslateVertex(v3, (*Iterator).x, (*Iterator).y, (*Iterator).z);
				// Swap X & Y
				//
				tmpv = v1; v1.x = tmpv.y; v1.y = tmpv.x;
				tmpv = v2; v2.x = tmpv.y; v2.y = tmpv.x;
				tmpv = v3; v3.x = tmpv.y; v3.y = tmpv.x;

				AddFace(v1, v2, v3);

			}
			++ModelIterator;
		}

		++Iterator;
	}


}

void QTBuilder::RotateVertex(VERTEX &v, float XRotation, float YRotation, float ZRotation) {

	VERTEX nv;

	nv = v;

	// Rotate about the X axis
	//
	nv.y = (cos(XRotation) * v.y) - (sin(XRotation) * v.z);
	nv.z = (sin(XRotation) * v.y) + (cos(XRotation) * v.z);

	v = nv;


	// Rotate about the Y axis
	//
	nv.x = (cos(YRotation) * v.x) + (sin(YRotation) * v.z) ;
	nv.z = -(sin(YRotation) * v.x) + (cos(YRotation) * v.z);

	v = nv;

	// Rotate about the Z axis
	//
	nv.x = (cos(ZRotation) * v.x) - (sin(ZRotation) * v.y) ;
	nv.y = (sin(ZRotation) * v.x) + (cos(ZRotation) * v.y) ;

	v = nv;
}

void QTBuilder::ScaleVertex(VERTEX &v, float XScale, float YScale, float ZScale) {

	v.x = v.x * XScale;
	v.y = v.y * YScale;
	v.z = v.z * ZScale;
}


void QTBuilder::TranslateVertex(VERTEX &v, float XOffset, float YOffset, float ZOffset) {

	v.x = v.x + XOffset;
	v.y = v.y + YOffset;
	v.z = v.z + ZOffset;
}






