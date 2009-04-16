#include <stdlib.h>
#include <stdio.h>
#include "ga_kdt.h"

static ga_kdn_t *ga_kdn_new_node(float split, int axis){
	ga_kdn_t *kdn = (ga_kdn_t*)malloc(sizeof(ga_kdn_t));
	kdn->split = split;
	kdn->axis = axis;
	kdn->left = NULL;
	kdn->right = NULL;
	kdn->data = NULL;
	return kdn;
}
static ga_kdn_t *ga_kdn_new_leaf(ga_list_t *tri_list){
	ga_kdn_t *kdn = (ga_kdn_t*)malloc(sizeof(ga_kdn_t));
	kdn->split = 0.0f;
	kdn->axis = 0;
	kdn->left = NULL;
	kdn->right = NULL;
	kdn->data = tri_list;
	return kdn;
}
static int ga_kdn_is_leaf(ga_kdn_t *kdn){
	if(kdn->data && !kdn->left && !kdn->right){
		return 1;
	}else if(!kdn->data && kdn->left && kdn->right){
		return 0;
	}else{
		fprintf(stderr,"WARNING: kdn neither leaf or node \n");
		return 0;
	}
}
ga_kdn_t *ga_kdn_octree(ga_list_t *tri_list, int max_depth, int min_tri, int axis,
		vec_t bmin, vec_t bmax){
	ga_list_t *left = ga_list_new();
	ga_list_t *right = ga_list_new();
	ga_node_t *n = tri_list->first;
	ga_kdn_t  *kdn = NULL;
	float split = 0.0f;
	float min,max;
	vec_t lmax = bmax;
	vec_t rmin = bmin;
	switch(axis){
		case GA_AXIS_X: split = (bmin.x+bmax.x)/2.0f;
				lmax.x = split;
				rmin.x = split;
				break;
		case GA_AXIS_Y: split = (bmin.y+bmax.y)/2.0f;
				lmax.y = split;
				rmin.y = split;
				break;
		case GA_AXIS_Z: split = (bmin.z+bmax.z)/2.0f;
				lmax.z = split;
				rmin.z = split;
				break;
		default: fprintf(stderr,"ERROR: build octree : wrong axis\n");
	}
	while(n){
		ga_tri_bound((tri_t*)n->data,axis,&min,&max);
		if(min < split){
			ga_list_add(left,n->data);
		}
		if(max > split){
			ga_list_add(right,n->data);
		}
		n = n->next;
	}
	kdn = ga_kdn_new_node(split,axis);
	if(left->size <= min_tri || max_depth <=0){
		kdn->left = ga_kdn_new_leaf(left);
	}else{
		kdn->left = ga_kdn_octree(left,max_depth-1,min_tri,(axis+1)%GA_AXIS_COUNT, bmin,lmax);
		ga_list_free(left);
	}
	if(right->size <= min_tri || max_depth <=0){
		kdn->right = ga_kdn_new_leaf(right);
	}else{
		kdn->right = ga_kdn_octree(right,max_depth-1,min_tri,(axis+1)%GA_AXIS_COUNT,rmin,bmax);
		ga_list_free(right);
	}
	return kdn;
}
int ga_kdn_trace(ga_kdn_t *root, vec_t min, vec_t max, vec_t origin, vec_t dir, tri_t **tri, float *u, float *v, float *dist){
	float a1,b1,a2,b2;
	int hit1,hit2;
	vec_t lmax = max;
	vec_t rmin = min;
	int hit = 0;
	if(!ga_ray_box_intersect(&origin,&dir,&min,&max,&a1,&b1)){
		return 0;
	}else if(ga_kdn_is_leaf(root)){
		/*test triangles*/
		return hit;
	}else{
		switch(root->axis){
			case GA_AXIS_X:lmax.x = root->split;
				       rmin.x = root->split;
				       break;
			case GA_AXIS_Y:lmax.y = root->split;
				       rmin.y = root->split;
				       break;
			case GA_AXIS_Z:lmax.z = root->split;
				       rmin.z = root->split;
				       break;
			default:fprintf(stderr,"ERROR: wrong axis in kdtree\n");
		}
		hit1 = ga_ray_box_intersect(&origin,&dir,&min,&lmax,&a1,&b1);
		hit2 = ga_ray_box_intersect(&origin,&dir,&rmin,&max,&a2,&b2);
		if(hit1 && hit2){
			if(a1 < a2){
				if(!ga_kdn_trace(root,min,lmax,origin,dir,tri,u,v,dist)){
					if(!ga_kdn_trace(root,rmin,max,origin,dir,tri,u,v,dist)){
						return 0;
					}else{
						return 1;
					}
				}else{
					return 1;
				}
			}else{
				if(!ga_kdn_trace(root,rmin,max,origin,dir,tri,u,v,dist)){
					if(!ga_kdn_trace(root,min,lmax,origin,dir,tri,u,v,dist)){
						return 0;
					}else{
						return 1;
					}
				}else{
					return 1;
				}
			}
		}else if(hit1){
			return ga_kdn_trace(root,min,lmax,origin,dir,tri,u,v,dist);
		}else if(hit2){
			return ga_kdn_trace(root,min,lmax,origin,dir,tri,u,v,dist);
		}
		fprintf(stderr,"WARNING : impossible case in ray tracing\n");
		return 0;
	}
}
	
		


	

