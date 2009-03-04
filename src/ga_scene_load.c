#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "ga_scene.h"
#include "ga_math.h"

/**
 * parse a 'Camera' xml tag, and adds a camera to the scene, with
 * default values if missing tags.
 */
static void ga_cam_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	vec_t position 	= vec_new(0,0,0,1);
	vec_t dir 	= vec_new(1,0,0,1);
	vec_t up 	= vec_new(0,0,1,1);
	float fov	= 90.0f;
	ga_cam_t * cam 	= NULL;
	char * name	= "unnamed_camera";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"position")){
			position = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"direction")){
			dir	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"up")){
			up	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"fovy")){
			fov	 = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"\nWARNING: unimplemented camera property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	cam = ga_cam_new(name,position,dir,up,fov);
	ga_scene_add_camera(s,cam);
	ga_cam_print(cam);
}
/**
 * parse a 'PointLight' xml tag, and adds a PointLight to the scene, with
 * default values if missing attributes.
 */
static void ga_pointlight_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	vec_t position 	= vec_new(0,0,0,1);
	vec_t color 	= vec_new(1,1,1,1);
	float intensity	= 1.0f;
	ga_light_t*light= NULL;
	char * name	= "unnamed_point_light";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"position")){
			position = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"color")){
			color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"intensity")){
			intensity= (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"\nWARNING: unimplemented light property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	color.w = intensity;
	light = ga_light_new(name,position,color);
	ga_scene_add_light(s,light);
	ga_light_print(light);
}
/**
 * parse a 'Sphere' xml tag, and adds a Sphere to the scene, with
 * default values if missing attributes.
 */
static void ga_sphere_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	float radius	= 0.5f;
	ga_geom_t*geom= NULL;
	char * name	= "unnamed_sphere";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"radius")){
			radius= (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"\nWARNING: unimplemented sphere property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	geom = ga_geom_new_sphere(name,radius);
	ga_scene_add_geom(s,geom);
	ga_geom_print(geom);
}
/**
 * parse a 'DiffuseMaterial' xml tag, and adds a DiffuseMaterial to the scene, with
 * default values if missing attributes.
 */
static void ga_diffusematerial_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	vec_t color 	= vec_new(1,1,1,1);
	ga_material_t*mat= NULL;
	char * name	= "unnamed_diffuse_material";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"color")){
			color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"\nWARNING: unimplemented diffuse material property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	mat = ga_material_new_diffuse(name,color);
	ga_scene_add_material(s,mat);
	ga_material_print(mat);
}
/**
 * Parse a 'Shape' xml tag, checks that geometry and material attributes
 * point to valid objects, and if yes adds itself as children of the parent
 * transform. Root transform is identity.
 */
static void ga_shape_explore(xmlNodePtr n, ga_scene_t *s, ga_transform_t *t){
	xmlAttrPtr a = n->properties;
	char *geometry = "default_sphere";
	char *material = "default_material";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"geometry")){
			geometry = (char*)a->children->content;
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"material")){
			material = (char*)a->children->content;
		}else{
			fprintf(stderr,"\nWARNING: unexpected shape property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	if(!ga_scene_get_geom(s,geometry)){
		fprintf(stderr,"\nERROR: geometry '%s' undefined\n", geometry);
		return;
	}
	if(!ga_scene_get_material(s,material)){
		fprintf(stderr,"\nERROR: material '%s' undefined\n", material);
		return;
	}
	ga_transform_add_shape(	t, ga_shape_new(
			geometry,
			ga_scene_get_geom(s,geometry),
			ga_scene_get_material(s,material)	));
}
/**
 * Parse 'Scene' or 'Transform' childrens and various transformations tags to
 * create the scene graph.
 */
static void ga_scene_graph_explore(xmlNodePtr n, ga_scene_t *s, ga_transform_t *t){
	xmlAttrPtr a;
	vec_t param = vec_new(0,0,0,1);
	float angle = 0.0f;
	int type;
	ga_transform_t *newt = NULL;
	while(n){
		if(!xmlStrcmp(n->name,(const xmlChar*)"Shape")){
			ga_shape_explore(n,s,t);
			n = n->next;
			continue;
		}else if(!xmlStrcmp(n->name,(const xmlChar*)"Rotate")){
			type = GA_ROTATE;
		}else if(!xmlStrcmp(n->name,(const xmlChar*)"Scale")){
			type = GA_SCALE;
		}else if(!xmlStrcmp(n->name,(const xmlChar*)"Translate")){
			type = GA_TRANSLATE;
		}else if(!xmlStrcmp(n->name,(const xmlChar*)"text")){
			n = n->next;
			continue;
		}else{
			fprintf(stderr,"\nWARNING: unimplemented transform '%s'\n",(const char*)n->name);
			n = n->next;
			continue;
		}
		a = n->properties;
		while(a){
			a = a->next;
			if(!xmlStrcmp(a->name,(const xmlChar*)"axis")){
				param = vec_parse((char*)a->children->content);
			}else if(!xmlStrcmp(a->name,(const xmlChar*)"vector")){
				param = vec_parse((char*)a->children->content);
			}else if(!xmlStrcmp(a->name,(const xmlChar*)"scale")){
				param = vec_parse((char*)a->children->content);
			}else if(!xmlStrcmp(a->name,(const xmlChar*)"angle")){
				angle = (float)strtod((char*)a->children->content,NULL);
			}else{
				fprintf(stderr,"\nWARNING: unexpected transform attribute '%s'\n",(const char*)a->name);
			}
		}
		switch(type){
			case GA_ROTATE: newt = ga_transform_rotate(param,angle); break;
			case GA_SCALE:  newt = ga_transform_scale(param);  break;
			case GA_TRANSLATE: newt = ga_transform_translate(param);
			default: break;
		}
		ga_transform_add_child(t,newt);
		ga_scene_graph_explore(n->xmlChildrenNode,s,newt);
		n = n->next;
	}
}
/**
 * Parse the 'Scene' tag, updates background color and active lights / camera
 * TODO : active lights not implemented.
 */
static void ga_scene_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a;
	a = n->properties;
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"camera")){
			s->active_camera = ga_scene_get_camera(s,(char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"lights")){
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"background")){
			s->bg_color = vec_parse((char*)a->children->content);
		}
		a = a->next;
	}
	ga_scene_graph_explore(n->xmlChildrenNode,s,s->transform);
}
/**
 * Parse the whole SDL xml document in order to create the scene
 */
static void ga_xml_explore(xmlNodePtr n, ga_scene_t *s){
	while(n){
		if(xmlStrcmp(n->name,(const xmlChar*)"text")){
			if(!xmlStrcmp(n->name,(const xmlChar*)"Camera")){
				ga_cam_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"PointLight")){
				ga_pointlight_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"DiffuseMaterial")){
				ga_diffusematerial_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"Sphere")){
				ga_sphere_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"Scene")){
				ga_scene_explore(n,s);
			}else{
				if(n->xmlChildrenNode){
					ga_xml_explore(n->xmlChildrenNode,s);
				}else{
					fprintf(stderr,"WARNING: unimplemented feature '%s'\n",
						(char*)n->name	);
				}
			}
		}
		n = n->next;
	}
}
ga_scene_t *ga_scene_load(char *path){
	xmlDocPtr doc;
	xmlNodePtr cur;
	ga_scene_t *s = ga_scene_new(path);

	doc = xmlParseFile(path);	/*load file*/
	if(!doc){
		fprintf(stderr,"ERROR: scene '%s' not parsed successfully \n",path);
		return NULL;
	}
	cur = xmlDocGetRootElement(doc);
	if(!cur){
		fprintf(stderr,"ERROR: scene '%s' is empty \n",path);
		xmlFreeDoc(doc);
		return NULL;
	}
	if(xmlStrcmp(cur->name, (const xmlChar *) "Sdl")){
		fprintf(stderr,"ERROR: error in scene format '%s' : root note must be 'Sdl'\n",path);
		xmlFreeDoc(doc);
		return NULL;
	}
	/* At this point we have successfully loaded a non empty sdl
	 * document.*/

	ga_xml_explore(cur,s);

	printf("Document '%s' succesfully loaded\n",path);
	return NULL;	
}

int main(int argc, char **argv){
	if(argc < 2){
		fprintf(stderr,"ERROR: a path to the sdl scene file must be given as argument\n");
		return 1;
	}
	ga_scene_load(argv[1]);
	return 0;
}