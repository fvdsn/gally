#ifndef __GALLY_GEOMETRY_H__
#define __GALLY_GEOMETRY_H__
#include "ga_math.h"

/*max number of faces and vertices that can be loaded from an obj file */
#define GA_MAX_VERT 5000000

typedef struct tri_s{
	vec_t norm;	/*triangle normal*/
	vec_t vert[3];
	vec_t vnorm[3];	/*vertex normals*/
	vec_t vtex[3];	/*vertex texture coordinates*/
	vec_t vcolor[3];/*vertex colors*/
	vec_t edge[2];	/*v0->v1 , v0->v2*/
	void * material;
}tri_t;
/**
 * Prints the triangle to the console
 */
tri_t *tri_print(tri_t *t);
/**
 * transform the triangle with the matrix m
 */
tri_t *tri_transform(tri_t *t, const mat_t *m, const mat_t*n);
/**
 * copy the triangle t2 into triangle t
 */
void   tri_cpy(tri_t *t, const tri_t *t2);
void ga_tri_bound(const tri_t *tri, int axis, float *min, float *max);

typedef struct model_s{
	int tri_count;
	tri_t *tri;
}model_t;
/**
 * Loads obj file at path.
 */
model_t *model_load(char *path);
/**
 * Create a duplicate model of m
 */
model_t *model_dup(const model_t *m);
/**
 * Free the memory allocated by m
 */
void 	model_free(model_t *m);
/**
 * Prints the model to the console
 */
void model_print(const model_t*m);

int ga_ray_box_intersect(	const vec_t *origin, 
				const vec_t *dir, 
				const vec_t *min, 
				const vec_t *max, float *ta, float *tb);
int ga_ray_tri_intersect(	const tri_t *tr, 
				const vec_t *origin, 
				const vec_t * dir, float *t, float*u,float*v);

#endif



