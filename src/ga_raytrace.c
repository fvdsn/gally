#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <pthread.h>
#include "ga_raytrace.h"
#include "ga_img.h"
#include "ga_kdt.h"
static  int ga_ray_length(const ga_scene_t *s,vec_t origin,vec_t dir){
	float u = 0.0f;
	float v = 0.0f;
	float t = 0.0;
	tri_t *tri = NULL;
	if( ga_kdn_trace(s->kdtree,s->box_min,s->box_max,
			origin,dir,&tri,&u,&v,&t) && t > 0){
		return 1;
	}else{
		return 0;
	}
}
static void ga_material_shade(	vec_t *color, 
				const vec_t *pos,
				const vec_t *dir, 
				const vec_t *norm, 
				const vec_t *ldir, 
				const vec_t *lcol,
				const vec_t *uv,
				const ga_material_t *mat){
	float fact = 0.0f;
	float power = 0.0f;
	vec_t tmp_color = vec_new(0,0,0,1);
	ga_node_t *n;
	ga_material_t *comb;
	vec_t r;
	switch(mat->type){
		case GA_MATERIAL_PHONG:
			r = vec_sub(
				vec_scale(2.0,vec_scale(vec_dot(*norm,*ldir),*norm)),
				*ldir);	
			if((fact = vec_dot(r,vec_neg(*dir))) > 0.0f){
				power = powf(fact,mat->power);
				*color = vec_add(*color,
					vec_scale( power,
						vec_mult(mat->color,*lcol)));
			}
			break;
		case GA_MATERIAL_DIFFUSE:
			if((fact = vec_dot(*norm,*ldir)) > 0.0f){
				*color = vec_add(*color,
					vec_scale(fact,vec_mult(mat->color,*lcol)));
			}
			break;
		case GA_MATERIAL_EMIT:
			*color = vec_add(*color,mat->color);
			break;
		case GA_MATERIAL_FLAT:
			*color = vec_add(*color,vec_mult(mat->color,*lcol));
			break;
		case GA_MATERIAL_BLENDING:
			ga_material_shade(&tmp_color,pos,dir,norm,ldir,lcol,uv,mat->child);
			*color = vec_add(*color,vec_scale(mat->alpha,tmp_color));
			break;
		case GA_MATERIAL_COMB:
			n = mat->comb->first;
			while(n){
				comb = (ga_material_t*)n->data;
				ga_material_shade(color,pos,dir,norm,ldir,lcol,uv,comb);
				n = n->next;
			}
			break;
		default:break;
	}
}
/**
 * Returns the shading color from ray intersection point at pos, view point is
 * from dir, intersection normal is at norm, surface material is mat, scene is
 * s
 */
static vec_t ga_ray_shade(vec_t pos, vec_t dir, vec_t norm,const ga_material_t *mat,ga_scene_t *s){
	ga_node_t *n = s->light->first;
	ga_light_t *light;
	vec_t ldir;
	vec_t color = vec_new(0,0,0,1);
	vec_t uv = vec_new(0,0,0,1);
	vec_t lcolor = vec_new(0,0,0,1);
	float fact;
	float len;

	dir = vec_norm(dir);
	if(n){	/*if there is a lamp in the scene*/ 
		while(n){
			light = (ga_light_t*)n->data;
			ldir = vec_norm(vec_sub(light->pos,pos));
			len = vec_len(vec_sub(light->pos,pos)) + 0.001;
			if(!ga_ray_length(s,vec_add(pos,vec_scale(1,ldir)),ldir)){
				len *= len;
				len = 1.0/len;
				lcolor = vec_scale(len*light->color.w,light->color);
				ga_material_shade(	&color,
								&pos,
								&dir,
								&norm,
								&ldir,
								&(lcolor),
								&uv,
								mat);
				}
			n = n->next;
		}
		return color;
	}else{	/* there is no light in the scene so we draw a default shading */
		fact = -vec_dot(norm,dir);
		if(fact < 0.0f){ fact = 0.0f; }
		return vec_scale(fact,mat->color);
	}
}
/**
 * Returns the color from launching a ray at start in direction dir, in scene
 * s
 */
vec_t ga_ray_trace(ga_scene_t *s, vec_t start, vec_t dir){
	tri_t *tri  = NULL;
	float t = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	float ab = 0.0f;
	int   nnode = 0;
	vec_t normal;
	vec_t pos;
	vec_t ret;
	dir = vec_norm(dir);
	if(ga_kdn_trace(s->kdtree,s->box_min,s->box_max,start,dir,
				&tri,&u,&v,&t)){
		normal = vec_add(vec_scale(u,tri->vnorm[1]),
			 vec_add(vec_scale(v,tri->vnorm[2]),
				vec_scale(1.0f-u-v,tri->vnorm[0])));
		pos = vec_add(start,vec_scale(t,dir));
		return ga_ray_shade( pos,dir,normal,(ga_material_t*)tri->material,s );
	}
	ret = s->bg_color;
	ret.x += ab*0.05;
	ret.z += nnode/255.0f;
	return ret;
}
/* render a part of the render image to the scene output buffer.
 * see ga_ray_thread_data_t def in ga_raytrace.h for further indications */
static void *ga_ray_thread_func(void *data){
	ga_ray_thread_data_t *td = (ga_ray_thread_data_t*)data;
	vec_t dir;
	int x = td->px;	/*pixel coordinate*/
	int y = td->py;
	float fx,fy;
	while(x < td->sx){
		y = td->py;
		fx = (float)x/(float)td->scene->img->sizex;
		if((x%(10*GA_THREAD_COUNT)) == 0){
			printf("thread %d, line %d\n",td->id,td->sx - x);
		}
		while(y < td->sy){	/*iterate over pixels */
			fy = (float)y/(float)td->scene->img->sizey;
			/*compute direction to shoot ray */
			dir = vec_norm(vec_add(td->front,vec_add(
						vec_sub( vec_scale(fx,td->cr),
							vec_scale(1.0-fx,td->cr)),
						vec_sub( vec_scale(fy,td->cu),
							vec_scale(1.0-fy,td->cu)))));
			/*launch the ray and sets the result in the image */
			ga_image_set_pixel(td->scene->img,x,y,
					ga_ray_trace(td->scene,td->origin,dir));
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
	while(i--){
		thread_data[i].id = i;
		thread_data[i].front = front;
		thread_data[i].origin = origin;
		thread_data[i].cu = cu;
		thread_data[i].cr = cr;
		thread_data[i].scene = s;
		thread_data[i].px = 0;
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

