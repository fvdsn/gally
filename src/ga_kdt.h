#ifndef __GA_KD_TREE_H__
#define __GA_KD_TREE_H__
#include "ga_geom.h"
#include "ga_list.h"
#define GA_MAX_REC_DEPTH 50

enum{
	GA_AXIS_X ,
	GA_AXIS_Y ,
	GA_AXIS_Z ,
	GA_AXIS_COUNT 
};
typedef struct ga_kdn_s{
	float split;		/* separating distance */
	int axis;		/* separating axis */
	struct ga_kdn_s *left;	/* left subtree */
	struct ga_kdn_s *right;	/* right subtree */
	ga_list_t *data;	/* triangles */
}ga_kdn_t;
/* Creates an octree from a triangle list.
 * max_depth : maximum depth of a leaf.
 * min_tri. if there is less than min_tri in a node, it will stop recursion.
 * axis : the first division axis.
 * min,max : bounding box vector
 */
ga_kdn_t *ga_kdn_octree(	ga_list_t *tri_list, 
				int max_depth, 
				int min_tri, 
				int axis, 
				vec_t min, 
				vec_t max	);
/* traces a ray in the kd tree, and returns 1 if hit. 0 else.
 * root : the kdtree or octree.
 * min,max : bounding box of the scene
 * origin : starting position
 * dir : direction of moving ray.
 * tri : the triangle that has been hit if any
 * u,v : uv coordinates of the hit point
 * dist : distance of the hitpoint from origin. (in multiples of dir )
 */
int ga_kdn_trace(	const ga_kdn_t *root, 
			vec_t min, 
			vec_t max, 
			vec_t origin, 
			vec_t dir, 
			tri_t **tri, 
			float *u, 
			float *v, 
			float *dist		);
/*
int ga_kdn_trace_fast(	ga_kdn_t *root,
			const vec_t *box_min,
			const vec_t *box_max,
			const vec_t *origin,
			const vec_t *dir,
			tri_t **tri, float *u, float *v, float *dist);
*/
#endif
