#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
static int ga_kdn_is_leaf(const ga_kdn_t *kdn){
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
int ga_kdn_trace(const ga_kdn_t *root, vec_t min, vec_t max, vec_t origin, vec_t dir, tri_t **tri, float *u, float *v, float *t){
	float a1,b1,a2,b2,_u,_v,_t;
	int hit1,hit2;
	vec_t lmax = max;
	vec_t rmin = min;
	int hit = 0;
	ga_node_t *n = NULL;
	if(!ga_ray_box_intersect(&origin,&dir,&min,&max,&a1,&b1)){
		return 0;
	}else if(ga_kdn_is_leaf(root)){
		n = root->data->first;
		while(n){
			if(ga_ray_tri_intersect((tri_t*)n->data,&origin,&dir,
						&_t,&_u,&_v) 
					&& (_t >= 0.0f) && (_t >= a1) && (_t <= b1)){
				if(!hit || (hit && _t < *t)){
					hit = 1;
					*t = _t;
					*u = _u;
					*v = _v;
					*tri = (tri_t*)n->data;
				}
			}
			n = n->next;
		}
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
				if(!ga_kdn_trace(root->left,min,lmax,origin,dir,tri,u,v,t)){
					if(!ga_kdn_trace(root->right,rmin,max,origin,dir,tri,u,v,t)){
						return 0;
					}else{
						return 1;
					}
				}else{
					return 1;
				}
			}else{
				if(!ga_kdn_trace(root->right,rmin,max,origin,dir,tri,u,v,t)){
					if(!ga_kdn_trace(root->left,min,lmax,origin,dir,tri,u,v,t)){
						return 0;
					}else{
						return 1;
					}
				}else{
					return 1;
				}
			}
		}else if(hit1){
			return ga_kdn_trace(root->left,min,lmax,origin,dir,tri,u,v,t);
		}else if(hit2){
			return ga_kdn_trace(root->right,rmin,max,origin,dir,tri,u,v,t);
		}
		fprintf(stderr,"WARNING : impossible case in ray tracing\n");
		return 0;
	}
}
typedef struct stack_elem_s{
	ga_kdn_t *node;	/*pointer to far child*/
	float t;	/*entry/exit signed distance*/
	vec_t pb;	/*entry/exit point*/
	int   prev;	/*pointer to previous stack element*/
}stack_elem_t;
/*
#define KDN_LEFT(kdn) (kdn)->left
#define KDN_RIGHT(kdn) (kdn)->right
#define KDN_IS_LEAF(kdn) ga_kdn_is_leaf(kdn)
#define KDN_AXIS(kdn)	(kdn)->axis
#define KDN_SPLIT(kdn)  (kdn)->split
#define KDN_LIST(kdn)	(kdn)->data
#define NAXIS(axis) ((axis)+1)%3
#define PAXIS(axis) ((axis)+2)%3
inline static void stack_set_pb_axis(stack_elem_t *se, int axis, float val){
	float *vec = (float*)(&(se->pb));
	vec[axis] = val;
}
inline static float stack_get_pb_axis(stack_elem_t *se, int axis){
	return ((float*)(&(se->pb)))[axis];
}
int ga_kdn_trace_fast(	ga_kdn_t *root,
			const vec_t *box_min,
			const vec_t *box_max,
			const vec_t *origin,
			const vec_t *dir,
			tri_t **tri, float *u, float *v, float *dist){
	float a,b;
	float t;

	if(!ga_ray_box_intersect(origin,dir,box_min,box_max,&a,&b)){
		return 0;
	}

	stack_elem_t stack[GA_MAX_REC_DEPTH];
	memset(&stack,0,GA_MAX_REC_DEPTH*sizeof(stack_elem_t));

	ga_kdn_t *far_child, *curr_node;
	curr_node = root;

	int enpt = 0;
	stack[enpt].t = a;
	if(a >= 0.0){
		stack[enpt].pb = vec_add(*origin,vec_scale(a,*dir));
	}else{
		stack[enpt].pb = *origin;
	}
	int expt = 1;
	stack[expt].t = b;
	stack[expt].pb = vec_add(*origin,vec_scale(b,*dir));
	stack[expt].node = NULL;

	ga_kdn_t *test = NULL;
	while(curr_node != NULL){
		int i = 0;
		while(!ga_kdn_is_leaf(curr_node)){
			float split = KDN_SPLIT(curr_node);
			int   axis  = KDN_AXIS(curr_node);
			i++;
			if(stack_get_pb_axis(stack + enpt, axis) <= split){
				if(stack_get_pb_axis(stack + expt, axis) <= split){
					curr_node = KDN_LEFT(curr_node);
					continue;
				}
				if(stack_get_pb_axis(stack + expt, axis) == split){
					curr_node = KDN_RIGHT(curr_node);
				}
				curr_node = KDN_LEFT(curr_node);
				far_child = KDN_RIGHT(curr_node);
			}else{
				if(stack_get_pb_axis(stack + expt, axis) > split){
					curr_node = KDN_RIGHT(curr_node);
					continue;
				}
				far_child = KDN_LEFT(curr_node);
				curr_node = KDN_RIGHT(curr_node);
			}
			t = (split - vec_idx(*origin,axis)) / vec_idx(*dir,axis);
			int tmp = expt;
			if(++expt == enpt){ expt++; }
			stack[expt].prev = tmp;
			stack[expt].t = t;
			stack[expt].node = far_child;
			stack_set_pb_axis(stack + expt,axis,split);
			int naxis = NAXIS(axis);
			int paxis = PAXIS(axis);
			stack_set_pb_axis(stack + expt, naxis,
				vec_idx(*origin,naxis) + t*vec_idx(*dir,naxis));
			stack_set_pb_axis(stack + expt, paxis,
				vec_idx(*origin,paxis) + t*vec_idx(*dir,paxis));
		}
		ga_node_t *n = (KDN_LIST(curr_node))->first;
		float _dist,_u,_v;
		int hit = 0;
		while(n){
			if(ga_ray_tri_intersect((tri_t*)n->data,origin,dir,
						&_dist,&_u,&_v) 
					&& _dist >= 0.0f
					&& _dist > stack[enpt].t
					&& _dist < stack[expt].t ){
				if(!hit || (hit && _dist < *dist)){
					hit = 1;
					*dist = _dist;
					*u = _u;
					*v = _v;
					*tri = (tri_t*)n->data;
				}
			}
			n = n->next;
		}
		if(hit){ return 1; }
		enpt = expt;
		curr_node = stack[expt].node;
		expt = stack[enpt].prev;
	}
	return 0;
}*/
	
		


	

