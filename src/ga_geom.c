#include "ga_geom.h"
#include <stdlib.h>
#include <string.h> 
#include <stdio.h> 

#define LINE_LENGTH 256
/* Buffers to store data when we load an obj file */
static vec_t vert[GA_MAX_VERT];
static int  vert_count;
static vec_t norm[GA_MAX_VERT];
static int  norm_count;
static vec_t tex[GA_MAX_VERT];
static int  tex_count;
static tri_t tri[GA_MAX_VERT];
static int  tri_count;

tri_t *tri_print(tri_t *t){
	printf("vert: [ %f %f %f | %f %f %f | %f %f %f ] \n",
			t->vert[0].x,t->vert[0].y,t->vert[0].z,
			t->vert[1].x,t->vert[1].y,t->vert[1].z,
			t->vert[2].x,t->vert[2].y,t->vert[2].z	);
	printf("vnorm:[ %f %f %f | %f %f %f | %f %f %f ] norm: [ %f %f %f ]\n",
			t->vnorm[0].x,t->vnorm[0].y,t->vnorm[0].z,
			t->vnorm[1].x,t->vnorm[1].y,t->vnorm[1].z,
			t->vnorm[2].x,t->vnorm[2].y,t->vnorm[2].z,
			t->norm.x,t->norm.y,t->norm.z	);
	printf("vtex: [ %f %f | %f %f | %f %f ]\n",
			t->vtex[0].x,t->vtex[0].y,
			t->vtex[1].x,t->vtex[1].y,
			t->vtex[2].x,t->vtex[2].y	);
	return t;
}
tri_t *tri_transform(tri_t *t, const mat_t *m){
	int i = 3;
	while(i--){
		t->vert[i] = vec_point(vec_vec(mat_vmult(m,t->vert[i])));
	}
	return t;
}
void   tri_cpy(tri_t *t, const tri_t *t2){
	memcpy(t,t2,sizeof(tri_t));
}
void	model_free(model_t*m){
	free(m->tri);
	free(m);
}
void model_print(const model_t*m){
	int i = 0;
	printf("MODEL: tri_count:%d\n",m->tri_count);
	while(i < m->tri_count){
		tri_print(m->tri + i);
		i++;
	}
}
/* reads a line of file into a buffer */
static int readline(char *buffer, FILE*file, int length){
	char c;
	int i = 0;
	while(i < length){
		c = (char)fgetc(file);
		buffer[i] = c;
		if( i == length -1){
			buffer[i] = '\0';
			printf("ERROR: readline() : line too long\n");
			return 0;
		}else if( c == EOF){
			buffer[i] = '\0';
			return 0;
		}else if (c == '\n'){
			buffer[i] = '\0';
			return 1;
		}
		i++;
	}
	return 0;
}
/* removes special caracters from lines and replaces them by spaces */
static void cleanline(char *line, int length){
	int i = 0;
	while(i < length){
		if (line[i] == '/'){
			line[i] = ' ';
		}
		i++;
	}
}
/** 
 * creates a triangle from the data in the loading buffers. 
 * 'face' is the list of numbers in the obj file specifying the face
 * 'fcount' is the count of those numbers ( 3, 5 or 9) 
 */
static void  tri_load(tri_t *tri, int *face, int fcount){
	int i = 3;
	vec_t nnorm;
	if(fcount == 9){
		i = 3;
		while(i--){	/*TODO security : check for overflows*/
			tri->vert[i]  = vert[face[i*3]];
			tri->vtex[i]  = tex[face[i*3+1]];
			tri->vnorm[i] = norm[face[i*3+2]];
		}
	}else if(fcount == 6){
		i = 3;
		while(i--){
			tri->vert[i]  = vert[face[i*2]];
			if(norm_count){
				tri->vnorm[i] = norm[face[i*2+1]];
			}else{
				tri->vtex[i]  = tex[face[i*2+1]];
			}
		}
	}else{
		i = 3;
		while(i--){
			tri->vert[i]  = vert[face[i]];
		}
	}
	if(norm_count){	
		/* we compute the polygon normal to be on the same
		   side as the mean of its vertex normals	*/
		nnorm = vec_scale(0.3333333333,
			vec_add( tri->vnorm[0],
			vec_add( tri->vnorm[1],
			tri->vnorm[0])));
		tri->norm = vec_norm(vec_cross(	vec_delta(tri->vert[0],tri->vert[1]),
					vec_delta(tri->vert[0],tri->vert[2]) ));
		if(vec_dot(nnorm,tri->norm) < 0){
			tri->norm = vec_neg(tri->norm);
		}
	}
}
static model_t *model_new(void){
	model_t *m = (model_t*)malloc(sizeof(model_t));
	m->tri_count = tri_count;
	m->tri = (tri_t*)malloc(tri_count*sizeof(tri_t));
	memcpy(m->tri,tri,tri_count*sizeof(tri_t));
	return m;
}
/**
 * Loads obj file from path
 */
model_t *model_load(char *path){
	FILE*f = NULL;
	char line[LINE_LENGTH];
	char *lptr = NULL;
	char *sptr = NULL;
	float x,y,z;
	int face[9];
	int i = 0;
	model_t *m;

	vert_count = 0;
	norm_count = 0;
	tri_count  = 0;
	tex_count  = 0;

	if(!(f = fopen(path,"r"))){
		printf("ERROR: could not read file %s\n",path);
		return NULL;
	}
	while(readline(line,f,LINE_LENGTH)){
		switch(line[0]){
			case 'v':
				x = (float)strtod(line + 2,&lptr);
				y = (float)strtod(lptr, &lptr);
				z = (float)strtod(lptr, &lptr);
				if(line[1]==' '){
					vert[vert_count] = vec_new(x,y,z,1);
					vert_count++;
				}else if(line[1]=='n'){
					norm[norm_count] = vec_new(x,y,z,0);
					norm_count++;
				}else if(line[1]=='t'){
					tex[tex_count] = vec_new(x,y,0,0);
					tex_count++;
				}
				break;
			case 'f':
				cleanline(line,LINE_LENGTH);
				i = 0;
				lptr = line + 1;
				while(i < 9){
					face[i] = (int)strtol(lptr,&sptr,10) - 1;
					if(lptr == sptr){
						break;
					}else{
						lptr = sptr;
						i++;
					}
				}
				tri_load(tri + tri_count, face, i);
				tri_count++;
				break;
			case '#':break;
			default :
				printf("WARNING: feature not implemented in obj format : '%s'\n",line);
				break;
		}
		if(	vert_count >= GA_MAX_VERT
			||norm_count >= GA_MAX_VERT
			||tex_count  >= GA_MAX_VERT
			||tri_count  >= GA_MAX_VERT ){
			fprintf(stderr,"ERROR: model '%s' too big\n",path);
			return NULL;
		}
	}
	m = model_new();
	memset(tri,0,tri_count*sizeof(tri_t));
	fclose(f);
	printf("SUCCESS: Model '%s' successfully loaded\n",path);
	return m;
}
/*TODO loading geometry from xml file */
model_t *model_parse(	char *v, 
			char* n, 
			char* t, 
			char* vi,
			char* ni,
			char* ti	);
/*int main(int argc, char **argv){
	model_t *m;
	if(argc < 2){
		printf("ERROR: no .obj file path argument given\n");
		return 1;
	}
	m = model_load(argv[1]);
	model_print(m);
	return 0;
}*/

