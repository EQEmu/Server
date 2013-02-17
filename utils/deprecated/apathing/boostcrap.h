#ifndef BOOSTCRAP_H
#define BOOSTCRAP_H

#include "apathing.h"

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
using namespace boost;


//man I hate boost
typedef adjacency_list < vecS, vecS, undirectedS,
	property<vertex_distance_t, int>, property < edge_weight_t, int > >
	MyGraph;

typedef graph_traits < MyGraph >::vertex_descriptor VertDesc;
typedef graph_traits < MyGraph >::edge_descriptor EdgeDesc;

void build_boost_graph(MyGraph &vg, property_map<MyGraph, edge_weight_t>::type &weightmap, map<PathEdge *, EdgeDesc> &em, PathGraph *big, bool set_weights = true);
void run_min_spanning_tree(MyGraph &vg, property_map<MyGraph, edge_weight_t>::type &weightmap, map<PathEdge *, EdgeDesc> &em, PathGraph *big, int start_node);
void find_disjoint_grids(Map *map, MyGraph &vg, PathGraph *big, const char *file, vector<int> &start_nodes, vector<PathGraph *> &disjoints);
void calc_path_lengths(Map *map, MyGraph &vg, PathGraph *big, map<PathEdge *, EdgeDesc> &em, const char *fname);
void color_disjoint_graphs(PathGraph *big, MyGraph &vg, Map *map, const char *fname, vector< vector<int> > &D, vector<int> &counts, vector<int> &disjoint_counts, vector<int> &first_node );
void count_crossing_lines(list<PathEdge *> &edges, PathGraph *out, PathGraph *excess, map<PathEdge*, vector<GPoint> > &cross_list);
void consolidate_cross_graphs(Map *map, PathGraph *cross_big, PathGraph *cross_excess, MyGraph &cross_graph, const char *fname);
void just_color_the_damned_thing(Map *map, PathGraph *big, const char *fname);

#endif

