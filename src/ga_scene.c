#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


ga_material_t *ga_material_new_diffuse(char *name, vec_t color){
	ga_material_t*m = (ga_material_t*)malloc(sizeof(ga_material_t));
	strncpy(m->name,name,STRING_LENGTH);
	m->type = GA_MATERIAL_DIFFUSE;
	m->color = color;
	return m;
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
ga_geom_t *ga_geom_new_sphere(char *name, float radius){
	ga_geom_t*g = (ga_geom_t*)malloc(sizeof(ga_geom_t));
	strncpy(g->name,name,STRING_LENGTH);
	g->type = GA_GEOM_SPHERE;
	g->radius = radius;
	return g;
}
ga_geom_t *ga_geom_new_model(char *name, model_t *m){
	ga_geom_t*g = (ga_geom_t*)malloc(sizeof(ga_geom_t));
	strncpy(g->name,name,STRING_LENGTH);
	g->type = GA_GEOM_MODEL;
	g->model = m;
	return g;
}
void	ga_geom_print(ga_geom_t*g){
	switch(g->type){
		case GA_GEOM_SPHERE:
			printf("<Sphere radius='%f' name='%s' />\n",
					g->radius, g->name);
			break;
		default:
			printf("<UnknownGeometry />\n");
	}
}
ga_shape_t *ga_shape_new(char*name, ga_geom_t*g, ga_material_t *m){
	ga_shape_t*s = (ga_shape_t*)malloc(sizeof(ga_shape_t));
	strncpy(s->name,name,STRING_LENGTH);
	s->geom = g;
	s->material = m;
	return s;
}
ga_transform_t *ga_transform_identity(void){
	ga_transform_t *t = (ga_transform_t*)malloc(sizeof(ga_transform_t));
	strncpy(t->name,"identity",12);
	t->type = GA_IDENTITY;
	t->child = ga_list_new();
	t->shape = ga_list_new();
	return t;
}
ga_transform_t *ga_transform_translate(vec_t param){
	ga_transform_t *t = ga_transform_identity();
	strncpy(t->name,"translation",12);
	t->type = GA_TRANSLATE;
	t->param = param;
	t->matrix = mat_new_zero();
	mat_set_trans(param,t->matrix);
	return t;
}
ga_transform_t *ga_transform_scale(vec_t param){
	ga_transform_t *t = ga_transform_identity();
	strncpy(t->name,"scaling",8);
	t->type = GA_SCALE;
	t->param = param;
	t->matrix = mat_new_zero();
	mat_set_scale(param,t->matrix);
	return t;
}
ga_transform_t *ga_transform_rotate(vec_t param, float angle){
	ga_transform_t *t = ga_transform_identity();
	strncpy(t->name,"rotation",8);
	t->type = GA_ROTATE;
	t->param = param;
	t->matrix = mat_new_zero();
	mat_set_rot(0,0,t->matrix);/*TODO*/
	return t;
}
void ga_transform_add_child(ga_transform_t *p, ga_transform_t *c){
	ga_list_add(p->child,c);
}
void ga_transform_add_shape(ga_transform_t *p, ga_shape_t *c){
	ga_list_add(p->shape,c);
}
ga_scene_t *ga_scene_new(char *name){
	ga_scene_t *s = (ga_scene_t*)malloc(sizeof(ga_scene_t));
	strncpy(s->name,name,STRING_LENGTH);
	s->bg_color = vec_new(0,0,0,1);
	s->active_camera = NULL;
	s->camera = ga_list_new();
	s->light = ga_list_new();
	s->material = ga_list_new();
	s->geom = ga_list_new();
	s->transform = ga_transform_identity();
	return s;
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
