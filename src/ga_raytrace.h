#ifndef __GALLY_RAYTRACE_H__
#define __GALLY_RAYTRACE_H__
#include "ga_math.h"
#include "ga_geom.h"
#include "ga_scene.h"

#define GA_RAY_MAX_DEPTH 64

int   ga_ray_intersect(tri_t *tr, vec_t start, vec_t dir,float *t, float*u,float*v);
vec_t ga_ray_trace(ga_scene_t *s, vec_t start, vec_t dir);
void  ga_ray_render(ga_scene_t *s);

#endif
