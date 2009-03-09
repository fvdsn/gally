#ifndef __GALLY_IMAGE_H__
#define __GALLY_IMAGE_H__
#include "ga_math.h"

typedef struct ga_image_s{
	int sizex;
	int sizey;
	vec_t *pixel;
	float *zbuffer;
}ga_image_t;
ga_image_t *ga_image_new(int sizex, int sizey);
void	ga_image_save(ga_image_t *i, char *path);
void	ga_image_set_pixel(ga_image_t *i, int x, int y, vec_t color);
void	ga_image_set_zpixel(ga_image_t *i, int x, int y, float z, vec_t color);
void	ga_image_fill(ga_image_t *i,vec_t color);
void	ga_image_display(ga_image_t *i);

#endif

