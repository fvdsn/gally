#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ga_raytrace.h"
#include "ga_img.h"
#define EPSILON 0.000001

int   ga_ray_intersect(tri_t *tr, vec_t start, vec_t dir, float *t, float*u,float*v){
	vec_t edge1; 
	vec_t edge2; 
	vec_t pvec; 
	vec_t tvec;
	vec_t qvec;
	float det;
	float inv_det;
	edge1 = vec_sub(tr->vert[1],tr->vert[0]);
	edge2 = vec_sub(tr->vert[2],tr->vert[0]);
	pvec  = vec_cross(dir,edge2);
	det = vec_dot(edge1,pvec);
	if(det > -EPSILON && det < EPSILON){
		return 0;
	}
	inv_det = 1.0f / det;
	
	tvec = vec_sub(start,tr->vert[0]);
	*u = vec_dot(tvec,pvec)*inv_det;
	if(*u <0.0 || *u > 1.0){
		return 0;
	}
	qvec = vec_cross(tvec,edge1);
	*v = vec_dot(dir,qvec)*inv_det;
	if(*v <0.0f || *u + *v >1.0f){
		return 0;
	}
	*t = vec_dot(edge2,qvec)*inv_det;
	return 1;
}
vec_t ga_ray_trace(ga_scene_t *s, vec_t start, vec_t dir){
	int i = 0;
	ga_geom_t *g;
	ga_material_t *material;
	ga_shape_t *shape;
	model_t *m;
	ga_node_t *n = s->shape->first;
	int   first = 1;
	float t,u,v,_t,_u,_v;
	float fact;
	tri_t *tr;	
	while(n){
		shape = (ga_shape_t*)n->data;
		g = shape->geom;
		m = g->model;
		material = shape->material;
		if(m){
			i = m->tri_count;
			first = 1;
			while(i--){
				if (ga_ray_intersect(m->tri + i,start,dir,&_t,&_u,&_v)){
					if(vec_dot(dir,(m->tri+i)->norm) < 0.0f &&
							(first || (!first && _t < t))){
						t = _t;
						u = _u;
						v = _v;
						tr = m->tri + i;
						first = 0;
					}

				}
			}
			if(!first){
					fact = 0.0f;
					fact += u*vec_dot(dir,tr->vnorm[1]);
					fact += v*vec_dot(dir,tr->vnorm[2]);
					fact += (1.0f-u-v)*vec_dot(dir,tr->vnorm[0]);
					if(fact < 0){
						return vec_scale(-fact,material->color);
					}
			}
		}
		n = n->next;
	}
	return s->bg_color;
}
void ga_ray_render(ga_scene_t *s){
	vec_t origin 	= s->active_camera->pos;
	vec_t front  	= s->active_camera->dir;
	vec_t up	= s->active_camera->up;
	vec_t right 	= vec_print(vec_norm(vec_cross(front,up)));
	float fov	= s->active_camera->fov/2.0*3.141592/180.0;
	vec_t cr = vec_print(vec_scale(tanf(fov),right));
	vec_t cu = vec_scale(tanf(fov)*s->img->sizey/(float)s->img->sizex,up);
	int x = 0;
	int y = 0;
	vec_t dir;
	float fx, fy;
	while(x < s->img->sizex){
		y = 0;
		fx = (float)x/(float)s->img->sizex;
		printf("x:%d\n",x);
		while(y < s->img->sizey){
			fy = (float)y/(float)s->img->sizey;
			dir = vec_norm(vec_add(front,vec_add(
						vec_sub( vec_scale(fx,cr),
							vec_scale(1.0-fx,cr)),
						vec_sub( vec_scale(fy,cu),
							vec_scale(1.0-fy,cu)))));
			ga_image_set_pixel(s->img,x,y,ga_ray_trace(s,origin,dir));
			y++;
		}
		x++;
	}
}
int main(int argc, char **argv){
	ga_scene_t *s;
	if(argc < 2){
		fprintf(stderr,"ERROR: you must specify a scene file as argument\n");
		return 1;
	}
	s = ga_scene_load("xml/bunny.sdl");
	if(!s->active_camera){
		fprintf(stderr,"ERROR: the scene doesn't have an active camera \n");
		return 1;
	}
	ga_scene_set_image(s,200,200);
	printf("Starting render ...\n ");
	ga_ray_render(s);
	printf("Done\n");
	ga_scene_save_image(s);
	return 0;
}

/*
int main(int argc, char **argv){
	ga_image_t *img = ga_image_new(100,100);
	tri_t tr;
	float t,u,v;
	int i = 100;
	int j = 100;
	int k = 4;
	int r;
	tr.vert[0] = vec_new(0.05,0.1,0,1);
	tr.vert[1] = vec_new(0.1,0.9,0,1);
	tr.vert[2] = vec_new(0.9,0.1,0,1);
	tri_print(&tr);
	ga_image_fill(img,vec_new(0,0,0,1));
	vec_t dir = vec;
	vec_t start;
	while(i--){
		j = 100;
		while(j--){
			k = 0;
			k += ga_ray_intersect(&tr,vec_new(i*0.01,j*0.01,-0.5,0),vec_new(0,0,1,0),
						&t,&u,&v	)){
				ga_image_set_pixel(img,i,j,vec_new(u,v,1-u-v,1));
			}
		}
	}
	ga_image_save(img,"out.png");
	return 0;
}*/

