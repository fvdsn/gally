#ifndef __GA_RASTER_H__
#define	__GA_RASTER_H__
#include "ga_img.h"
#include "ga_scene.h"
#include "ga_geom.h"

/* renders a triangle on an image */
void ga_raster_triangle(ga_image_t *img, tri_t *tri);
/* renders the scene to s->img using raterisation */
void ga_raster_render(ga_scene_t *s);

#endif


