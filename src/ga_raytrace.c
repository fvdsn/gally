#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ga_raytrace.h"
#include "ga_img.h"
#define EPSILON 0.000001f

int   ga_ray_intersect(const tri_t *tr, const vec_t *start, const vec_t * dir, float *t, float*u,float*v){
	vec_t edge1; 
	vec_t edge2; 
	vec_t pvec; 
	vec_t tvec;
	vec_t qvec;
	float det;
	float inv_det;
	if(vec_fdot(dir,&(tr->norm)) >0){
		return 0;
	}
	vec_fsub(&edge1,tr->vert + 1,tr->vert + 0);
	vec_fsub(&edge2,tr->vert + 2,tr->vert + 0);
	vec_fcross(&pvec,dir,&edge2);
	det = vec_fdot(&edge1,&pvec);
	if(det > -EPSILON && det < EPSILON){
		return 0;
	}
	inv_det = 1.0f / det;
	
	vec_fsub(&tvec,start,tr->vert +0);
	*u = vec_fdot(&tvec,&pvec)*inv_det;
	if(*u <-EPSILON || *u >= 1.0 + EPSILON){
		return 0;
	}
	vec_fcross(&qvec,&tvec,&edge1);
	*v = vec_fdot(dir,&qvec)*inv_det;
	if(*v <-EPSILON || *u + *v >1.0f +EPSILON){
		return 0;
	}
	*t = vec_fdot(&edge2,&qvec)*inv_det;
	return 1;
}
static vec_t ga_ray_shade(vec_t pos, vec_t dir, vec_t norm, ga_material_t *mat,ga_scene_t *s){
	ga_node_t *n = s->light->first;
	ga_light_t *light;
	vec_t vl;
	vec_t color = vec_new(0,0,0,1);
	float fact;
	if(n){
		while(n){
			light = (ga_light_t*)n->data;
			vl = vec_norm(vec_sub(light->pos,pos));
			if((fact = vec_dot(norm,vl)) > 0.0f){
				color = vec_add(color,vec_scale(fact,vec_mult(mat->color,light->color)));
			}
			n = n->next;
		}
		return color;
	}else{
		fact = -vec_dot(norm,dir);
		if(fact < 0.0f){ fact = 0.0f; }
		return vec_scale(fact,mat->color);
	}
}
vec_t ga_ray_trace(ga_scene_t *s, vec_t start, vec_t dir){
	int i = 0;
	ga_geom_t *g;
	ga_material_t *material;
	ga_shape_t *shape;
	model_t *m;
	ga_node_t *n = s->shape->first;
	int   first = 1;
	float t = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	float _t = 0.0f;
	float _u = 0.0f;
	float _v = 0.0f;
	vec_t normal;
	tri_t *tr = NULL;	
	vec_t pos;
	dir = vec_norm(dir);
	while(n){
		shape = (ga_shape_t*)n->data;
		g = shape->geom;
		m = g->model;
		material = shape->material;
		if(m){
			i = m->tri_count;
			first = 1;
			while(i--){
				if (ga_ray_intersect(m->tri + i,&start,&dir,&_t,&_u,&_v)){
					if( first || (!first && _t < t)){
						t = _t;
						u = _u;
						v = _v;
						tr = m->tri + i;
						first = 0;
					}

				}
			}
			if(!first){
				normal = vec_add(vec_scale(u,tr->vnorm[1]),
					 vec_add(vec_scale(v,tr->vnorm[2]),
							vec_scale(1.0f-u-v,tr->vnorm[0])));
				pos = vec_add(start,vec_scale(t,dir));
				return ga_ray_shade( pos,dir,normal,material,s );
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
	vec_t right 	= vec_norm(vec_cross(front,up));
	float fov	= s->active_camera->fov/2.0*3.141592/180.0;
	vec_t cr = vec_scale(tanf(fov),right);
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
	s = ga_scene_load(argv[1]);
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


