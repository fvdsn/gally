#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "ga_scene.h"

ga_cam_t *ga_cam_new(char *name, vec_t pos, vec_t dir, vec_t up, float fov){
	ga_cam_t*c = (ga_cam_t*)malloc(sizeof(ga_cam_t));
	strncpy(c->name,name,STRING_LENGTH);
	c->pos = pos;
	c->dir = dir;
	c->up  = up;
	c->fov = fov;
	return c;
}
void  ga_cam_print(ga_cam_t*c){
	printf("<Camera position='%f %f %f' direction='%f %f %f' up='%f %f %f' fovy='%f' name='%s' />\n",
			c->pos.x, c->pos.y, c->pos.z,
			c->dir.x, c->dir.y, c->dir.z,
			c->up.x,  c->up.y,  c->up.z,
			c->fov, c->name);
}
ga_light_t *ga_light_new(char *name, vec_t pos, vec_t color){
	ga_light_t*l = (ga_light_t*)malloc(sizeof(ga_light_t));
	strncpy(l->name,name,STRING_LENGTH);
	l->pos   = pos;
	l->color = color;
	return l;
}
void	ga_light_print(ga_light_t*l){
	printf("<PointLight position='%f %f %f' color='%f %f %f' intensity='%f' name='%s' />\n",
			l->pos.x,   l->pos.y,   l->pos.z,
			l->color.x, l->color.y, l->color.z,
			l->color.w, l->name);
}

static ga_material_t *ga_material_new(char *name){
	ga_material_t*m = (ga_material_t*)malloc(sizeof(ga_material_t));
	memset(m,0,sizeof(ga_material_t));
	strncpy(m->name,name,STRING_LENGTH);
	return m;
}
ga_material_t *ga_material_new_diffuse(char *name, vec_t color){
	ga_material_t*m = ga_material_new(name);
	m->type = GA_MATERIAL_DIFFUSE;
	m->color = color;
	return m;
}
ga_material_t *ga_material_new_phong(char *name, vec_t color, float power){
	ga_material_t*m = ga_material_new(name);
	m->type = GA_MATERIAL_PHONG;
	m->color = color;
	m->power = power;
	return m;
}
ga_material_t *ga_material_new_blending(char *name, float alpha, ga_material_t *c){
	ga_material_t*m = ga_material_new(name);
	m->type = GA_MATERIAL_BLENDING;
	m->alpha = alpha;
	m->child = c;
	return m;
}	
ga_material_t *ga_material_new_comb(char *name){
	ga_material_t *m = ga_material_new(name);
	m->type = GA_MATERIAL_COMB;
	m->comb = ga_list_new();
	return m;
}
ga_material_t *ga_material_new_flat(char *name,vec_t color){
	ga_material_t *m = ga_material_new(name);
	m->type = GA_MATERIAL_FLAT;
	m->color = color;
	return m;
}
ga_material_t *ga_material_new_emit(char *name,vec_t color){
	ga_material_t *m = ga_material_new(name);
	m->type = GA_MATERIAL_EMIT;
	m->color = color;
	return m;
}
void	ga_material_add_comb(ga_material_t *mat, ga_material_t *comb){
	ga_list_add(mat->comb,comb);
}

void	ga_material_print(ga_material_t *m){
	switch(m->type){
		case GA_MATERIAL_DIFFUSE:
			printf("<DiffuseMaterial color='%f %f %f' name='%s' />\n",
					m->color.x, m->color.y, m->color.z,
					m->name);
			break;
		default:
			printf("<UnknownKindMaterial />\n");
	}
}
ga_geom_t *ga_geom_new_model(char *name, model_t *m){
	ga_geom_t*g = (ga_geom_t*)malloc(sizeof(ga_geom_t));
	strncpy(g->name,name,STRING_LENGTH);
	g->model = m;
	return g;
}
void	ga_geom_print(ga_geom_t*g){
	printf("<Obj file='TODO' name='%s' />\n",g->name);
}
ga_shape_t *ga_shape_new(char*name, ga_geom_t*g, ga_material_t *m){
	ga_shape_t*s = (ga_shape_t*)malloc(sizeof(ga_shape_t));
	strncpy(s->name,name,STRING_LENGTH);
	s->geom = g;
	s->material = m;
	return s;
}
void ga_shape_print(ga_shape_t*s){
	printf("<Shape geometry='%s' material='%s' />\n",
			s->geom->name,
			s->material->name);
}
ga_transform_t *ga_transform_identity(void){
	ga_transform_t *t = (ga_transform_t*)malloc(sizeof(ga_transform_t));
	strncpy(t->name,"identity",12);
	t->type = GA_IDENTITY;
	t->child = ga_list_new();
	t->shape = ga_list_new();
	t->matrix = mat_set_id(mat_new_zero());
	return t;
}
ga_transform_t *ga_transform_translate(vec_t param){
	ga_transform_t *t = ga_transform_identity();
	strncpy(t->name,"translation",12);
	t->type = GA_TRANSLATE;
	t->param = param;
	t->matrix = mat_set_trans(param,mat_new_zero());
	return t;
}
ga_transform_t *ga_transform_scale(vec_t param){
	ga_transform_t *t = ga_transform_identity();
	strncpy(t->name,"scaling",8);
	t->type = GA_SCALE;
	t->param = param;
	t->matrix = mat_set_scale(param,mat_new_zero());
	return t;
}
ga_transform_t *ga_transform_rotate(vec_t param, float angle){
	ga_transform_t *t = ga_transform_identity();
	strncpy(t->name,"rotation",8);
	t->type = GA_ROTATE;
	t->angle = angle;
	t->param = param;
	t->matrix = mat_set_rot(param,angle,mat_new_zero());
	return t;
}
void ga_transform_add_child(ga_transform_t *p, ga_transform_t *c){
	ga_list_add(p->child,c);
}
void ga_transform_add_shape(ga_transform_t *p, ga_shape_t *c){
	ga_list_add(p->shape,c);
}
void ga_transform_print(ga_transform_t *t){
	ga_node_t *n;
	switch(t->type){
		case GA_TRANSLATE:
			printf("<Translate vector='%f %f %f'>\n",
					t->param.x,t->param.y,t->param.z);
			break;
		case GA_ROTATE:
			printf("<Rotate axis='%f %f %f' angle='%f'>\n",
					t->param.x,t->param.y,t->param.z,t->angle);
			break;
		case GA_SCALE:
			printf("<Scale scale='%f %f %f'>\n",
					t->param.x,t->param.y,t->param.z);
			break;
		default:break;
	}
	n = t->shape->first;
	while(n){
		ga_shape_print((ga_shape_t*)n->data);
		n = n->next;
	}
	n = t->child->first;
	while(n){
		ga_transform_print((ga_transform_t*)n->data);
		n = n->next;
	}
	switch(t->type){
		case GA_TRANSLATE:printf("</Translate>\n");break;
		case GA_ROTATE:	printf("</Rotate>\n");break;
		case GA_SCALE:	printf("</Rotate>\n");break;
		default:break;
	}
}


ga_scene_t *ga_scene_new(char *name){
	ga_scene_t *s = (ga_scene_t*)malloc(sizeof(ga_scene_t));
	strncpy(s->name,name,STRING_LENGTH);
	s->bg_color = vec_new(0,0,0,1);
	s->active_camera = NULL;
	s->camera = ga_list_new();
	s->light = ga_list_new();
	s->material = ga_list_new();
	s->shape = ga_list_new();
	s->geom = ga_list_new();
	s->transform = ga_transform_identity();
	s->tri_pool = ga_list_new();
	return s;
}
void ga_scene_set_image(ga_scene_t *s, int sizex, int sizey){
	s->img = ga_image_new(sizex,sizey);
	ga_image_fill(s->img,vec_new(0,0,0,1));
}
void ga_scene_save_image(ga_scene_t *s,char *path){
	ga_image_save(s->img,path);
}
void	ga_scene_add_camera(ga_scene_t *s, ga_cam_t *c){
	ga_list_add(s->camera,c);
}
void	ga_scene_add_light(ga_scene_t *s, ga_light_t *l){
	ga_list_add(s->light,l);
}
void	ga_scene_add_material(ga_scene_t *s, ga_material_t *m){
	ga_list_add(s->material,m);
}
void	ga_scene_add_geom(ga_scene_t *s, ga_geom_t *g){
	ga_list_add(s->geom,g);
}
ga_cam_t 	*ga_scene_get_camera(ga_scene_t *s, const char *name){
	return (ga_cam_t*)ga_list_find(s->camera,name);
}
ga_light_t	*ga_scene_get_light(ga_scene_t *s, const char *name){
	return (ga_light_t*)ga_list_find(s->light,name);
}
ga_material_t	*ga_scene_get_material(ga_scene_t *s, const char *name){
	return (ga_material_t*)ga_list_find(s->material,name);
}
ga_geom_t	*ga_scene_get_geom(ga_scene_t *s, const char *name){
	return (ga_geom_t*)ga_list_find(s->geom,name);
}
void	ga_scene_print(ga_scene_t *s){
	ga_node_t *n = NULL;
	printf("<?xml version='1.0' encoding='utf-8'?>\n");
	printf("<Sdl>\n");
	printf("  <Cameras>\n");
	n = s->camera->first;
	while(n){
		ga_cam_print((ga_cam_t*)n->data);
		n = n->next;
	}
	printf("  </Cameras>\n\n");
	printf("  <Lights>\n");
	n = s->light->first;
	while(n){
		ga_light_print((ga_light_t*)n->data);
		n = n->next;
	}
	printf("  </Lights>\n\n");
	printf("  <Geometry>\n");
	n = s->geom->first;
	while(n){
		ga_geom_print((ga_geom_t*)n->data);
		n = n->next;
	}
	printf("  </Geometry>\n\n");
	printf("  <Materials>\n");
	n = s->material->first;
	while(n){
		ga_material_print((ga_material_t*)n->data);
		n = n->next;
	}
	printf("  </Materials>\n\n");
	printf("  <Scene camera='%s' lights='all' background='%f %f %f'>\n",
			s->active_camera->name,
			s->bg_color.x, s->bg_color.y, s->bg_color.z	);
	ga_transform_print(s->transform);
	printf("  </Scene>\n");
	printf("</Sdl>\n\n");
}
static void ga_model_build(	ga_scene_t *s, 
				model_t *model, 
				ga_material_t *material,
				mat_t * trmat ){
	int i = 0;
	mat_t n;
	tri_t *tri = (tri_t*)malloc(sizeof(tri_t)*model->tri_count);/*TODO free this */
	memcpy(tri,model->tri,model->tri_count*sizeof(tri_t));
	mat_set_norm(&n,trmat);
	while( i < model->tri_count){
		tri[i].material = material;
		tri_transform(tri + i,trmat,&n);
		ga_list_add(s->tri_pool,tri + i);
		i++;
	}
}
static void ga_graph_build(	ga_scene_t *s, 
				const ga_transform_t *tr, 
				const mat_t *trmat ){
	mat_t mat;
	ga_node_t *n = NULL;
	ga_shape_t *shape = NULL;
	ga_geom_t *geom = NULL;
	mat_cpy(&mat,tr->matrix);
	mat_mult2(trmat,&mat);

	n = tr->shape->first;
	while(n){
		shape = (ga_shape_t*)n->data;
		geom  = shape->geom;
		ga_model_build(s,geom->model,shape->material,&mat);
		n = n->next;
	}
	
	n = tr->child->first;
	while(n){
		ga_graph_build(s,(ga_transform_t*)n->data,&mat);
		n = n->next;
	}
}
void ga_scene_build_triangle(ga_scene_t *s){
	mat_t trmat;
	mat_set_id(&trmat);
	ga_graph_build(s,s->transform,&trmat);
}
void ga_scene_build_bounding_box(ga_scene_t *s){
	const ga_node_t *n = s->tri_pool->first;
	vec_t min;
	vec_t max;
	int axis = 3;
	while(axis--){
		((float*)&(s->box_min))[axis] = FLT_MAX;
		((float*)&(s->box_max))[axis] = -FLT_MAX;
	}
	while(n){
		axis = 3;
		while(axis--){
			ga_tri_bound((tri_t*)n->data,axis,
					(float*)(&min) + axis,
					(float*)(&max) + axis	);
		}
		vec_fmin(&(s->box_min),&min);
		vec_fmax(&(s->box_max),&max);
		n = n->next;
	}
}
void ga_scene_build_kdtree(ga_scene_t *s){
	s->kdtree = ga_kdtree_build_octree(s->tri_pool,10,20,0,s->box_min,s->box_max);
}

