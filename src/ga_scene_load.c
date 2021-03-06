#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "ga_scene.h"
#include "ga_math.h"
/**
 * SEE http://xmlsoft.org/html/libxml-parser.html 
 * for documentation on the libxml2 api used in this file
 */
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
	char * name	= "unnamed_camera";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"position")){
			position = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"direction")){
			dir	 = vec_norm(vec_parse((char*)a->children->content));
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"up")){
			up	 = vec_norm(vec_parse((char*)a->children->content));
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"fovy")){
			fov	 = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"WARNING: unimplemented camera property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	ga_scene_add_camera(s,ga_cam_new(name,position,dir,up,fov));
}
/**
 * parse a 'PointLight' xml tag, and adds a PointLight to the scene, with
 * default values if missing attributes.
 */
static void ga_pointlight_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	vec_t position 	= vec_new(0,0,0,1);
	vec_t color 	= vec_new(1,1,1,1);
	float radius	= 0.0f;
	int   samples   = 1;
	int   photons   = 0;
	float photon_weight = 1.0f;
	float intensity	= 1.0f;
	char * name	= "unnamed_point_light";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"position")){
			position = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"color")){
			color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"intensity")){
			intensity= (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"radius")){
			radius = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"samples")){
			samples = (int)strtod((char*)a->children->content,NULL);
			if(samples <= 0){ samples = 1; }
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"photons")){
			photons = (int)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"photon_weight")){
			photon_weight = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"WARNING: unimplemented light property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	color.w = intensity;
	ga_scene_add_light(s,ga_light_new(name,position,color,radius,samples,
				photons,photon_weight));
}
static void ga_obj_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	model_t *m	= NULL;
	char * path	= NULL;
	char * name	= "unnamed_obj_file";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"src")){
			path = (char*)a->children->content;
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name = (char*)a->children->content;
		}else{
			fprintf(stderr,"WARNING: unimplemented obj property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	if(!path){
		fprintf(stderr,"ERROR: <Obj name='%s' /> missing a 'file' attribute\n",name); 
	}else if(!(m = model_load(path))){
		fprintf(stderr,"ERROR: unable to load obj file '%s'\n",path);
	}else{
		ga_scene_add_geom(s,ga_geom_new_model(name,m));
	}
}
/**
 * parse a 'DiffuseMaterial' xml tag, and adds a DiffuseMaterial to the scene, with
 * default values if missing attributes.
 */
static void ga_diffusematerial_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	vec_t color 	= vec_new(1,1,1,1);
	char * name	= "unnamed_diffuse_material";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"color")){
			color	 = vec_parse((char*)a->children->content);
			color.w = 1.0f;
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"WARNING: unimplemented diffuse material property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	ga_scene_add_material(s,ga_material_new_diffuse(name,color));
}
static void ga_phongmaterial_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	vec_t color 	= vec_new(1,1,1,1);
	float power = 2.0;
	char * name	= "unnamed_phong_material";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"color")){
			color	 = vec_parse((char*)a->children->content);
			color.w = 1.0f;
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"shininess")){
			power = (float)strtod((char*)a->children->content,NULL);
		}else{
			fprintf(stderr,"WARNING: unknown phong material property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	ga_scene_add_material(s,ga_material_new_phong(name,color,power));
}
static void ga_fullmaterial_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	ga_material_t *m = ga_material_new_full("unnamed_material");
	while(a){
		/* BASE PROP */
		if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			strncpy(m->name,(char*)a->children->content,STRING_LENGTH);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"diff_color")){
			m->diff_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"spec_color")){
			m->spec_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"emit_color")){
			m->emit_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"flat_color")){
			m->flat_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ref_color")){
			m->ref_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ao_min_color")){
			m->ao_min_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ao_max_color")){
			m->ao_max_color	 = vec_parse((char*)a->children->content);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"diff_factor")){
			m->diff_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"spec_factor")){
			m->spec_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"spec_power")){
			m->spec_power = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"emit_factor")){
			m->emit_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"flat_factor")){
			m->flat_factor = (float)strtod((char*)a->children->content,NULL);
		/* TRANSPARENCY / REFLECTIONS */
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ior")){
			m->ior = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"transp_factor")){
			m->transp_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ref_factor")){
			m->ref_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ref_fresnel")){
			m->ref_fresnel = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"soft_ref_angle")){
			m->soft_ref_angle = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"soft_ref_sample")){
			m->soft_ref_sample = (int)strtod((char*)a->children->content,NULL);
		/* AMBIANT OCCLUSION */
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ao_factor")){
			m->ao_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ao_sample")){
			m->ao_sample = (int)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ao_min_dist")){
			m->ao_min_dist = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"ao_max_dist")){
			m->ao_max_dist = (float)strtod((char*)a->children->content,NULL);
		/* GLOBAL ILLUMINATION */
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"gi_factor")){
			m->gi_factor = (float)strtod((char*)a->children->content,NULL);
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"gi_sample")){
			m->gi_sample = (int)strtod((char*)a->children->content,NULL);	
		}else{
			fprintf(stderr,"WARNING: unimplemented full material property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	ga_scene_add_material(s,m);
}
static void ga_combmaterial_explore(xmlNodePtr n, ga_scene_t *s){
	xmlAttrPtr a 	= n->properties;
	int comb_weight_count = 0;
	float comb_weight[GA_MAX_COMB_MATERIAL];
	int comb_material_count = 0;
	ga_material_t *comb_material[GA_MAX_COMB_MATERIAL];
	ga_material_t *comb = NULL;
	char * name	= "unnamed_comb_material";
	int i = 0;
	while(a){
		if(!xmlStrncmp(a->name,(const xmlChar*)"material",8)){
			i = atoi((const char*)a->name + 8) -1;
			if(i >= 0 && i < GA_MAX_COMB_MATERIAL){
				comb_material[i] = ga_scene_get_material(s,
						(char*)a->children->content);
				if(i+1 > comb_material_count){
					comb_material_count = i+1;
				}
			}else{
				fprintf(stderr,"ERROR: sub material index out of range : %d\n",i);
			}
		}else if(!xmlStrncmp(a->name,(const xmlChar*)"weight",6)){
			i = atoi((const char*)a->name + 6) -1;
			if(i >= 0 && i < GA_MAX_COMB_MATERIAL){
				comb_weight[i] =(float)strtod((char*)a->children->content,NULL);
				if(i+1 > comb_weight_count){
					comb_weight_count = i+1;
				}
			}else{
				fprintf(stderr,"ERROR: weight index out of range : %d\n",i);
			}
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"name")){
			name	 = (char*)a->children->content;
		}else{
			fprintf(stderr,"WARNING: unimplemented combinaison material property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	if(comb_weight_count == comb_material_count){
		i = comb_weight_count;
		comb = ga_material_new_comb(name);
		while(i--){
			ga_material_add_comb(comb,
				ga_material_new_blending("blending",
							comb_weight[i],
							comb_material[i] ));
		}
		ga_scene_add_material(s,comb);
	}else{
		fprintf(stderr,"WARNING: weight index doesn't match material index : %d, %d \n",
				comb_weight_count,comb_material_count);
	}
}
/**
 * Parse a 'Shape' xml tag, checks that geometry and material attributes
 * point to valid objects, and if yes adds itself as children of the parent
 * transform. Root transform is identity.
 */
static void ga_shape_explore(xmlNodePtr n, ga_scene_t *s, ga_transform_t *t){
	xmlAttrPtr a = n->properties;
	ga_shape_t *shape = NULL;
	char *geometry = "default_sphere";
	char *material = "default_material";
	while(a){
		if(!xmlStrcmp(a->name,(const xmlChar*)"geometry")){
			geometry = (char*)a->children->content;
		}else if(!xmlStrcmp(a->name,(const xmlChar*)"material")){
			material = (char*)a->children->content;
		}else{
			fprintf(stderr,"WARNING: unexpected shape property '%s' \n",(const char*)a->name);
		}
		a = a->next;
	}
	if(!ga_scene_get_geom(s,geometry)){
		fprintf(stderr,"ERROR: geometry '%s' undefined\n", geometry);
		return;
	}
	if(!ga_scene_get_material(s,material)){
		fprintf(stderr,"ERROR: material '%s' undefined\n", material);
		return;
	}
	shape = ga_shape_new(geometry,ga_scene_get_geom(s,geometry),
					ga_scene_get_material(s,material));
	ga_list_add(s->shape,shape);
	ga_transform_add_shape(	t, shape); 
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
			fprintf(stderr,"WARNING: unimplemented transform '%s'\n",(const char*)n->name);
			n = n->next;
			continue;
		}
		a = n->properties;
		while(a){
			if(!xmlStrcmp(a->name,(const xmlChar*)"axis")){
				param = vec_parse((char*)a->children->content);
			}else if(!xmlStrcmp(a->name,(const xmlChar*)"vector")){
				param = vec_parse((char*)a->children->content);
			}else if(!xmlStrcmp(a->name,(const xmlChar*)"scale")){
				param = vec_parse((char*)a->children->content);
			}else if(!xmlStrcmp(a->name,(const xmlChar*)"angle")){
				angle = (float)strtod((char*)a->children->content,NULL);
			}else{
				fprintf(stderr,"WARNING: unexpected transform attribute '%s'\n",(const char*)a->name);
			}
			a = a->next;
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
			s->bg_color.w = 1.0f;
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
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"PhongMaterial")){
				ga_phongmaterial_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"FullMaterial")){
				ga_fullmaterial_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"LinearCombinedMaterial")){
				ga_combmaterial_explore(n,s);
			}else if(!xmlStrcmp(n->name,(const xmlChar*)"Obj")){
				ga_obj_explore(n,s);
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

	printf("SUCCESS: Document '%s' succesfully loaded\n",path);
	return s;	
}
/*
int main(int argc, char **argv){
	ga_scene_t *s = NULL;
	ga_image_t *img = ga_image_new(400,300);
	ga_image_fill(img,vec_new(0.8,0.4,0.1,1));
	ga_image_save(img,"out.png");
	if(argc < 2){
		fprintf(stderr,"ERROR: a path to the sdl scene file must be given as argument\n");
		return 1;
	}
	s = ga_scene_load(argv[1]);
	ga_scene_print(s);
	return 0;
}*/
