#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "ga_kdtree.h"
#include "ga_list.h"

static ga_kdn_t *ga_kdn_new(){
	ga_kdn_t *kdn = (ga_kdn_t*)malloc(sizeof(ga_kdn_t));
	memset(kdn,0,sizeof(ga_kdn_t*));
	return kdn;
}
static ga_kdn_t *ga_kdn_new_leaf(ga_list_t*l){
	ga_kdn_t *kdn = ga_kdn_new();
	kdn->flags = GA_KDN_LEAF;
	kdn->p0 = l;
	return kdn;
}
static void ga_kdn_free(ga_kdn_t*kdn){
	free(kdn);
}
ga_kdn_t *ga_kdtree_build_mean(ga_list_t*tri_list, int max_tri, int max_depth, int axis){
	float bound_low, bound_up;
	ga_node_t *n  	= tri_list->first;
	tri_t *tri 	= (tri_t*)n->data;
	ga_list_t *up 	= ga_list_new();
	ga_list_t *low 	= ga_list_new();
	ga_kdn_t  *kdn 	= ga_kdn_new();
	float inv_size = 1.0f/tri_list->size;
	kdn->axis  = (char)axis;
	kdn->flags = GA_KDN_NODE;

	/*finding limit*/
	kdn->split = 0.0f;
	while(n){
		tri = (tri_t*)n->data;
		ga_tri_bound(tri,axis,&bound_low,&bound_up);
		kdn->split += bound_low * inv_size;
		n = n->next; 
	}
	n = tri_list->first;

	while(n){	/*sorting triangles into list*/
		tri = (tri_t*)n->data;
		ga_tri_bound(tri,axis,&bound_low,&bound_up);
		if(bound_low <= kdn->split){
			ga_list_add(low,tri);
		}
		if(bound_up >= kdn->split){
			ga_list_add(up,tri);
		}
		n = n->next;
	}
	/* first recursive call*/
	if(ga_list_size(low) <= max_tri || max_depth <= 0){
		kdn->p0 = ga_kdn_new_leaf(low);
	}else{
		kdn->p0 = ga_kdtree_build_mean(low,max_tri,max_depth -1,(axis+1)%GA_AXIS_COUNT);
		ga_list_free(low);
	}
	/* second recursive call */
	if(ga_list_size(up) <= max_tri || max_depth <= 0){
		kdn->p1 = ga_kdn_new_leaf(up);
	}else{
		kdn->p1 = ga_kdtree_build_mean(up,max_tri,max_depth -1,(axis+1)%GA_AXIS_COUNT);
		ga_list_free(up);
	}
	return kdn;
}
ga_kdn_t *ga_kdtree_build_octree(ga_list_t *tri_list, int max_tri, int max_depth, int axis, vec_t min, vec_t max){
	ga_kdn_t *kdn = ga_kdn_new();
	ga_list_t *l0 = ga_list_new();
	ga_list_t *l1 = ga_list_new();
	ga_node_t *n  = NULL;
	vec_t tmp;
	float low,up;
	kdn->split = (vec_idx(min,axis) + vec_idx(max,axis))/2.0f;
	kdn->flags  = GA_KDN_NODE;
	kdn->axis   = axis;

	n = tri_list->first;
	while(n){
		ga_tri_bound((tri_t*)n->data,axis,&low,&up);
		if(low < kdn->split){
			ga_list_add(l0,n->data);
		}
		if(up > kdn->split){
			ga_list_add(l1,n->data);
		}
		n = n->next;
	}
	if(max_depth <= 0){
		kdn->p0 = ga_kdn_new_leaf(l0);
		kdn->p1 = ga_kdn_new_leaf(l1);
	}else{
		if(l0->size <= max_tri){
			kdn->p0 = ga_kdn_new_leaf(l0);
		}else{
			tmp = max;
			((float*)(&tmp))[axis] = kdn->split;
			kdn->p0 = ga_kdtree_build_octree(l0,max_tri,max_depth-1, (axis+1)%3,
					min, tmp );
			ga_list_free(l0);
		}
		if(l1->size <= max_tri){
			kdn->p1 = ga_kdn_new_leaf(l1);
		}else{
			tmp = min;
			((float*)(&tmp))[axis] = kdn->split;
			kdn->p1 = ga_kdtree_build_octree(l1,max_tri,max_depth-1,(axis+1)%3,
					tmp,max);
			ga_list_free(l1);
		}
	}
	return kdn;
}
inline static void pl(int level){
	while(level--){	printf(" "); }
}
void ga_kdtree_print(ga_kdn_t *kdn,int level){
	if(!kdn){  printf("NULL\n"); return; }
	
	if(kdn->flags == GA_KDN_LEAF){
		pl(level);
		printf("leaf: %d\n",ga_list_size( (ga_list_t*)kdn->p0 ) );
	}else{
		pl(level);
		switch(kdn->axis){
			case 0:printf("X");break;
			case 1:printf("Y");break;
			case 2:printf("Z");break;
		}
		printf(" : %f\n",kdn->split);
		ga_kdtree_print((ga_kdn_t*)kdn->p0,level+1);
		ga_kdtree_print((ga_kdn_t*)kdn->p1,level+1);
	}
}	
inline static ga_node_t *kdn_list_node(ga_kdn_t*kdn){
	return ((ga_list_t*)(kdn->p0))->first;
}
typedef struct stack_elem_s{
	ga_kdn_t *node;	/*pointer to far child*/
	float t;	/*entry/exit signed distance*/
	vec_t pb;	/*entry/exit point*/
	int   prev;	/*pointer to previous stack element*/
}stack_elem_t;
#define KDN_LEFT(kdn) (ga_kdn_t*)((kdn)->p0)
#define KDN_RIGHT(kdn) (ga_kdn_t*)((kdn)->p1)
#define KDN_IS_LEAF(kdn) ((kdn)->flags == GA_KDN_LEAF)
#define KDN_AXIS(kdn)	(int)((kdn)->axis)
#define KDN_SPLIT(kdn)  (kdn)->split
#define NAXIS(axis) ((axis)+1)%3
#define PAXIS(axis) ((axis)+2)%3
inline static void stack_set_pb_axis(stack_elem_t *se, int axis, float val){
	float *vec = (float*)(&(se->pb));
	vec[axis] = val;
}
inline static float stack_get_pb_axis(stack_elem_t *se, int axis){
	return ((float*)(&(se->pb)))[axis];
}
int ga_kdtree_ray_trace(	ga_kdn_t *root,
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

	while(curr_node != NULL){
		while(!KDN_IS_LEAF(curr_node)){
			float split = KDN_SPLIT(curr_node);
			int   axis  = KDN_AXIS(curr_node);
			if(stack_get_pb_axis(stack + enpt, axis) <= split){
				if(stack_get_pb_axis(stack + expt, axis) <=split){
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
		ga_node_t *n = kdn_list_node(curr_node);
		float _dist,_u,_v;
		int hit = 0;
		while(n){
			if(ga_ray_tri_intersect((tri_t*)n->data,origin,dir,
						&_dist,&_u,&_v) 
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
}




	






