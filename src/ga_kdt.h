#ifndef __GA_KD_TREE_H__
#define __GA_KD_TREE_H__
#include "ga_geom.h"
#include "ga_list.h"

enum{
	GA_AXIS_X ,
	GA_AXIS_Y ,
	GA_AXIS_Z ,
	GA_AXIS_COUNT 
};
typedef struct ga_kdn_s{
	float split;
	int axis;
	struct ga_kdn_s *left;
	struct ga_kdn_s *right;
	ga_list_t *data;
}ga_kdn_t;

ga_kdn_t *ga_kdn_octree(ga_list_t *tri_list, int max_depth, int min_tri, int axis, vec_t min, vec_t max);
int ga_kdn_trace(ga_kdn_t *root, vec_t min, vec_t max, vec_t origin, vec_t dir, tri_t **tri, float *u, float *v, float *dist){

#endif
