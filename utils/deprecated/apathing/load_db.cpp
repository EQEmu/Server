/*

Fear Pathing generation utility.
(c) 2005 Father Nitwit

*/
#include "../common/types.h"
#include "../zone/map.h"
#include "../common/rdtsc.h"
#include "quadtree.h"
#include "apathing.h"
#include <stdio.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>



bool load_paths_from_db(MYSQL *m, Map *map, const char *zone, list<PathGraph*> &db_paths, list<PathNode*> &end_points) {
	char query[512];
	
	sprintf(query, 
		"SELECT x,y,z,gridid FROM grid_entries,zone "
		"WHERE zone.zoneidnumber=zoneid AND short_name='%s' "
		"ORDER BY gridid,number", zone);
	if(mysql_query(m, query) != 0) {
		printf("Unable to query: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_RES *res = mysql_store_result(m);
	if(res == NULL) {
		printf("Unable to store res: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_ROW row;
	
	PathNode *cur = NULL, *last = NULL, *first = NULL;
	PathGraph *g = NULL;
	int cur_g = -1,last_g = -1;
	
//	int lid = 0;
	
	while((row = mysql_fetch_row(res))) {
		last = cur;
		cur = new PathNode;
//		cur->load_id = lid++;
		cur->x = atof(row[0]);
		cur->y = atof(row[1]);
		cur->z = atof(row[2]);
		cur_g = atoi(row[3]);
		if(cur_g != last_g) {
			if(g != NULL) {
				//if we have a first and last node for this path
				//and they are not the same node, try to connect them.
				if(first != NULL && last != NULL && first != last && last->Dist2(first) < ENDPOINT_CONNECT_MAX_DISTANCE*ENDPOINT_CONNECT_MAX_DISTANCE) {
					if(CheckLOS(map, last, first))
						g->add_edge(last, first);
				}
#ifdef LINK_PATH_ENDPOINTS
				if(first != last && last != NULL)
					end_points.push_back(last);
#endif
				db_paths.push_back(g);
			}
			g = new PathGraph();
			first = cur;
			last_g = cur_g;
			last = NULL;
		}
		
		g->nodes.push_back(cur);
		
#ifdef LINK_PATH_ENDPOINTS
		//this is a begining point
		if(last == NULL)
			end_points.push_back(cur);
#endif
		
		if(last != NULL) {
#ifdef SPLIT_INVALID_PATHS
			if(CheckLOS(map, last, cur)) {
				g->edges.push_back(new PathEdge(last, cur));
			} else {
				//no LOS, split the path into two
				load_split_paths++;
				last_g = -1;	//tell this thing to start over
			}
#else
			g->edges.push_back(new PathEdge(last, cur));
#endif
		}
	}
	
	//handle the last active path
	if(g != NULL) {
		if(first != NULL && cur->Dist2(first) < ENDPOINT_CONNECT_MAX_DISTANCE*ENDPOINT_CONNECT_MAX_DISTANCE) {
			if(CheckLOS(map, cur, first))
				g->add_edge(cur, first);
		}
		db_paths.push_back(g);
	}
	
	mysql_free_result(res);
	return(true);
}


bool load_spawns_from_db(MYSQL *m, const char *zone, list<PathNode*> &db_spawns) {
	char query[512];
	
	sprintf(query, 
		"SELECT x,y,z FROM spawn2 "
		"WHERE  zone='%s'", zone);
	if(mysql_query(m, query) != 0) {
		printf("Unable to query: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_RES *res = mysql_store_result(m);
	if(res == NULL) {
		printf("Unable to store res: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_ROW row;
	
	PathNode *cur = NULL;
	
	while((row = mysql_fetch_row(res))) {
		cur = new PathNode;
		cur->x = atof(row[0]);
		cur->y = atof(row[1]);
		cur->z = atof(row[2]);
		db_spawns.push_back(cur);
	}
	
	mysql_free_result(res);
	
	return(true);
}


bool load_doors_from_db(MYSQL *m, const char *zone, list<PathNode*> &db_spawns) {
	char query[512];
	
	sprintf(query, 
		"SELECT pos_x,pos_y,pos_z FROM doors "
		"WHERE  zone='%s'", zone);
	if(mysql_query(m, query) != 0) {
		printf("Unable to query: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_RES *res = mysql_store_result(m);
	if(res == NULL) {
		printf("Unable to store res: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_ROW row;
	
	PathNode *cur = NULL;
	
	while((row = mysql_fetch_row(res))) {
		cur = new PathNode;
		cur->x = atof(row[0]);
		cur->y = atof(row[1]);
		cur->z = atof(row[2]);
		//TODO: it would be nice if we could get to the middle of these
		//doors, not the edge of them which I assume these points are
		db_spawns.push_back(cur);
	}
	
	mysql_free_result(res);
	
	return(true);
}


bool load_hints_from_db(MYSQL *m, const char *zone, list<PathNode*> &db_spawns) {
	char query[512];
	
	sprintf(query, 
		"SELECT x,y,z,forced,disjoint FROM fear_hints "
		"WHERE  zone='%s'", zone);
	if(mysql_query(m, query) != 0) {
		printf("Unable to query: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_RES *res = mysql_store_result(m);
	if(res == NULL) {
		printf("Unable to store res: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_ROW row;
	
	PathNode *cur = NULL;
	
	while((row = mysql_fetch_row(res))) {
		cur = new PathNode;
		cur->x = atof(row[0]);
		cur->y = atof(row[1]);
		cur->z = atof(row[2]);
		cur->forced = atoi(row[3])?true:false;
		cur->disjoint = atoi(row[4])?true:false;
		db_spawns.push_back(cur);
	}
	
	mysql_free_result(res);
	
	return(true);
}


bool load_settings_from_db(MYSQL *m, const char *zone) {
	char query[512];
	
	sprintf(query, 
		"SELECT use_doors, min_fix_z, max_fear_distance, image_scale, split_invalid_paths,"
		" link_path_endpoints, end_distance, split_long_min, split_long_step, same_dist, node_combine_dist,"
		" grid_combine_dist, close_all_los, cross_count, cross_min_length, cross_max_z_diff, cross_combine_dist,"
		" second_link_dist, link_max_dist, link_count"
		" FROM fear_settings"
		" WHERE zone='%s'", zone);
	if(mysql_query(m, query) != 0) {
//		printf("Unable to query: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_RES *res = mysql_store_result(m);
	if(res == NULL) {
//		printf("Unable to store res: %s\n", mysql_error(m));
		return(false);
	}
	
	MYSQL_ROW row;
		
	int r = 0;
	if((row = mysql_fetch_row(res))) {
		INCLUDE_DOORS = atoi(row[r++])?true:false;
		MIN_FIX_Z = atof(row[r++]);
		FEAR_MAXIMUM_DISTANCE = atof(row[r++]);
		IMAGE_SCALE = atoi(row[r++]);
		SPLIT_INVALID_PATHS = atoi(row[r++])?true:false;
		LINK_PATH_ENDPOINTS = atoi(row[r++])?true:false;
		ENDPOINT_CONNECT_MAX_DISTANCE = atof(row[r++]);
		SPLIT_LINE_LENGTH = atof(row[r++]);
		SPLIT_LINE_INTERVAL = atof(row[r++]);
		CLOSE_ENOUGH = atof(row[r++]);
		CLOSE_ENOUGH_COMBINE = atof(row[r++]);
		MERGE_MIN_SECOND_DIST = atof(row[r++]);
		COMBINE_CHECK_ALL_LOS = atoi(row[r++])?true:false;
		CROSS_REDUCE_COUNT = atoi(row[r++]);
		CROSS_MIN_LENGTH = atof(row[r++]);
		CROSS_MAX_Z_DIFF = atof(row[r++]);
		CLOSE_ENOUGH_CROSS = atof(row[r++]);
		
		SPAWN_MIN_SECOND_DIST = atof(row[r++]);
		MAX_LINK_SPAWN_DIST = atof(row[r++]);
		int sc = atoi(row[r++]);
		SPAWN_LINK_TWICE = sc >= 2?true:false;
		SPAWN_LINK_THRICE = sc >= 3?true:false;
		mysql_free_result(res);
		return(true);
	}
	
	mysql_free_result(res);
	
	return(false);
}

