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
#define GA_KDN_LEAF 1
#define GA_KDN_NODE 0
#define GA_MAX_REC_DEPTH 50

typedef struct ga_kdn_s{
	float split;
	char  axis;
	char  p0_flags;
	char  p1_flags;
	char  flags;
	void  *p0;
	void  *p1;
}ga_kdn_t;


ga_kdn_t *ga_kdtree_build_mean(ga_list_t *tri_list, 
				int max_tri, 
				int max_depth, 
				int axis);
void ga_kdtree_print(ga_kdn_t* kdn, int level);
int ga_kdtree_ray_trace(const ga_kdn_t *root, 
			const vec_t *box_min, 
			const vec_t *box_max, 
			const vec_t *origin, 
			const vec_t *dir, 
			tri_t **tri, float *u, float *v, float *dist);

#endif

