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
	vec_t front;	/*normalized vector in the viewing direction*/ 
	vec_t origin;	/*camera position*/
	vec_t cu;	/*vector to the top of the complete render zone*/
	vec_t cr; 	/*vector to the right side of the complete render zone*/
	ga_scene_t *scene;
}ga_ray_thread_data_t;

/* checks for intersection and barycentric coordinates */
int   ga_ray_intersect(const tri_t *tr, const vec_t *start, const vec_t* dir,float *t, float*u,float*v);
/* returns the color of a raytrace */
vec_t ga_ray_trace(ga_scene_t *s, vec_t start, vec_t dir);
/* renders a scene to s->img by raytracing it */
void  ga_ray_render(ga_scene_t *s);

#endif
