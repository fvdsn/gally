#ifndef __GALLY_RAYTRACE_H__
#define __GALLY_RAYTRACE_H__
#include "ga_math.h"
#include "ga_geom.h"
#include "ga_scene.h"

#define GA_THREAD_COUNT 8

/* this is the data given to each rendering thread */
typedef struct ga_ray_thread_data_s{
	int id;	/* thread number */
	int px;	/*bottom left corner of thread render zone (included) */
	int py;
	int sx;	/*top right corner of thread render zone (excluded) */
	int sy;
	int samples;	/* for antialiasing */
	float dither;
	vec_t front;	/* normalized vector in the viewing direction */ 
	vec_t origin;	/*camera position*/
	vec_t cu;	/*vector to the top of the complete render zone*/
	vec_t cr; 	/*vector to the right side of the complete render zone*/
	ga_scene_t *scene;
}ga_ray_thread_data_t;

/* returns the color of a raytrace */
vec_t ga_ray_trace(	const ga_scene_t *s, 
			vec_t start, 
			vec_t dir,
			float importance,
			int max_rec,
			vec_t *pos		);
/* renders a scene to s->img by raytracing it */
void  ga_ray_render(ga_scene_t *s);
/**
 * returns the length of the ray before it hits something. It will return
 * FLT_MAX if there is no hit
 */
float ga_ray_length(const ga_scene_t *s,vec_t origin,vec_t dir);
/**
 * Compute the photon map
 */
void ga_ray_gi_compute(ga_scene_t *s);
/**
 * Computes the shading of a point in the scene
 */
vec_t ga_ray_shade(	const vec_t *pos, 
			const vec_t *dir, 
			const vec_t *norm,
			const ga_material_t *mat,
			const ga_scene_t *s,
			float importance, 
			int max_rec);

#endif
