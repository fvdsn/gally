#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include"ga_photonmap.h"


ga_photon_t *ga_photon_new(vec_t pos, vec_t color){
	ga_photon_t * p= (ga_photon_t*)malloc(sizeof(ga_photon_t));
	p->pos = pos;
	p->color = color;
	return p;
}
ga_photonmap_t * ga_photonmap_new(vec_t bound_min,vec_t bound_max, vec_t outside, int resolution){
	ga_photonmap_t *pm = (ga_photonmap_t*)malloc(sizeof(ga_photonmap_t));
	pm->bound_min = bound_min;
	pm->bound_max = bound_max;
	pm->size = vec_sub(bound_max,bound_min);
	pm->outside = outside;
	pm->resolution = resolution;
	pm->interval = vec_scale(1.0f/(float)(resolution-1),pm->size);
	pm->inv_size.x = 1.0f/pm->size.x;
	pm->inv_size.y = 1.0f/pm->size.y;
	pm->inv_size.z = 1.0f/pm->size.z;
	pm->inv_interval.x = 1.0f/pm->interval.x;
	pm->inv_interval.y = 1.0f/pm->interval.y;
	pm->inv_interval.z = 1.0f/pm->interval.z;
	pm->map = (vec_t*)malloc(resolution*resolution*resolution*sizeof(vec_t));
	memset(pm->map,0,resolution*resolution*resolution*sizeof(vec_t));
	pm->pmap = (vec_t*)malloc(resolution*resolution*resolution*sizeof(ga_list_t));
	memset(pm->map,0,resolution*resolution*resolution*sizeof(ga_list_t));
	return pm;
}

#define PM_XYZ(pm,x,y,z)  ((pm)->map  + (x) + (y) * (pm)->resolution + (z) * (pm)->resolution * (pm)->resolution) 
#define PMP_XYZ(pm,x,y,z) ((pm)->pmap + (x) + (y) * (pm)->resolution + (z) * (pm)->resolution * (pm)->resolution) 
void ga_photonmap_add(ga_photonmap_t *pm,vec_t pos, vec_t color){
	int x,y,z;
	float fx,fy,fz;
	if(	pos.x <= pm->bound_min.x || 
		pos.y <= pm->bound_min.y ||
		pos.z <= pm->bound_min.z ||
		pos.x >= pm->bound_max.x ||
		pos.y >= pm->bound_max.y ||
		pos.z >= pm->bound_max.z 	){
		return;
	}
	pos = vec_sub(pos,pm->bound_min);
	x = (int)(pos.x*pm->inv_size.x*(pm->resolution-1));
	y = (int)(pos.y*pm->inv_size.y*(pm->resolution-1));
	z = (int)(pos.z*pm->inv_size.z*(pm->resolution-1));

	/*printf("[ %f %f %f ] - [%d %d %d] - ",pos.x,pos.y,pos.z,x,y,z);
	vec_print(color);*/
	
	pos.x -= x * pm->interval.x;
	pos.y -= y * pm->interval.y;
	pos.z -= z * pm->interval.z;
	
	fx = pos.x * pm->inv_interval.x;
	fy = pos.y * pm->inv_interval.y;
	fz = pos.z * pm->inv_interval.z;
	/*printf("pos: %f int: %f, inv_int: %f\n",pos.x pm->interval.x, pm->inv_interval.x);
	printf("[%f %f %f]\n",fx,fy,fz);*/
	
	vec_ffadd(PM_XYZ(pm,x,y,z),	(1.0f-fx)*(1.0f-fy)*(1.0f-fz),	&color);
	vec_ffadd(PM_XYZ(pm,x+1,y,z),	(fx)*(1.0f-fy)*(1.0f-fz),	&color);
	vec_ffadd(PM_XYZ(pm,x,y+1,z),	(1.0f-fx)*(fy)*(1.0f-fz),	&color);
	vec_ffadd(PM_XYZ(pm,x+1,y+1,z),	(fx)*(fy)*(1.0f-fz),		&color);
	vec_ffadd(PM_XYZ(pm,x,y,z+1),	(1.0f-fx)*(1.0f-fy)*(fz),	&color);
	vec_ffadd(PM_XYZ(pm,x+1,y,z+1),	(fx)*(1.0f-fy)*(fz),		&color);
	vec_ffadd(PM_XYZ(pm,x,y+1,z+1),	(1.0f-fx)*(fy)*(fz),		&color);
	vec_ffadd(PM_XYZ(pm,x+1,y+1,z+1),(fx)*(fy)*(fz),		&color);
}

void ga_photonmap_add_photon(ga_photonmap_t *pm,vec_t pos, vec_t color, vec_t normal){
	int x,y,z;
	ga_list_t *list;
	if(	pos.x <= pm->bound_min.x || 
		pos.y <= pm->bound_min.y ||
		pos.z <= pm->bound_min.z ||
		pos.x >= pm->bound_max.x ||
		pos.y >= pm->bound_max.y ||
		pos.z >= pm->bound_max.z 	){
		return;
	}
	pos = vec_sub(pos,pm->bound_min);
	x = (int)(pos.x*pm->inv_size.x*(pm->resolution-1));
	y = (int)(pos.y*pm->inv_size.y*(pm->resolution-1));
	z = (int)(pos.z*pm->inv_size.z*(pm->resolution-1));
	ga_list_add(PMP(pm,x,y,z),ga_photon_new(pos,color,normal));
}
ga_list_t *ga_photonmap_get_list(const ga_photonmap_t *pm, int x, int y, int z){
	if(	x < 0 || x >= pm->resolution ||
		y < 0 || y >= pm->resolution ||
		z < 0 || z >= pm->resolution ){
		return NULL;
	}else{
		return PMP(pm,x,y,z);
	}
}
void  ga_photonmap_get_coord(const ga_photonmap_t *pm, vec_t pos, int *x, int *y, int *z){
	pos = vec_sub(pos,pm->bound_min);
	*x = (int)(pos.x*pm->inv_size.x*(pm->resolution));
	*y = (int)(pos.y*pm->inv_size.y*(pm->resolution));
	*z = (int)(pos.z*pm->inv_size.z*(pm->resolution));
}
	
vec_t ga_photonmap_get(const ga_photonmap_t *pm, vec_t pos){
	int x,y,z;
	float fx,fy,fz;
	vec_t result = vec_new(0,0,0,1);
	if(	pos.x <= pm->bound_min.x || 
		pos.y <= pm->bound_min.y ||
		pos.z <= pm->bound_min.z ||
		pos.x >= pm->bound_max.x ||
		pos.y >= pm->bound_max.y ||
		pos.z >= pm->bound_max.z 	){
		return pm->outside;
	}
	pos = vec_sub(pos,pm->bound_min);
	x = (int)(pos.x*pm->inv_size.x*(pm->resolution-1));
	y = (int)(pos.y*pm->inv_size.y*(pm->resolution-1));
	z = (int)(pos.z*pm->inv_size.z*(pm->resolution-1));
	
	pos.x -= x * pm->interval.x;
	pos.y -= y * pm->interval.y;
	pos.z -= z * pm->interval.z;
	
	fx = pos.x * pm->inv_interval.x;
	fy = pos.y * pm->inv_interval.y;
	fz = pos.z * pm->inv_interval.z;
	
	vec_ffadd(&result, (1.0f-fx)*(1.0f-fy)*(1.0f-fz),PM_XYZ(pm,x,y,z)	);
	vec_ffadd(&result, (fx)*(1.0f-fy)*(1.0f-fz),	PM_XYZ(pm,x+1,y,z)	);
	vec_ffadd(&result, (1.0f-fx)*(fy)*(1.0f-fz),	PM_XYZ(pm,x,y+1,z)	);
	vec_ffadd(&result, (fx)*(fy)*(1.0f-fz),		PM_XYZ(pm,x+1,y+1,z)	);
	vec_ffadd(&result, (1.0f-fx)*(1.0f-fy)*(fz),	PM_XYZ(pm,x,y,z+1)	);
	vec_ffadd(&result, (fx)*(1.0f-fy)*(fz),		PM_XYZ(pm,x+1,y,z+1)	);
	vec_ffadd(&result, (1.0f-fx)*(fy)*(fz),		PM_XYZ(pm,x,y+1,z+1)	);
	vec_ffadd(&result, (fx)*(fy)*(fz),		PM_XYZ(pm,x+1,y+1,z+1)	);
	return result;
}
