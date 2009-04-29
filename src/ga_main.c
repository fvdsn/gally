#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include "ga_raytrace.h"
#include "ga_raster.h"
#include "ga_kdt.h"

static const char *help_text = "Mendatory arguments : \
\n\t-scene=PATH   : the path to the sdl scene file\n\
Optional arguments:\
\n\t-ray(tracer)  : use the raytracer.  (default method)\
\n\t-ras(teriser) : use the rasteriser.\
\n\t-out=PATH     : path to the png output -default : out.png. \
\n\t-sx=INT       : the horizontal rendering size (default:256)\
\n\t-sy=INT       : the vertical rendering size   (default:256)\
\n\t-samples=INT  : oversampling (default:1)\
\n\t-dither=FLOAT : dithering [0,1], (default:0)\
\n\t-photonmap=FLOAT : photon radius (default:0.2)\
\n\t-h(elp)       : displays this message.\n";

int main(int argc, char **argv){
	char * sdl_path = NULL;
	char * png_path = "out.png";
	int	sizex = 512;
	int 	sizey = 512;
	int	raytrace = 1;
	int	samples  = 1;
	float 	pm_res   = 0.2;
	float   dither = 0.0f;
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
		}else if(!strncmp(argv[i],"-samples=",9)){
			samples = (int)strtol(argv[i]+9,NULL,10);
			if(samples <= 0){
				fprintf(stderr,"WARNING: invalid sampling:%d, set to default(1)\n",samples);
			}else if (samples > 8){
				fprintf(stderr,"WARNING: very high sampling:%d, render will be slow\n",samples);
			}
		}else if(!strncmp(argv[i],"-photonmap=",11)){
			pm_res = (float)strtod(argv[i]+11,NULL);
			if(pm_res <= 0){
				fprintf(stderr,"WARNING: invalid photon radius:%f, set to default(0.2)\n",pm_res);
			}else if (pm_res > 0.5){
				fprintf(stderr,"WARNING: very high photon radius :%f, render will be slow\n",pm_res);
			}
		}else if(!strncmp(argv[i],"-dither=",8)){
			dither = (float)strtod(argv[i]+8,NULL);
			if(dither < 0.0f || dither > 1.0f){
				fprintf(stderr,"WARNING: dithering beyond reasonable range [0,1] : %f\n",dither);
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
	ga_scene_set_sampling(s,samples);
	ga_scene_set_dithering(s,dither);
	ga_scene_set_pm_resolution(s,pm_res);
	if(raytrace){
		printf("Applying scene graph transforms\n");
		ga_scene_build_triangle(s);
		printf("Building Scene Bounding Box\n");
		ga_scene_build_bounding_box(s);
		vec_print(s->box_min);
		vec_print(s->box_max);
		printf("Building KD-Tree\n");
		ga_scene_build_kdtree(s);
		printf("Rendering Photon Pass ... \n");
		ga_ray_gi_compute(s);
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
