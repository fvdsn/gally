#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include"ga_photonmap.h"


ga_photon_t *ga_photon_new(const vec_t *pos, const vec_t *color, const vec_t *normal, const vec_t *ldir){
	ga_photon_t * p= (ga_photon_t*)malloc(sizeof(ga_photon_t));
	p->pos = *pos;
	p->color = *color;
	p->normal = *normal;
	p->ldir = *ldir;
	return p;
}

ga_photonmap_t * ga_photonmap_new(vec_t bound_min,vec_t bound_max,float resolution){
	ga_photonmap_t *pm = (ga_photonmap_t*)malloc(sizeof(ga_photonmap_t));
	pm->bound_min 	= bound_min;
	pm->bound_max 	= bound_max;
	pm->size 	= vec_sub(bound_max,bound_min);
	pm->resx 	= (int)(pm->size.x / resolution) + 1; 
	pm->resy 	= (int)(pm->size.y / resolution) + 1; 
	pm->resz 	= (int)(pm->size.z / resolution) + 1; 
	pm->interval.x  = pm->size.x / (float) pm->resx; 
	pm->interval.y  = pm->size.y / (float) pm->resy; 
	pm->interval.z  = pm->size.z / (float) pm->resz; 
	pm->inv_size.x 	= 1.0f/pm->size.x;
	pm->inv_size.y 	= 1.0f/pm->size.y;
	pm->inv_size.z 	= 1.0f/pm->size.z;
	pm->map = (ga_list_t*)malloc(pm->resx*pm->resy*pm->resz*sizeof(ga_list_t));
	memset(pm->map,0,pm->resx*pm->resy*pm->resz*sizeof(ga_list_t));
	return pm;
}

#define PM_XYZ(pm,x,y,z)  ((pm)->map  + (x) + (y) * (pm)->resx + (z) * (pm)->resx * (pm)->resy) 
static void ga_photonmap_add_cell(ga_photonmap_t *pm, ga_photon_t *p, int x, int y, int z){
	if( 	x >= 0 && x < pm->resx &&
		y >= 0 && y < pm->resy && 
		z >= 0 && z < pm->resz ){
		ga_list_add(PM_XYZ(pm,x,y,z),p);
	}
}	
void ga_photonmap_add(ga_photonmap_t *pm, const vec_t *pos, const vec_t *color, const vec_t *normal, const vec_t *ldir){
	int x,y,z,j,k,l;
	ga_photon_t * p;
	if(	pos->x <= pm->bound_min.x || 
		pos->y <= pm->bound_min.y ||
		pos->z <= pm->bound_min.z ||
		pos->x >= pm->bound_max.x ||
		pos->y >= pm->bound_max.y ||
		pos->z >= pm->bound_max.z 	){
		return;
	}
	x = (int)((pos->x - pm->bound_min.x)*pm->inv_size.x*pm->resx);
	y = (int)((pos->y - pm->bound_min.y)*pm->inv_size.y*pm->resy);
	z = (int)((pos->z - pm->bound_min.z)*pm->inv_size.z*pm->resz);
	p = ga_photon_new(pos,color,normal,ldir);
	j = 1;
	while(j >= -1){
		k = 1;
		while(k >= -1){
			l = 1;
			while(l >= -1){
				ga_photonmap_add_cell(pm,p,x+j,y+k,z+l);
				l--;
			}
			k--;
		}
		j--;
	}
}

ga_list_t * ga_photonmap_get(const ga_photonmap_t *pm, const vec_t *pos){
	int x,y,z;
	if(	pos->x <= pm->bound_min.x || 
		pos->y <= pm->bound_min.y ||
		pos->z <= pm->bound_min.z ||
		pos->x >= pm->bound_max.x ||
		pos->y >= pm->bound_max.y ||
		pos->z >= pm->bound_max.z 	){
		return NULL;
	}
	x = (int)((pos->x - pm->bound_min.x)*pm->inv_size.x*pm->resx);
	y = (int)((pos->y - pm->bound_min.y)*pm->inv_size.y*pm->resy);
	z = (int)((pos->z - pm->bound_min.z)*pm->inv_size.z*pm->resz);
	return PM_XYZ(pm,x,y,z);
}
