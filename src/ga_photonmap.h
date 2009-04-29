#ifndef __GA_PHOTONMAP_H__
#define __GA_PHOTONMAP_H__
#include "ga_math.h"
#include "ga_list.h"

typedef struct ga_photon_s{
	vec_t pos;
	vec_t color;
	vec_t normal;
	vec_t ldir;	/* points towards the light */
}ga_photon_t;

typedef struct ga_photonmap_s{
	int resx;	/* cell counts */
	int resy;
	int resz;
	vec_t bound_min;	/*bounding box*/
	vec_t bound_max;
	vec_t size;
	vec_t inv_size;
	ga_list_t *map;		/*photon map*/
}ga_photonmap_t;
/**
 * Creates a new photonmap
 * bound_min,max : the bounding box of the scene
 * resolution : the size of a grid element. Must be equal or larger
 * than the maximum photon radius. Equal is optimal.
 */
ga_photonmap_t *ga_photonmap_new(	vec_t bound_min,
					vec_t bound_max, 
					float resolution	);
/**
 * Adds a photon to the photon map
 */
void ga_photonmap_add( 	ga_photonmap_t *pm,
			const vec_t * pos,
			const vec_t *color, 
			const vec_t * normal,
			const vec_t * ldir	);
/**
 * Returns the list of photons that could be closer than the 
 * photonmap resolution to the point pos.
 */
ga_list_t * ga_photonmap_get(const ga_photonmap_t *pm, const vec_t * pos);
#endif
