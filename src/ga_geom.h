#ifndef __GALLY_GEOMETRY_H__
#define __GALLY_GEOMETRY_H__
#include "ga_math.h"
#define GA_MAX_VERT 50000
typedef struct tri_s{
	vec_t norm;
	vec_t vert[3];
	vec_t vnorm[3];
	vec_t vtex[3];
}tri_t;

tri_t *tri_print(tri_t *t);
tri_t *tri_transform(tri_t *t, const mat_t *m);

typedef struct model_s{
	int tri_count;
	tri_t *tri;
}model_t;

model_t *model_load(char *path);
model_t *model_dup(const model_t *m);
void 	model_free(model_t *m);
void model_print(const model_t*m);

#endif



