#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "ga_raytrace.h"
#include "ga_raster.h"
#include "ga_kdtree.h"

static const char *help_text = "Mendatory arguments : \
\n\t-scene=PATH   : the path to the sdl scene file\n\
Optional arguments:\
\n\t-ray(tracer)  : use the raytracer.  (default method)\
\n\t-ras(teriser) : use the rasteriser.\
\n\t-out=PATH     : path to the png output -default : out.png. \
\n\t-sx=INT       : the horizontal rendering size (default:256)\
\n\t-sy=INT       : the vertical rendering size   (default:256)\
\n\t-h(elp)       : displays this message.\n";

int main(int argc, char **argv){
	char * sdl_path = NULL;
	char * png_path = "out.png";
	int	sizex = 256;
	int 	sizey = 256;
	int	raytrace = 1;
	ga_scene_t *s = NULL;
	int i = argc;
	while(i--){
		if(i == 0){break;}
		if(!strncmp(argv[i],"-ray",4)){
			raytrace = 1;
		}else if(!strncmp(argv[i],"-ras",4)){
			raytrace = 0;
		}else if(!strncmp(argv[i],"-out=",5)){
			png_path = argv[i] + 5;
		}else if(!strncmp(argv[i],"-scene=",7)){
			sdl_path = argv[i] + 7;
		}else if(!strncmp(argv[i],"-sx=",4)){
			sizex = (int)strtol(argv[i]+4,NULL,10);
			if(sizex <= 0){
				fprintf(stderr,"WARNING: invalid x size:%d, set to default (256)\n",sizex);
				sizex = 256;
			}
		}else if(!strncmp(argv[i],"-sy=",4)){
			sizey = (int)strtol(argv[i]+4,NULL,10);
			if(sizey <= 0){
				fprintf(stderr,"WARNING: invalid y size:%d, set to default (256)\n",sizey);
				sizey = 256;
			}
		}else if(!strncmp(argv[i],"-h",2)){
			printf("%s",help_text);
			return 0;
		}else{
			fprintf(stderr,"WARNING:invalid argument:%s\n",argv[i]);
		}
	}
	if(!sdl_path){
		fprintf(stderr,"ERROR: you must specify a scene file as argument : '-scene=PATH' \n");
		return 1;
	}
	s = ga_scene_load(sdl_path);
	if(!s){
		fprintf(stderr,"ERROR: couldn't load scene \n");
		return 1;
	}else if(!s->active_camera){
		fprintf(stderr,"ERROR: the scene doesn't have an active camera \n");
		return 1;
	}
	ga_scene_set_image(s,sizex,sizey);
	if(raytrace){
		printf("Applying scene graph transforms\n");
		ga_scene_build_triangle(s);
		printf("Building Scene Bounding Box\n");
		ga_scene_build_bounding_box(s);
		vec_print(s->box_min);
		vec_print(s->box_max);
		printf("Building KD-Tree\n");
		ga_scene_build_kdtree(s);
		printf("Printing KD-Tree\n");
		/*ga_kdtree_print(s->kdtree,0);*/
		printf("Rendering pixel collumn ...\n");
		ga_ray_render(s);
		printf("Done\n");
	}else{
		printf("Rendering polygon number ...\n");
		ga_raster_render(s);
		printf("Done\n");
	}	
	ga_scene_save_image(s,png_path);
	printf("Render saved at '%s'\n",png_path);
	return 0;
}
