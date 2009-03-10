#ifndef __GALLY_IMAGE_H__
#define __GALLY_IMAGE_H__
#include "ga_math.h"

typedef struct ga_image_s{
	int sizex;	/* x size in pixel */
	int sizey;	/* y size in pixel */
	vec_t *pixel;	/* pixel buffer */
	float *zbuffer; /* z buffer */
}ga_image_t;
/**
 * Creates a new image of size (sizex*sizey)
 */
ga_image_t *ga_image_new(int sizex, int sizey);
/**
 * Save image to path as png file 
 */
void	ga_image_save(ga_image_t *i, char *path);
/**
 * Set a pixel at coordinate x,y  (0,0)@bottom left
 */
void	ga_image_set_pixel(ga_image_t *i, int x, int y, vec_t color);
/**
 * Set a pixel at coordinate x,y if the z value is higher than the z value
 * in the buffer. if the pixel is set, the z buffer is updated to the new
 * value.
 */
void	ga_image_set_zpixel(ga_image_t *i, int x, int y, float z, vec_t color);
/**
 * Fills the image with color
 */
void	ga_image_fill(ga_image_t *i,vec_t color);

#endif

