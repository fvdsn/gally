#ifndef __GA_PHOTONMAP_H__
#define __GA_PHOTONMAP_H__
#include "ga_math.h"
#include "ga_list_t.h"
typedef struct ga_photon_s{
	vec_t pos;
	vec_t color;
	vec_t normal;
}ga_photon_t;
typedef struct ga_photonmap_s{
	int resolution;
	vec_t bound_min;
	vec_t bound_max;
	vec_t size;
	vec_t inv_size;
	vec_t interval;
	vec_t inv_interval;
	vec_t outside;
	vec_t * map;
	ga_list_t *pmap;
}ga_photonmap_t;
ga_photon_t *ga_photon_new(vec_t pos, vec_t color, vec_t normal);
ga_photonmap_t *ga_photonmap_new(vec_t bound_min,vec_t bound_max, vec_t outside,int resolution);
void ga_photonmap_add(ga_photonmap_t *pm,vec_t pos, vec_t color);
vec_t ga_photonmap_get(const ga_photonmap_t *pm, vec_t pos);
#endif
