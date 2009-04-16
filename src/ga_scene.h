#ifndef __GA_SCENE_H__
#define __GA_SCENE_H__
#include "ga_geom.h"
#include "ga_list.h" 
#include "ga_img.h"
#include "ga_kdt.h"

#define STRING_LENGTH GA_NAME_LENGTH
#define GA_MAX_COMB_MATERIAL 16

/**	EXPLAINATION
 *
 * The scene is stored in memory in a data structure
 * modeling the file layout. 
 * Each entity type has its own structure and 
 * are stored in lists. 
 *
 * All those data structure are stored in the
 * scene data structure.
 *
 * The scene graph is stored as a tree of transformation
 * starting by an identity transformation.
 * each transformation has a list of shapes and child
 * transformation.
 *
 * transformations are currently not used by the renderer.
 */

/*------- CAMERA --------*/
typedef struct ga_cam_s{
	char name[STRING_LENGTH];
	vec_t pos;	/*camera position*/
	vec_t dir; 	/*camera direction*/
	vec_t up;	/*camera up vector*/
	float fov;	/*field of view (horizontal) in degrees.*/
}ga_cam_t;
ga_cam_t *ga_cam_new(char *name, vec_t pos, vec_t dir, vec_t up, float fov);
void	  ga_cam_print(ga_cam_t*c);

/*------- LIGHT --------*/
typedef struct ga_light_s{
	char name[STRING_LENGTH];
	vec_t  pos;	/* light position */
	vec_t  color; 	/*.w = intensity */
}ga_light_t;
ga_light_t *ga_light_new(char *name, vec_t pos, vec_t color);
void	ga_light_print(ga_light_t*l);

/*------- MATERIAL --------*/
enum ga_material_type{
	GA_MATERIAL_PHONG,
	GA_MATERIAL_DIFFUSE,
	GA_MATERIAL_FLAT,
	GA_MATERIAL_EMIT,
	GA_MATERIAL_COMB,
	GA_MATERIAL_BLENDING
};
typedef struct ga_material_s{
	char name[STRING_LENGTH];
	int type;	/* type of material */ 
	float power;
	float alpha;
	ga_list_t *comb;
	struct ga_material_s *child;
	vec_t color;	/* color of the material. w is intensity */
}ga_material_t;
ga_material_t *ga_material_new_diffuse(char *name, vec_t color);
ga_material_t *ga_material_new_phong(char *name, vec_t color,float power);
ga_material_t *ga_material_new_blending(char *name, float alpha, ga_material_t *mat);
ga_material_t *ga_material_new_comb(char *name);
ga_material_t *ga_material_new_flat(char *name,vec_t color);
ga_material_t *ga_material_new_emit(char *name,vec_t color);
void	ga_material_add_comb(ga_material_t *mat, ga_material_t *comb);
void	ga_material_print(ga_material_t *m);

/*------- GEOMETRY --------*/
typedef struct ga_geom_s{
	char name[STRING_LENGTH];
	model_t *model; 
}ga_geom_t;
ga_geom_t *ga_geom_new_model(char *name, model_t *m);
void	ga_geom_print(ga_geom_t*g);

/*------- SHAPE --------*/
typedef struct ga_shape_s{
	char name[STRING_LENGTH];
	ga_geom_t 	*geom;		/* geometry of the shape */
	ga_material_t 	*material; 	/* material of the shape */
}ga_shape_t;
ga_shape_t *ga_shape_new(char*name, ga_geom_t*g, ga_material_t *m);
void	ga_shape_print(ga_shape_t*s);

/*------- TRANSFORMATION --------*/
enum ga_transform_type{
	GA_IDENTITY,
	GA_TRANSLATE,
	GA_ROTATE,
	GA_SCALE
};
typedef struct ga_transform_s{
	char name[STRING_LENGTH];
	int type;		/* type of transform */
	vec_t param;		/* the direction/scale/translation vector */
	float angle;		/* the rotation angle in degrees */
	mat_t *matrix;		/* the tranformation matrix */
	ga_list_t *child;	/* list of child transformations */
	ga_list_t *shape;	/* list of shapes */
}ga_transform_t;
ga_transform_t *ga_transform_identity(void);
ga_transform_t *ga_transform_translate(vec_t param);
ga_transform_t *ga_transform_rotate(vec_t param,float angle);
ga_transform_t *ga_transform_scale(vec_t param);
void ga_transform_add_child(ga_transform_t *p, ga_transform_t *c);
void ga_transform_add_shape(ga_transform_t *p, ga_shape_t *c);
void ga_transform_print(ga_transform_t *t);
void ga_transform_apply(ga_transform_t *t);

/*------- SCENE --------*/
typedef struct ga_scene_s{
	char name[STRING_LENGTH];
	vec_t	 bg_color;		/* background color of the scene */
	ga_cam_t *active_camera;	/* the active_camera */
	ga_list_t *camera;		/* list of camera */
	ga_list_t *light;		/* list of light */
	ga_list_t *active_light;	/* list of active light */
	ga_list_t *material;		/* material list */
	ga_list_t *geom;		/* geometry list */
	ga_list_t *shape;		/* shape list */
	ga_list_t *tri_pool;		/* all the triangles in worldspace coords */
	ga_transform_t *transform;	/* base transform */
	ga_image_t *img;		/* output image */
	ga_kdn_t   *kdtree;
	vec_t box_min;
	vec_t box_max;
}ga_scene_t;
ga_scene_t *ga_scene_new(char *name);
/**
 * Sets the image rendering size.
 */
void	ga_scene_set_image(ga_scene_t *s, int sizex, int sizey);
/**
 * Saves the rendering to path
 */
void	ga_scene_save_image(ga_scene_t *s, char * path);
/**
 * Print the scene to standard output in sdl format
 */
void	ga_scene_print(ga_scene_t *s);
void	ga_scene_add_camera(ga_scene_t *s, ga_cam_t *c);
void	ga_scene_add_light(ga_scene_t *s, ga_light_t *l);
void	ga_scene_add_material(ga_scene_t *s, ga_material_t *m);
void	ga_scene_add_geom(ga_scene_t *s, ga_geom_t *g);

ga_cam_t 	*ga_scene_get_camera(ga_scene_t *s, const char *name);
ga_light_t	*ga_scene_get_light(ga_scene_t *s, const char *name);
ga_material_t	*ga_scene_get_material(ga_scene_t *s, const char *name);
ga_geom_t	*ga_scene_get_geom(ga_scene_t *s, const char *name);
/**
 * Create a new scene from sdl file in path
 */
ga_scene_t *ga_scene_load(char *path);
void	ga_scene_build_triangle(ga_scene_t *s);
void	ga_scene_build_bounding_box(ga_scene_t *s);
void	ga_scene_build_kdtree(ga_scene_t *s);

#endif
