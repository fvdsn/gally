#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "ga_raytrace.h"
#include "ga_img.h"
#include "ga_kdt.h"
#include "ga_photonmap.h"


static void ga_shade_emit(vec_t *color, const ga_material_t *mat){
	*color = vec_add(*color,vec_scale(mat->emit_factor,mat->emit_color));
}
static void ga_shade_flat(vec_t *color, const ga_material_t *mat, const vec_t *lcolor,float factor){
	*color = vec_add(*color,vec_scale(mat->flat_factor*factor,vec_mult(mat->flat_color,*lcolor)));
}
static void ga_shade_diffuse(vec_t *color, const ga_material_t *mat,const vec_t *lcolor, const vec_t *ldir, const vec_t *norm, float factor){
	float fact = vec_dot(*norm,*ldir);
	if(fact > 0.0f){
		*color = vec_add(*color,
			vec_scale(fact*factor*mat->diff_factor
				,vec_mult(mat->diff_color,*lcolor)));
	}
}
static void ga_shade_phong(vec_t *color, const ga_material_t *mat,const vec_t *lcolor, const vec_t *ldir, const vec_t *dir, const vec_t *norm, float factor){
	vec_t r;
	float fact;
	float power;
	r = vec_sub(
		vec_scale(2.0,vec_scale(vec_dot(*norm,*ldir),*norm)),
		*ldir);	
	if((fact = vec_dot(r,vec_neg(*dir))) > 0.0f){
		power = powf(fact,mat->spec_power)*mat->spec_factor;
		*color = vec_add(*color, vec_scale( power*factor,
				vec_mult(mat->spec_color,*lcolor)));
	}
}
static void ga_shade_reflect(vec_t *color, const ga_material_t *mat);
static void ga_shade_ao(vec_t *color, const ga_scene_t *s, const ga_material_t *mat, const vec_t *pos, const vec_t *norm, float importance){
	int sample = mat->ao_sample;
	float rlen,fact;
	vec_t ao_color = vec_new(0,0,0,1);
	vec_t dsample,d1,d2;
	vec_fperp(norm,&d1,&d2);
	while(sample--){
		dsample = vec_new(0,0,0,1);
		vec_ffadd(&dsample,random()*RAND_NORM,norm);
		vec_ffadd(&dsample,(2.0f*random()*RAND_NORM) -1.0f,&d1);
		vec_ffadd(&dsample,(2.0f*random()*RAND_NORM) -1.0f,&d2);
		vec_fnorm(&dsample);
		rlen = ga_ray_length(s,*pos,dsample);
		if (rlen <= mat->ao_min_dist){
			ao_color = vec_add(ao_color,mat->ao_min_color);
		}else if (rlen >= mat->ao_max_dist){
			ao_color = vec_add(ao_color,mat->ao_max_color);
		}else{
			fact = (rlen - mat->ao_min_dist) / (mat->ao_max_dist - mat->ao_min_dist) ;
			ao_color = vec_add(ao_color,
					vec_add(vec_scale(fact,mat->ao_max_color),
						vec_scale(1.0f-fact,mat->ao_min_color)));
		}
	}
	*color = vec_add(*color,
			vec_scale(1.0f/mat->ao_sample*mat->ao_factor,ao_color));
}

static void ga_shade_sky(vec_t *color, const ga_scene_t *s, const ga_material_t *mat, const vec_t *pos, const vec_t *norm, float importance){
	int sample = mat->gi_sample;
	float rlen;
	vec_t sky_color = vec_new(0,0,0,1);
	vec_t dsample,d1,d2;
	vec_fperp(norm,&d1,&d2);
	while(sample--){
		dsample = vec_new(0,0,0,1);
		vec_ffadd(&dsample,random()*RAND_NORM,norm);
		vec_ffadd(&dsample,(2.0f*random()*RAND_NORM) -1.0f,&d1);
		vec_ffadd(&dsample,(2.0f*random()*RAND_NORM) -1.0f,&d2);
		rlen = ga_ray_length(s,*pos,dsample);
		if(rlen >= FLT_MAX){
			ga_shade_diffuse(&sky_color,mat,&(s->bg_color),&dsample,norm,1.0f);
		}
	}
	*color = vec_add(*color,
			vec_scale(1.0f/mat->gi_sample*mat->gi_factor,sky_color));
}
static void ga_shade_gi(vec_t *color, const ga_scene_t *s, const ga_material_t *mat, const vec_t *pos, const vec_t *norm){
	float sqradius = s->pm_resolution*s->pm_resolution;
	vec_t gi_color = vec_new(0,0,0,1);
	vec_t lcolor;
	ga_node_t *n = ga_photonmap_get(s->pm,pos)->first;
	ga_photon_t *p = NULL;
	float factor,dist;
	float energy = 0;
	while(n){
		p = (ga_photon_t*)n->data;
		dist = vec_fsqdist(pos,&(p->pos));
		if(dist < sqradius && vec_fdot(norm,&(p->normal)) >= 0.8f){
			factor = 1.0f/ (3.141592 * sqradius) *mat->gi_factor;
			energy += factor;
			lcolor = vec_scale(factor,p->color);
			ga_shade_diffuse(&gi_color,mat,&lcolor,&(p->ldir),norm,1.0f);
		}
		n = n->next;
	}
	*color = vec_add(*color,gi_color);
			
}					

vec_t ga_ray_shade(	const vec_t * pos, 
			const vec_t * dir, 
			const vec_t * norm,
			const ga_material_t *mat,
			const ga_scene_t *s,
			float importance, 
			int max_rec){
	ga_node_t *n        = NULL; 
	ga_light_t *light   = NULL;
	ga_material_t *comb = NULL;
	vec_t color = vec_new(0,0,0,1);
	vec_t dpos = vec_add(*pos,vec_scale(0.00001,*norm));
	vec_t lsample,d1,d2,lcolor,ldir;
	int sample;
	float len;
	float rlen;
	/*dir = vec_norm(dir);*/
	if(mat->type == GA_MATERIAL_BLENDING){
		return vec_scale(mat->blend_factor,
		ga_ray_shade(pos,dir,norm,mat->child,s,importance*mat->blend_factor, max_rec));
	}
	if(mat->type == GA_MATERIAL_COMB){
		n = mat->comb->first;
		while(n){
			comb = (ga_material_t*)n->data;
			color = vec_add(color, ga_ray_shade(pos,dir,norm,comb,s,importance,max_rec));
			n = n->next;
		}
		return color;
	}
	if(mat->emit_factor != 0.0f){
		ga_shade_emit(&color,mat);
	}
	if(mat->ao_factor != 0.0f){
		ga_shade_ao(&color,s,mat,&dpos,norm,importance && max_rec > 0);
	}
	if(mat->gi_factor != 0.0f && mat->diff_factor != 0.0f && max_rec > 0){
		ga_shade_gi(&color,s,mat,&dpos,norm);
	}
	if(mat->gi_sample){
		ga_shade_sky(&color,s,mat,&dpos,norm,importance);
	}
		
	n = s->light->first;
	if(n && (mat->diff_factor != 0.0f || mat->spec_factor != 0.0f ||
				mat->flat_factor != 0.0f )){
		while(n){	/* for each light in scene */
			light = (ga_light_t*)n->data;
			sample = light->samples;
			lsample = light->pos;
			ldir = vec_sub(light->pos,dpos);
			vec_fperp(&ldir,&d1,&d2);
			while(sample--){	/*for each sample in light */
				do{
					lsample = vec_add(
							vec_scale((-1.0f + 2.0f*random()*RAND_NORM)*light->radius,d1),
							vec_scale((-1.0f + 2.0f*random()*RAND_NORM)*light->radius,d2));
				}while(vec_len(lsample) > light->radius + 0.000001);

				lsample = vec_add(light->pos,lsample);	/* sampled light position */
				ldir = vec_norm(vec_sub(lsample,dpos));  
				len = vec_len(vec_sub(light->pos,dpos));	/* distance to light */
				rlen = ga_ray_length(s,dpos,ldir);	
				if(len < rlen){ /* light is visible */
					len *= len;
					len = 1.0/len;
					lcolor = vec_scale(len*light->color.w,light->color);
					if(mat->diff_factor != 0.0f){
						ga_shade_diffuse(&color,mat,&lcolor,&ldir,norm,1.0/light->samples);
					}
					if(mat->spec_factor != 0.0f){
						ga_shade_phong(&color,mat,&lcolor,&ldir,dir,norm,1.0/light->samples);
					}
					if(mat->flat_factor != 0.0f){
						ga_shade_flat(&color,mat,&lcolor,1.0/light->samples);
					}
				}
			}
			n = n->next;
		}
	}
	return color;
}
