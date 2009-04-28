#include <stdio.h>
#include <math.h>
#include "ga_raster.h"
#include "ga_scene.h"
#define EPSILON 0.000001f
/* this is adapted from the raytracing triangle intersection code.
 * It's like shooting a ray parallel to z axis*/
int   ga_raster_intersect(const tri_t *tr, const vec_t *start, float *t, float*u,float*v){
	vec_t edge1; 
	vec_t edge2; 
	vec_t pvec; 
	vec_t tvec;
	vec_t qvec;
	vec_t dir = vec_new(0,0,-1,0);
	float det;
	float inv_det;
	/*if(tr->norm.z < 0.0f){
		return 0;
	}*/
	vec_fsub(&edge1,tr->vert + 1,tr->vert + 0);
	vec_fsub(&edge2,tr->vert + 2,tr->vert + 0);
	vec_fcross(&pvec,&dir,&edge2);
	det = vec_fdot(&edge1,&pvec);
	if(det > -EPSILON && det < EPSILON){
		return 0;
	}
	inv_det = 1.0f / det;
	
	vec_fsub(&tvec,start,tr->vert +0);
	*u = vec_fdot(&tvec,&pvec)*inv_det;
	if(*u <-EPSILON || *u >= 1.0 + EPSILON){
		return 0;
	}
	vec_fcross(&qvec,&tvec,&edge1);
	*v = vec_fdot(&dir,&qvec)*inv_det;
	if(*v <-EPSILON || *u + *v >1.0f +EPSILON){
		return 0;
	}
	*t = vec_fdot(&edge2,&qvec)*inv_det;
	return 1;
}
/*paint a triangle on the image, using z buffer values to mask hidden pixels */
void  ga_raster_triangle(ga_image_t *img,tri_t *tri){
	float minx = tri->vert[2].x;	/* triangle bounding box*/
	float maxx = minx;
	float miny = tri->vert[2].y;
	float maxy = miny;
	float u = 0;
	float v = 0;
	float t = 0;
	int x = 0;
	int y = 0;
	int i = 2;
	vec_t color;
	vec_t start;
	while(i--){	/*compute triangle bounding box */
		if(tri->vert[i].x < minx){
			minx = tri->vert[i].x;
		}else if(tri->vert[i].x > maxx){
			maxx = tri->vert[i].x;
		}
		if(tri->vert[i].y < miny){
			miny = tri->vert[i].y;
		}else if(tri->vert[i].y > maxy){
			maxy = tri->vert[i].y;
		}
	}
	if(minx > 0.0f){ x = (int)minx; }
	while(x < img->sizex && x < (maxx+1)){/*we iterate over pixels in BB*/
		y = 0;
		if(miny > 0.0f){ y = (int)miny; }
		while(y < img->sizey && y < (maxy+1)){
			start = vec_new(x,y,0,0);
			/* if this pixel is in the triangle we color it by
			 * interpolating the vertex colors and vertex z
			 * values
			 */
			if(ga_raster_intersect(tri,&start,&t,&u,&v)){
				color = vec_add( vec_scale(u,tri->vcolor[1]),
					vec_add( vec_scale(v,tri->vcolor[2]),
					vec_scale(1-u-v,tri->vcolor[0])));
				ga_image_set_zpixel(img,x,y,t,color);
			}
			y++;
		}
		x++;
	}
}
/* compute the shading of a point in the scene. Same as in raytracing,
 * without shadow check, and only done once per vertex */
static vec_t ga_raster_shade(vec_t pos, vec_t norm, ga_material_t *mat,ga_scene_t *s){
	ga_node_t *n = s->light->first;
	ga_light_t *light;
	vec_t vl;
	vec_t color = vec_new(0,0,0,1);
	float fact;
	if(n){
		while(n){
			light = (ga_light_t*)n->data;
			vl = vec_norm(vec_sub(light->pos,pos));
			if((fact = vec_dot(norm,vl)) > 0.0f){
				color = vec_add(color,vec_scale(fact,vec_mult(mat->diff_color,light->color)));
			}
			n = n->next;
		}
		return color;
	}else{
		return mat->diff_color;
	}
}
/*computes the camera view transform matrix*/
static void ga_raster_cam_transform(ga_scene_t *s, mat_t *tr){
	float fov = s->active_camera->fov;
	float l,b,n,r,t,f;
	vec_t w = vec_norm(vec_neg(s->active_camera->dir));
	vec_t u = vec_norm(vec_cross(s->active_camera->up,w));
	vec_t v = vec_norm(vec_cross(w,u));
	mat_t *tmp = mat_new_zero();
	n = 1.0f;
	f = 10.0f;
	r = tanf(fov*3.141592/180.0/2.0)*n;
	l = -r;
	t = r*s->img->sizey/s->img->sizex;
	b = -t;
	/*to pixel coordinates*/
	mat_set_2d(s->img->sizex,s->img->sizey,tr);
	/*to canonical view volume*/
	mat_set_ortho(vec_new(r,t,-n,1),vec_new(l,b,-f,1),tmp);
	mat_mult(tr,tmp);
	/*perspective*/
	mat_set_persp(n,f,tmp);
	mat_mult(tr,tmp);
	/*camera position*/
	mat_set_view(s->active_camera->pos,u,v,w,tmp);
	mat_mult(tr,tmp);
	mat_free(tmp);
}
/*renders the scene to s->img */
void ga_raster_render(ga_scene_t *s){
	ga_node_t *n = s->shape->first;
	ga_shape_t *shape;
	ga_geom_t  *geom;
	model_t *model;
	ga_material_t *material;
	tri_t *tri;
	tri_t  tri_tmp;
	int i = 0;
	int j = 0;
	mat_t cam_transform;	/*the transformation matrix*/
	ga_raster_cam_transform(s,&cam_transform);
	ga_image_fill(s->img,s->bg_color);
	while(n){	/*iterate over shapes*/
		shape = (ga_shape_t*)n->data;
		geom = shape->geom;
		model = geom->model;
		material = shape->material;
		if(!model || !material){
			fprintf(stderr,"WARNING: geometry without model or material\n");
			return;
		}
		i = model->tri_count;
		while(i--){	/*iterate over triangles*/
			if((i%100)==0){printf("%d\n",i);}
			j = 3;
			tri = model->tri + i;
			while(j--){	/*light the vertexes*/
				tri->vcolor[j] = ga_raster_shade(tri->vert[j],
								tri->vnorm[j],
								material,
								s	);

			}
			tri_cpy(&tri_tmp,model->tri + i);
			/*transform triangle*/
			tri_transform(&tri_tmp,&cam_transform,NULL);
			/*paint the triangle*/
			ga_raster_triangle(s->img,&tri_tmp);
		}
		n = n->next;
	}
}
