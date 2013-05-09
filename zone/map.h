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
#ifndef MAP_H
#define MAP_H

#include <stdio.h>

//this is the current version number to expect from the map header
#define MAP_VERSION 0x01000000

#define BEST_Z_INVALID -999999

#pragma pack(1)

typedef struct _vertex{
//	unsigned long order;
	float x;
	float y;
	float z;

	_vertex()
	{
		x = y = z = 0.0f;
	};

	_vertex(float ix, float iy, float iz)
	{
		x = ix;
		y = iy;
		z = iz;
	}
	bool operator==(const _vertex &v1) const
	{
		return((v1.x == x) && (v1.y == y) && (v1.z ==z));
	}

}VERTEX, *PVERTEX;

typedef struct _face{
//	unsigned long a, b, c;	//vertexs
	VERTEX a;
	VERTEX b;
	VERTEX c;
	float nx, ny, nz, nd;
}FACE, *PFACE;

typedef struct _mapHeader {
	uint32 version;
	uint32 face_count;
	uint16 node_count;
	uint32 facelist_count;
} mapHeader;

/*
 This is used for the recursive node structure
 unsigned shorts are adequate because, worst case
 even in a zone that is 6000x6000 with a small node
 size of 30x30, there are only 40000 nodes.

 quadrent definitions:
 quad 1 (nodes[0]):
 x>=0, y>=0
 quad 2 (nodes[1]):
 x<0, y>=0
 quad 3 (nodes[2]):
 x<0, y<0
 quad 4 (nodes[3]):
 x>=0, y<0

 */
enum { //node flags
	nodeFinal = 0x01
	//7 more bits if theres something to use them for...
};
typedef struct _nodeHeader {
	//bounding box of this node
	//there is no reason that these could not be unsigned
	//shorts other than we have to compare them to floats
	//all over the place, so they stay floats for now.
	//changing it could save 8 bytes per node record (~320k for huge maps)
	float minx;
	float miny;
	float maxx;
	float maxy;

	uint8 flags;
	union {
		uint16 nodes[4];	//index 0 means nullptr, not root
		struct {
			uint32 count;
			uint32 offset;
		} faces;
	};
} nodeHeader, NODE, *PNODE;

#pragma pack()

//special value returned as 'not found'
#define NODE_NONE 65534
#define MAP_ROOT_NODE 0

typedef uint16 NodeRef;

/*typedef struct _node {
	nodeHeader head;
	unsigned int *	pfaces;
	char						mask;
	struct _node	*	node1, *node2, *node3, *node4;
}NODE, *PNODE;*/

class Map {
public:
	static Map* LoadMapfile(const char* in_zonename, const char *directory = nullptr);

	Map();
	~Map();

	bool loadMap(FILE *fp);

	//the result is always final, except special NODE_NONE
	NodeRef SeekNode( NodeRef _node, float x, float y ) const;

	//these are untested since rewrite:
	int *SeekFace( NodeRef _node, float x, float y );
	float GetFaceHeight( int _idx, float x, float y ) const;

	bool LocWithinNode( NodeRef _node, float x, float y ) const;

	//nodes to these functions must be final
	bool LineIntersectsNode( NodeRef _node, VERTEX start, VERTEX end, VERTEX *result, FACE **on = nullptr) const;
	bool LineIntersectsFace( PFACE cface, VERTEX start, VERTEX end, VERTEX *result) const;
	float FindBestZ( NodeRef _node, VERTEX start, VERTEX *result, FACE **on = nullptr) const;
	bool LineIntersectsZone(VERTEX start, VERTEX end, float step, VERTEX *result, FACE **on = nullptr) const;

//	inline unsigned int		GetVertexNumber( ) {return m_Vertex; }
	inline uint32		GetFacesNumber( ) const { return m_Faces; }
//	inline PVERTEX	GetVertex( int _idx ) {return mFinalVertex + _idx;	}
	inline PFACE		GetFace( int _idx) {return mFinalFaces + _idx;		}
	inline PFACE		GetFaceFromlist( int _idx) {return &mFinalFaces[ mFaceLists[_idx] ];	}
	inline NodeRef		GetRoot( ) const { return MAP_ROOT_NODE; }
	inline PNODE		GetNode( NodeRef r ) { return( mNodes + r ); }

	inline float GetMinX() const { return(_minx); }
	inline float GetMaxX() const { return(_maxx); }
	inline float GetMinY() const { return(_miny); }
	inline float GetMaxY() const { return(_maxy); }
	inline float GetMinZ() const { return(_minz); }
	inline float GetMaxZ() const { return(_maxz); }
	bool LineIntersectsZoneNoZLeaps(VERTEX start, VERTEX end, float step_mag, VERTEX *result, FACE **on);
	float FindClosestZ(VERTEX p ) const;

private:
//	unsigned long m_Vertex;
	uint32 m_Faces;
	uint32 m_Nodes;
	uint32 m_FaceLists;
//	PVERTEX mFinalVertex;
	PFACE mFinalFaces;
	PNODE mNodes;
	uint32 *mFaceLists;


	int mCandFaces[100];

	float _minz, _maxz;
	float _minx, _miny, _maxx, _maxy;

	static void Normalize(VERTEX *p);

//	void	RecLoadNode( PNODE	_node, FILE *l_f );
//	void	RecFreeNode( PNODE	_node );
};

#endif

