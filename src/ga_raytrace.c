#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <pthread.h>
#include "ga_raytrace.h"
#include "ga_img.h"
#include "ga_kdt.h"

float ga_ray_length(const ga_scene_t *s,vec_t origin,vec_t dir){
	float u = 0.0f;
	float v = 0.0f;
	float t = FLT_MAX;
	tri_t *tri = NULL;
	if(ga_kdn_trace(s->kdtree,s->box_min,s->box_max,origin,dir,&tri,&u,&v,&t)){
		return t;
	}else{
		return FLT_MAX;
	}
}
static void ga_ray_photon_trace(const ga_scene_t *s, vec_t pos, vec_t dir, vec_t color, int max_rec, int min_store){
	ga_material_t *mat;
	tri_t *tri = NULL;
	float t = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	vec_t normal,dpos,d1,d2,ddir;
	if(max_rec <=0){
		return;
	}else if(ga_kdn_trace(s->kdtree,s->box_min,s->box_max,pos,dir,&tri,&u,&v,&t)){
		mat = tri->material;
		normal = vec_add(vec_scale(u,tri->vnorm[1]),
			 vec_add(vec_scale(v,tri->vnorm[2]),
				vec_scale(1.0f-u-v,tri->vnorm[0])));
		dpos = vec_add(vec_add(pos,vec_scale(t,dir)),vec_scale(0.0001,normal));
		dir  = vec_neg(dir);
		if(min_store <= 0){
			ga_photonmap_add(s->pm,&dpos,&color,&normal,&dir);
		}
		color = vec_mult(color,vec_scale(mat->diff_factor,mat->diff_color));
		vec_fperp(&normal,&d1,&d2);

		ddir = vec_new(0,0,0,1);
		vec_ffadd(&ddir,random()*RAND_NORM,&normal);
		vec_ffadd(&ddir,(2.0f*random()*RAND_NORM) -1.0f,&d1);
		vec_ffadd(&ddir,(2.0f*random()*RAND_NORM) -1.0f,&d2);

		ga_ray_photon_trace(s,dpos,ddir,color,max_rec -1,min_store -1);
	}
}
void ga_ray_gi_compute(ga_scene_t *s){
	ga_node_t *n = s->light->first;
	ga_light_t *l;
	int photons;
	vec_t dir;
	s->pm = ga_photonmap_new(s->box_min,s->box_max,s->pm_resolution);
	while(n){
		l = (ga_light_t*)n->data;
		photons = l->photons;
		printf("Light : %s, photons:%d \n",l->name,photons);
		while(photons--){
			dir.x = (2.0f*random()*RAND_NORM) - 1.0f;
			dir.y = (2.0f*random()*RAND_NORM) - 1.0f;
			dir.z = (2.0f*random()*RAND_NORM) - 1.0f;
			dir.w = 1;
			vec_fnorm(&dir);
			ga_ray_photon_trace(s,l->pos,dir,vec_scale(1.0f/l->photons*l->color.w*l->photon_weight,l->color),3,1);
		}
		n = n->next;
	}
}


/**
 * Returns the color from launching a ray at start in direction dir, in scene
 * s
 */
vec_t ga_ray_trace(const ga_scene_t *s, vec_t start, vec_t dir,float importance,int max_rec, vec_t *dpos){
	tri_t *tri  = NULL;
	float t = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	vec_t normal;
	vec_t pos;
	dir = vec_norm(dir);
	if(max_rec < 0){
		return s->bg_color;
	}
	if(ga_kdn_trace(s->kdtree,s->box_min,s->box_max,start,dir,
				&tri,&u,&v,&t)){
		normal = vec_add(vec_scale(u,tri->vnorm[1]),
			 vec_add(vec_scale(v,tri->vnorm[2]),
				vec_scale(1.0f-u-v,tri->vnorm[0])));
		pos = vec_add(start,vec_scale(t,dir));
		if(dpos){
			*dpos = pos;
		}
		return ga_ray_shade( &pos,&dir,&normal,(ga_material_t*)tri->material,s,importance,max_rec);
	}
	if(dpos){
		*dpos = vec_new(FLT_MAX,FLT_MAX,FLT_MAX,1);
	}
	return s->bg_color;
}
/*
static vec_t ga_ray_trace_photonmap(const ga_scene_t *s, vec_t start, vec_t dir){
	tri_t *tri  = NULL;
	float t = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	vec_t pos;
	dir = vec_norm(dir);
	if(ga_kdn_trace(s->kdtree,s->box_min,s->box_max,start,dir,
				&tri,&u,&v,&t)){
		pos = vec_add(start,vec_scale(t,dir));
		return ga_photonmap_get(s->pm,pos);
	}
	return s->bg_color;
}*/
/* render a part of the render image to the scene output buffer.
 * see ga_ray_thread_data_t def in ga_raytrace.h for further indications */
static void *ga_ray_thread_func(void *data){
	ga_ray_thread_data_t *td = (ga_ray_thread_data_t*)data;
	vec_t dir;
	int x = td->px;	/*pixel coordinate*/
	int y = td->py;
	int px,py;
	float fx,fy;
	vec_t color;
	float importance = 1.0/(td->samples*td->samples);
	while(x < td->sx){
		y = td->py;
		if((x%(10*GA_THREAD_COUNT)) == 0){
			printf("thread %d, line %d\n",td->id,td->sx - x);
		}
		while(y < td->sy){	/*iterate over pixels */
			color = vec_new(0,0,0,1);
			px = 0;
			while(px < td->samples){
				py = 0;
				while(py < td->samples){	/*iterate over samples*/
					fx = (x +(float)px/(float)td->samples)/(float)td->scene->img->sizex;
					fy = (y +(float)py/(float)td->samples)/(float)td->scene->img->sizey;
					/*compute direction to shoot ray */
					dir = vec_norm(vec_add(td->front,vec_add(
							vec_sub( vec_scale(fx,td->cr),
								vec_scale(1.0-fx,td->cr)),
							vec_sub( vec_scale(fy,td->cu),
								vec_scale(1.0-fy,td->cu)))));
					/*accumulate samples*/
					color = vec_add(color,
						vec_scale(importance,
						ga_ray_trace(td->scene,td->origin,dir,importance,10,NULL)));
					      	/*ga_ray_trace_photonmap(td->scene,td->origin,dir)));*/
					py++;
				}
				px++;
			}
			color = vec_add(color,vec_new(
				((float)random()/(float)RAND_MAX)*td->dither/256.0f,
				((float)random()/(float)RAND_MAX)*td->dither/256.0f,
				((float)random()/(float)RAND_MAX)*td->dither/256.0f,
				0));
			ga_image_set_pixel(td->scene->img,x,y,color);
			y++;
		}
		x++;
	}
	return NULL;
}
/* thread parameters */
static ga_ray_thread_data_t  thread_data[GA_THREAD_COUNT];
/* thread references */
static pthread_t	thread[GA_THREAD_COUNT];

/* renders the scene to s-img with raytracing */
void ga_ray_render(ga_scene_t *s){
	vec_t origin 	= s->active_camera->pos;
	vec_t front  	= vec_norm(s->active_camera->dir);
	vec_t right 	= vec_norm(vec_cross(front,s->active_camera->up));
	vec_t up	= vec_norm(vec_cross(right,front));
	float fov	= s->active_camera->fov/2.0*3.141592/180.0;
	vec_t cr = vec_scale(tanf(fov),right);	/*this vector points to the right
						  center of the rendering*/
	vec_t cu = vec_scale(tanf(fov)*s->img->sizey/(float)s->img->sizex,up);
						/*this vector points to the top
						 * center */
	int i = GA_THREAD_COUNT;
	int dy = (s->img->sizey / GA_THREAD_COUNT) + 1;
	printf("Sampling : %d\n",s->samples);
	while(i--){
		thread_data[i].id = i;
		thread_data[i].front = front;
		thread_data[i].origin = origin;
		thread_data[i].cu = cu;
		thread_data[i].cr = cr;
		thread_data[i].scene = s;
		thread_data[i].px = 0;
		thread_data[i].samples = s->samples;
		thread_data[i].dither = s->dither;
		thread_data[i].sx = s->img->sizex;
		if(dy*(i+1) > s->img->sizey){
			thread_data[i].sy = s->img->sizey;
		}else{
			thread_data[i].sy = dy*(i+1);
		}
		thread_data[i].py = dy*i;
		if(pthread_create(thread + i, NULL,ga_ray_thread_func,thread_data + i)){
			fprintf(stderr,"ERROR: couldn't create render thread\n");
			return;
		}
	}
	i = GA_THREAD_COUNT;
	while(i--){
		pthread_join(thread[i],NULL);
	}
}

