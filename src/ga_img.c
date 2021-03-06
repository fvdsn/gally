#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <png.h>
#include "ga_img.h"

ga_image_t *ga_image_new(int sizex, int sizey){
	ga_image_t *i = (ga_image_t*)malloc(sizeof(ga_image_t));
	int px = sizex*sizey;
	i->sizex = sizex;
	i->sizey = sizey;
	i->pixel = (vec_t*)malloc(sizex*sizey*sizeof(vec_t));
	memset(i->pixel,0,sizex*sizey*sizeof(vec_t));
	i->zbuffer = (float*)malloc(sizex*sizey*sizeof(float));
	while(px--){
		i->zbuffer[px] = -FLT_MAX;
	}
	return i;
}
static char float_to_char(float v){
	if(v >= 1.0f){
		return (char)255;
	}else if (v <0.0f){
		return 0;
	}else{
		return (char)(v*255.0);
	}
}
void	ga_image_set_pixel(ga_image_t *i, int x, int y, vec_t color){
	color = vec_max(vec_min(color,vec_new(1,1,1,1)),vec_new(0,0,0,0));
	i->pixel[i->sizex*(i->sizey-y -1) + x] = color;
}
void	ga_image_set_zpixel(ga_image_t *i, int x, int y, float z, vec_t color){
	if(i->zbuffer[i->sizex*(i->sizey-y-1)+x] < z){
		i->zbuffer[i->sizex*(i->sizey-y-1) + x] = z;
		ga_image_set_pixel(i,x,y,color);
	}
}
void	ga_image_fill(ga_image_t *img,vec_t color){
	int i = img->sizex*img->sizey;
	while(i--){
		img->pixel[i] = color;
	}
}
/* TODO : understand the png saving voodoo incantation */
void	ga_image_save(ga_image_t *img, char *path){
	png_byte color_type = PNG_COLOR_TYPE_RGBA;
	png_byte bit_depth = 8;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_ptr;
	float *buf = (float*)img->pixel;
	int i = img->sizey;
	int j = img->sizex;
	int k = 4;
	FILE *fp = fopen(path,"wb");
	if(!fp){
		fprintf(stderr,"ERROR: couldn't write file '%s'\n",path);
	}
	row_ptr = (png_bytep*)malloc(img->sizey*sizeof(png_bytep));
	while(i--){
		row_ptr[i] = (png_byte*)malloc(img->sizex*4*sizeof(png_byte));
		j = img->sizex;
		while(j--){
			k = 4;
			while(k--){
				row_ptr[i][j*4+k] = float_to_char(buf[i*img->sizex*4+j*4+k]);
			}
		}
	}
	png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));
	png_init_io(png_ptr,fp);
	setjmp(png_jmpbuf(png_ptr));
	png_set_IHDR(png_ptr,info_ptr,img->sizex,img->sizey,
			bit_depth, color_type, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
	setjmp(png_jmpbuf(png_ptr));
	png_write_image(png_ptr,row_ptr);
	setjmp(png_jmpbuf(png_ptr));
	png_write_end(png_ptr,NULL);
	fclose(fp);
	i = img->sizey;
	while(i--){
		free(row_ptr[i]);
	}
	free(row_ptr);
}
