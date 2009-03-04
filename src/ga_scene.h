#ifndef __GA_SCENE_H__
#define __GA_SCENE_H__
#include "ga_geom.h"
#include "ga_list.h" 
#include "ga_img.h"

#define STRING_LENGTH GA_NAME_LENGTH

typedef struct ga_cam_s{
	char name[STRING_LENGTH];
	vec_t pos;
	vec_t dir; 
	vec_t up;
	float fov;
}ga_cam_t;
ga_cam_t *ga_cam_new(char *name, vec_t pos, vec_t dir, vec_t up, float fov);
void	  ga_cam_print(ga_cam_t*c);
typedef struct ga_light_s{
	char name[STRING_LENGTH];
	vec_t  pos;
	vec_t  color; /*.w = intensity */
}ga_light_t;
ga_light_t *ga_light_new(char *name, vec_t pos, vec_t color);
void	ga_light_print(ga_light_t*l);

enum ga_material_type{
	GA_MATERIAL_PHONG,
	GA_MATERIAL_DIFFUSE,
	GA_MATERIAL_COMB
};
typedef struct ga_material_s{
	char name[STRING_LENGTH];
	int type;
	vec_t color;
}ga_material_t;
ga_material_t *ga_material_new_diffuse(char *name, vec_t color);
void	ga_material_print(ga_material_t *m);

enum ga_geom_type{
	GA_GEOM_SPHERE,
	GA_GEOM_CYLINDER,
	GA_GEOM_CONE,
	GA_GEOM_TORUS,
	GA_GEOM_MODEL
};
typedef struct ga_geom_s{
	char name[STRING_LENGTH];
	int type;
	float radius;
	model_t *model;
}ga_geom_t;
ga_geom_t *ga_geom_new_sphere(char *name, float radius);
ga_geom_t *ga_geom_new_model(char *name, model_t *m);
void	ga_geom_print(ga_geom_t*g);

typedef struct ga_shape_s{
	char name[STRING_LENGTH];
	ga_geom_t 	*geom;
	ga_material_t 	*material;
}ga_shape_t;
ga_shape_t *ga_shape_new(char*name, ga_geom_t*g, ga_material_t *m);
void	ga_shape_print(ga_shape_t*s);

enum ga_transform_type{
	GA_IDENTITY,
	GA_TRANSLATE,
	GA_ROTATE,
	GA_SCALE
};
typedef struct ga_transform_s{
	char name[STRING_LENGTH];
	int type;
	vec_t param;
	float angle;
	mat_t *matrix;
	ga_list_t *child;
	ga_list_t *shape;
}ga_transform_t;
ga_transform_t *ga_transform_identity(void);
ga_transform_t *ga_transform_translate(vec_t param);
ga_transform_t *ga_transform_rotate(vec_t param,float angle);
ga_transform_t *ga_transform_scale(vec_t param);
void ga_transform_add_child(ga_transform_t *p, ga_transform_t *c);
void ga_transform_add_shape(ga_transform_t *p, ga_shape_t *c);
void ga_transform_print(ga_transform_t *t);

typedef struct ga_scene_s{
	char name[STRING_LENGTH];
	vec_t	 bg_color;
	ga_cam_t *active_camera;
	ga_list_t *camera;
	ga_list_t *light;
	ga_list_t *active_light;
	ga_list_t *material;
	ga_list_t *geom;
	ga_transform_t *transform;
}ga_scene_t;
ga_scene_t *ga_scene_new(char *name);
void	ga_scene_print(ga_scene_t *s);
void	ga_scene_add_camera(ga_scene_t *s, ga_cam_t *c);
void	ga_scene_add_light(ga_scene_t *s, ga_light_t *l);
void	ga_scene_add_material(ga_scene_t *s, ga_material_t *m);
void	ga_scene_add_geom(ga_scene_t *s, ga_geom_t *g);

ga_cam_t 	*ga_scene_get_camera(ga_scene_t *s, const char *name);
ga_light_t	*ga_scene_get_light(ga_scene_t *s, const char *name);
ga_material_t	*ga_scene_get_material(ga_scene_t *s, const char *name);
ga_geom_t	*ga_scene_get_geom(ga_scene_t *s, const char *name);
ga_scene_t *ga_scene_load(char *path);

#endif
