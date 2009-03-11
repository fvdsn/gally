#ifndef __GALLY_RAYTRACE_H__
#define __GALLY_RAYTRACE_H__
#include "ga_math.h"
#include "ga_geom.h"
#include "ga_scene.h"

/* checks for intersection and barycentric coordinates */
int   ga_ray_intersect(const tri_t *tr, const vec_t *start, const vec_t* dir,float *t, float*u,float*v);
/* returns the color of a raytrace */
vec_t ga_ray_trace(ga_scene_t *s, vec_t start, vec_t dir);
/* renders a scene to s->img by raytracing it */
void  ga_ray_render(ga_scene_t *s);

#endif
